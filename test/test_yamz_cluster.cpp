/** Do a full, if somewhat usless, test of a yamz network.

    The goal of the network is to share local system time between the
    peers.

    Each peer is an instance of this program and has a variable number
    of "client" components each of the same type.  Each component has
    these ports.

    - "askme" is a server-like socket (for now, SERVER) from which the
      components current time may be requested.

    - "askyou" is the client-like socket (for now, CLIENT) used to
      make a request to an "askme".

    The number of components in a peer and the bind/connect details
    of these ports are driven by user-provided, schema-controlled
    JSON.

    Besides message handline, the client task performs some nominal
    comparisons between local and remote times and emits result to the
    terminal.


 */

#include <yamz/server.hpp>
#include <yamz/client.hpp>
#include <yamz/Nljs.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

using namespace yamz;


// fixme: move into util.hpp or somewhere
static yamz::UnixTime ut_now()
{
    using namespace std::chrono;
    auto now_tp = system_clock::now();
    time_t time = system_clock::to_time_t( now_tp );   
    auto now_s = system_clock::from_time_t( time );
    if ( now_s > now_tp ) {
        --time;
        now_s -= seconds(1);
    }
    int ns = duration_cast<duration<int,std::nano> >(now_tp - now_s).count( );
    return yamz::UnixTime{duration_cast<seconds>(now_s.time_since_epoch()).count(), ns};
}

// return ns duration from t1 to t2
static double ut_dt(const yamz::UnixTime& t1, const yamz::UnixTime& t2)
{
    return 1e9 * (t2.s - t1.s) + (t2.ns - t1.ns);
}

#define chirp(cfg, strm) { std::stringstream ss; ss << "test_yamz_cluster: " << cfg.clientid << ": " << strm << "\n"; std::cerr << ss.str(); }

static void handle_request(const ClientConfig& cfg, zmq::socket_t& sock)
{
    zmq::message_t msg;
    auto rres = sock.recv(msg, zmq::recv_flags::none);
    if (!rres) {
        chirp(cfg, "failed to recv request");
        return;
    }
    auto remid = msg.routing_id();
    auto jobj = yamz::data_t::parse(msg.to_string());
    auto ttreq = jobj.get<yamz::TestTimeRequest>();
    TestTimeReply ttrep{ut_now(), ttreq.reqtime};
    jobj = ttrep;
    zmq::message_t msg2(jobj.dump());
    msg2.set_routing_id(remid);
    auto sres = sock.send(msg2, zmq::send_flags::none);
    if (!sres) {
        chirp(cfg, "failed to send reply");
        return;
    }
}    

static void handle_reply(const ClientConfig& cfg, zmq::socket_t& sock)
{
    zmq::message_t msg;
    auto rres = sock.recv(msg, zmq::recv_flags::none);
    if (!rres) {
        chirp(cfg, "failed to recv request");
        return;
    }
    auto jobj = yamz::data_t::parse(msg.to_string());
    auto ttrep = jobj.get<yamz::TestTimeReply>();
    double dt1 = ut_dt(ttrep.reqtime, ttrep.reptime);
    double dt2 = ut_dt(ttrep.reptime, ut_now());
    chirp(cfg, "dt1: " << dt1 << " dt2: " << dt2);
}

// Ask the world for a time.  The client-like socket likely implements
// round-robin so this probes one linked server at a time.
static void make_request(const ClientConfig& cfg, zmq::socket_t& sock)
{
    yamz::TestTimeRequest ttr{ut_now()};
    yamz::data_t jobj = ttr;
    zmq::message_t msg(jobj.dump());

    auto res = sock.send(msg, zmq::send_flags::none);
    if (!res) {
        chirp(cfg, "failed to send request");
        return;
    }
    // chirp(cfg, "sent request");
}

// this thread function uses one yamz::Client.  It represents what
// might be a "component" or a "module" in some toolkits or
// frameworks.
void cluster_component(zmq::context_t& ctx, ClientConfig cfg)
{
    yamz::Client cli(ctx, cfg);
    auto& paskme = cli.get("askme");   // server-like port
    auto& paskyou = cli.get("askyou"); // client-like port
    auto& askme = paskme.sock;
    auto& askyou = paskyou.sock;


    while (paskyou.conns.empty()) {
        chirp(cfg, "wait for at least one connection");
        auto what = cli.discover();
        if (what == yamz::ClientAction::timeout) {
            std::this_thread::sleep_for(std::chrono::milliseconds{1000});
            continue;
        }
        if (what == yamz::ClientAction::terminate) {
            chirp(cfg, "terminate");
            return;
        }
        break;
    }

    askyou.set(zmq::sockopt::sndtimeo, 0);

    zmq::poller_t<> poller;
    poller.add(askme, zmq::event_flags::pollin);
    poller.add(askyou, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(2);
    std::chrono::milliseconds timeout{10000};
    while (true) {              // fixme: make way to break
        auto what = cli.discover();
        if (what == yamz::ClientAction::terminate) {
            chirp(cfg, "terminate");
            break;
        }


        //chirp(cfg, "poll on sockets");
        int nevents = 0;
        try {
            nevents = poller.wait_all(events, timeout);
        }
        catch (zmq::error_t& e) {
            chirp(cfg, "poll failure: " << e.what());
            break;
        }
        if (!nevents) {
            chirp(cfg, "timeout, make request");
            make_request(cfg, askyou);
            continue;
        }
        for (int iev = 0; iev < nevents; ++iev) {

            if (events[iev].socket == askme) { // get request
                //chirp(cfg, "got request");
                handle_request(cfg, askme);
            }

            if (events[iev].socket == askyou) { // get reply
                //chirp(cfg, "got reply");
                handle_reply(cfg, askyou);
            }
        }
        // fixme: add making a request
    }
    chirp(cfg, "exiting");
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: test_yamz_cluster config.json" << std::endl;
        return 0;               // don't consider this a failure
    }
    yamz::data_t jcfg;
    std::ifstream istr(argv[1]);
    istr >> jcfg;
    auto cfg = jcfg.get<yamz::TestJobCfg>();

    zmq::context_t ctx;

    yamz::Server server(ctx, cfg.server);
    server.start();

    std::vector<std::thread> cthreads;
    for (auto& cc : cfg.clients) {
        chirp(cc, "start client thread");
        cthreads.emplace_back([&ctx, cc]() {
            cluster_component(ctx, cc);
        });
    }
    
    while (true) {
        size_t ncanjoin = 0;
        for (auto& cli : cthreads) {
            if (cli.joinable()) ++ncanjoin;
        }
        if (ncanjoin == cthreads.size()) {
            std::cerr << "joining threads\n";
            for (auto& cli : cthreads) {
                cli.join();
            }
            break;
        }
        std::cerr << "joinable threads: "
                  << ncanjoin << "/" << cthreads.size() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});
    }
    std::cerr << "main exit \n";

    return 0;
}
