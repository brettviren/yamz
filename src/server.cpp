#include "yamz/server.hpp"
#include "yamz/zyre.hpp"
#include "Nljs.hpp"

#include <map>

struct RR {
    yamz::Request request{};
    yamz::Reply reply{};
};
using rrs_t = std::map<std::string, RR>;

static 
bool have_expected(const std::vector<std::string>& expected,
                   const rrs_t& rrs)
{
    if (expected.empty()) {
        return false;
    }
    for (const auto& one : expected) {
        if (rrs.find(one) == rrs.end()) {
            return false;
        }
    }
    return true;
}

// given YAMZ data from Zyre header from node, see if it is applicable
// to any rrs and update.  If any rr is fullfilled send a reply to
// client and remove the entry from the rrs.
static
void update(zmq::socket_t& sock,
            std::string your_node,
            yamz::data_t& jdata,
            rrs_t& rrs)
{
    // this deep loop could be unrolled by indexing the queries by
    // peer info....

    // fixme: what do when peer EXITs?

    //         jdata[rr.first] = rr.second.request.binds;
    for (auto& jcomp : jdata.items()) {
        std::string your_comp = jcomp.key();
        for (auto& jbind: jcomp.value()) {
            // .port, .type, .addrs            
            auto cport = jbind.get<yamz::ConcretePort>();
            auto your_port = cport.port;
            // scan if anyone wants (node,comp,port)
            for (auto& rrit : rrs) {
                const auto my_portname = rrit.first;
                auto& rr = rrit.second;

                for (auto& aport : rr.request.conns) {
                    // aport.port
                    for (auto& aaddr : aport.addrs) {
                        if (your_node != aaddr.node) { continue; }
                        if (your_comp != aaddr.comp) { continue; }
                        if (your_port != aaddr.port) { continue; }
                        // bingo
                        rr.reply.conns.push_back(cport);
                    }
                }


            // if any rr is interested then update
            // if this fullfills the rr send reply and remove

            }
        }
    }
}


static
void server(yamz::Server::Params params)
{
    rrs_t rrs;

    zmq::socket_t link(params.ctx, zmq::socket_type::pair);
    link.connect(params.linkname);
    // notify ready for caller to continue
    link.send(zmq::message_t{}, zmq::send_flags::none);

    zmq::socket_t sock(params.ctx, zmq::socket_type::server);
    for (auto& addr : params.tobind) {
        sock.bind(addr);
    }
    zmq::poller_t<> poller;
    poller.add(link, zmq::event_flags::pollin);
    poller.add(sock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(2);
    bool receiving = true;
    const std::chrono::milliseconds forever{-1};
    while (receiving) {
        const int nevents = poller.wait_all(events, forever);
        for (int iev = 0; iev < nevents; ++iev) {

            // actor protocol
            if (events[iev].socket == link) {
                zmq::message_t msg;
                auto res = sock.recv(msg);
                if (!res) {
                    // fixme: handle better
                    return;
                }

                auto cmd = msg.to_string();
                if (cmd == "ONLINE") {
                    std::string rep = "OKAY";
                    if (!have_expected(params.expected, rrs)) {
                        rep = "FAIL";
                    }
                    auto sres = link.send(zmq::message_t(rep),
                                          zmq::send_flags::none);
                    if (!sres) {
                        // fixme: handle better
                        return;
                    }
                    receiving = false;
                    break;
                }
                return;         // terminated
            }

            // recieve requests protocol
            if (events[iev].socket == sock) {
                zmq::message_t msg;
                auto res = sock.recv(msg);
                if (!res) {
                    // fixme: handle better
                    return;
                }
                // fixme: add ROUTER id storage!
                assert(false);

                auto sreq = msg.to_string();
                auto req = yamz::data_t::parse(sreq).get<yamz::Request>();

                rrs[req.comp] = RR{req, {req.comp}};
                if (have_expected(params.expected, rrs)) {
                    receiving = false;
                    break;
                }

                // collect request
                // store by ident
                // check if have expected and expected fullfilled
                continue;
            }
        }
    }

    yamz::Zyre zyre(params.nodename);

    // Build up zyre peer info.  We make a single header called YAMZ
    // which holds JSON rep like [<ident>][<port>][addr1,addr2,...].
    // This is not in keeping HTTP header conventions but, a) the
    // "standard" conventions for header syntax are all over the map
    // and b) the structured header proposal uses a goofy lookin'
    // syntax that I don't want to parse and c) this isn't HTTP.
    yamz::data_t jheadval;
    for (auto& rr : rrs) {
        jheadval[rr.first] = rr.second.request.binds;
    }
    zyre.set_header("YAMZ", jheadval.dump());
    auto zsock = zyre.socket();

    zmq::poller_t<> poller2;
    poller2.add(link, zmq::event_flags::pollin);
    poller2.add(zsock, zmq::event_flags::pollin);

    zyre.start();
    receiving = true;
    while (receiving) {
        const int nevents = poller2.wait_all(events, forever);
        for (int iev = 0; iev < nevents; ++iev) {

            // actor protocol
            if (events[iev].socket == link) {
                zmq::message_t msg;
                auto res = sock.recv(msg);
                if (!res) {
                    // fixme: handle better
                    return;
                }
                auto cmd = msg.to_string();
                if (cmd == "ONLINE") {
                    std::string rep = "OKAY";
                    if (!have_expected(params.expected, rrs)) {
                        rep = "FAIL";
                    }
                    auto sres = link.send(zmq::message_t(rep),
                                          zmq::send_flags::none);
                    if (!sres) {
                        // fixme: handle better
                        return;
                    }
                    continue;
                }
                return;         // terminated
            }

            // got zyre event
            if (events[iev].socket == zsock) {
                auto zev = zyre.event();
                if (zev.type() == "ENTER") {
                    auto nodename = zev.peer_name();
                    auto jdata = yamz::data_t::parse(zev.header("YAMZ"));
                    update(sock, nodename, jdata, rrs);
                    if (rrs.size()) {
                        continue;
                    }
                    receiving = false;
                    break;
                }
            }
        }
    }
    // poll on link + zyre
    // on link, bail unless get ONLINE, then return status of have_expected().
    // on zyre, check for matches
    // record matches
    // check of full filled matches
    // respond and close out request
        
    // wait for death signal
    zmq::message_t die;
    auto res = link.recv(die);
    if (!res) {
        // fixme: handle better;
        return;
    }
    return;
}
                

yamz::Server::Server(zmq::context_t& ctx,
                     std::vector<std::string> tobind)
    : params{ctx, "", tobind}
{
    std::stringstream ss;
    ss << "inproc://yamz-server-" << std::hex << this;
    params.linkname = ss.str();
}

void yamz::Server::set_name(std::string nodename)
{
    params.nodename = nodename;
}

void yamz::Server::set_port(int port)
{
    params.port = port;
}

void yamz::Server::start(std::vector<std::string> expected)
{
    params.expected = expected;

    alink = zmq::socket_t(params.ctx, zmq::socket_type::pair);
    alink.bind(params.linkname);
    athread = std::thread(server, params);

    // fixme: wait on signal
    // alink.recv()
}

bool yamz::Server::discover()
{
    // send ONLINE to link
    // read back OKAY/FAIL
    // return bool
    return false;
}

yamz::Server::~Server()
{
    // send shutdown signal
    // wait for death signal
    // join thread
}

