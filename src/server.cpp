#include "yamz/server.hpp"
#include "yamz/zyre.hpp"
#include "yamz/util.hpp"
#include "yamz/Nljs.hpp"

#include <map>

#include <iostream>             // debugging

// An RR collect a request and its reply.
struct RR {
    yamz::Request request{};
    yamz::Reply reply{};
};


// Manage outstanding requests
struct Outstanding {
    // Keyed by requester's remid
    using ident_rr_t = std::map<std::string, RR>;

    // Two RRs keyed by request ID.  The "open" set is filled by
    // requests and updated.  When satisfied, entires are moved to
    // done and from there they may be drained via ready().
    ident_rr_t open, done;

    // Index abstract node/comp/port names to concrete port
    using indexed_ports_t = std::map<std::string, yamz::ConcretePort>;

    // remember what ports we've seen advertized by peers
    indexed_ports_t history;

    void add(const std::string& remid, const yamz::Request& req) {
        open[remid] = RR{req, {req.comp}};
    }

    // Return true if we have a requets from component named by comp.
    bool have_comp(const std::string& comp) const {
        for (const auto& one : open) {
            if (one.second.request.comp == comp) {
                return true;
            }
        }
        return false;
    }

    // Return true if expected is empty or if all expected components
    // have made a request.
    bool have_all_comp(const std::vector<std::string>& expected) {
        if (expected.empty()) {
            return false;
        }
        for (const auto& one : expected) {
            if (have_comp(one)) continue;
            return false;
        }
        return true;
    }
    
    // Update outstanding requests given YAMZ header info from a peer
    // node.  The YAMZ header value is a list of entries like:
    // comp1/portA:PUB = tcp://127.0.0.1:8065, inproc://portA;
    void update(std::string node, std::string header) {
        indexed_ports_t seen;
        yamz::cportmap_t cportmap = yamz::parse_header(header);
        for (auto cportitem : cportmap) {
            auto comp = cportitem.first;
            for (auto cport : cportitem.second) {
                std::string key = node + "/" + comp + "/" + cport.portid;
                seen[key] = cport;
                history[key] = cport;
            }
        }
        std::vector<std::string> finished;
        for (auto& irr: open) {
            auto remid = irr.first;
            RR& rr = irr.second;
            auto cli_comp = rr.request.comp;
            for (auto& conne : rr.request.conns) {
                auto cli_port = conne.port;
                for (auto& want : conne.addrs) {
                    auto key = want.node + "/" + want.comp + "/" + want.port;
                    auto cpit = seen.find(key);
                    if (cpit == seen.end()) {
                        continue;
                    }
                    // this cli port wants this concrete port
                    rr.reply.conns.push_back(cpit->second);

                    if (rr.request.conns.size() == rr.reply.conns.size()) {
                        done[remid] = rr;
                        finished.push_back(remid);
                    }
                }
            }
        }
        for (auto die : finished) {
            open.erase(die);
        }
    }
    
    ident_rr_t ready() {
        ident_rr_t ret = done;
        done.clear();
        return ret;
    }
    
    // Header is a ;-separated list with entries like:
    // c/p:S=a,a,a;c/p:S=a,a,a;c/p:S=a,a,a;
    // comp1/portA:PUB = tcp://127.0.0.1:8065, inproc://portA;

    // Return a YAMZ header value for the requests
    std::string header() {
        std::string ret = "";
        for (auto& irr: open) {
            auto remid = irr.first;
            RR& rr = irr.second;
            auto cli_comp = rr.request.comp;
            std::string semicolon = "";
            for (auto& bind : rr.request.binds) {
                ret += semicolon;
                semicolon = ";";
                auto ztype = yamz::str(bind.ztype);
                ret += cli_comp + "/" + bind.portid + ":" + ztype + "=";
                std::string comma = "";
                for (auto& addr : bind.concs) {
                    ret += comma;
                    comma = ",";
                    ret += addr;
                }
            }
        }
        return ret;
    }
};


static
void server(yamz::Server::Params params)
{
    Outstanding rrs;

    zmq::socket_t link(params.ctx, zmq::socket_type::pair);
    link.connect(params.linkname);
    zmq::socket_t sock(params.ctx, zmq::socket_type::server);
    for (auto& addr : params.tobind) {
        std::cerr << "server actor: bind: " << addr << std::endl;
        sock.bind(addr);
    }

    // notify ready for caller to continue
    std::cerr << "server actor: sending ready" << std::endl;
    link.send(zmq::message_t{}, zmq::send_flags::none);

    zmq::poller_t<> poller;
    poller.add(link, zmq::event_flags::pollin);
    poller.add(sock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(2);
    bool receiving = true;
    bool terminated = false;
    const std::chrono::milliseconds forever{-1};
    while (receiving && !terminated) {
        std::cerr << "server actor: polling first stage" << std::endl;
        const int nevents = poller.wait_all(events, forever);
        std::cerr << "server actor: poll found " << nevents << std::endl;
        for (int iev = 0; iev < nevents; ++iev) {

            // actor protocol
            if (events[iev].socket == link) {
                std::cerr << "server actor: link hit" << std::endl;
                zmq::message_t msg;
                auto res = link.recv(msg);
                if (!res) {
                    throw yamz::server_error("Failed to recv on link");
                }
                auto cmd = msg.to_string();
                std::cerr << "server actor: command: " << cmd << std::endl;
                if (cmd == "ONLINE") {
                    std::string rep = "FAIL";
                    if (rrs.have_all_comp(params.expected)) {
                        rep = "OKAY";
                        receiving = false;
                    }
                    std::cerr << "server actor: send phase 1 ONLINE ack: "
                              << rep << std::endl;
                    auto sres = link.send(zmq::message_t(rep),
                                          zmq::send_flags::none);
                    if (!sres) {
                        throw yamz::server_error("Failed to ack ONLINE "+rep);
                    }
                    continue;
                }
                std::cerr << "server actor: terminate" << std::endl;
                terminated = true;
            }

            // recieve requests protocol
            if (events[iev].socket == sock) {
                zmq::multipart_t mmsg;
                auto res = mmsg.recv(sock);
                if (!res) {
                    throw yamz::server_error("Failed to recv on sock");
                }
                auto remid = mmsg.popstr();
                mmsg.pop();     // delimeter
                zmq::message_t msg = mmsg.pop();
                
                auto sreq = msg.to_string();
                auto req = yamz::data_t::parse(sreq).get<yamz::Request>();
                rrs.add(remid, req);

                if (rrs.have_all_comp(params.expected)) {
                    receiving = false;
                    break;
                }

                continue;
            }
        }
    }

    if (terminated) {
        zmq::message_t die;
        auto res = link.send(die, zmq::send_flags::none);
        res = {};               // don't care
        return;
    }


    yamz::Zyre zyre(params.nodename);
    zyre.set_header("YAMZ", rrs.header().c_str());
    auto zsock = zyre.socket();

    zmq::poller_t<> poller2;
    poller2.add(link, zmq::event_flags::pollin);
    poller2.add(zsock, zmq::event_flags::pollin);

    zyre.start();
    receiving = true;
    while (receiving && !terminated) {
        std::cerr << "server actor: polling second stage" << std::endl;
        const int nevents = poller2.wait_all(events, forever);
        for (int iev = 0; iev < nevents; ++iev) {

            // actor protocol
            if (events[iev].socket == link) {
                zmq::message_t msg;
                auto res = link.recv(msg);
                if (!res) {
                    throw yamz::server_error("Failed to recv on link");
                }
                auto cmd = msg.to_string();
                if (cmd == "ONLINE") {
                    std::cerr << "server actor: got ONLINE cmd in phase 2" << std::endl;
                    std::string rep = "FAIL";
                    if (rrs.have_all_comp(params.expected)) {
                        rep = "OKAY";
                    }
                    auto sres = link.send(zmq::message_t(rep),
                                          zmq::send_flags::none);
                    if (!sres) {
                        throw yamz::server_error("Failed to ack ONLINE "+rep);
                    }
                    continue;
                }
                std::cerr << "server actor: terminate" << std::endl;
                terminated = true;
                break;
            }

            // got zyre event
            if (events[iev].socket == zsock) {
                auto zev = zyre.event();
                if (zev.type() == "ENTER") {
                    auto nodename = zev.peer_name();
                    std::string header = zev.header("YAMZ");
                    rrs.update(nodename, header);
                    if (rrs.have_all_comp(params.expected)) {
                        receiving = false;
                        break;
                    }
                }
            }
        }
    }

    if (!terminated) {
        auto replies = rrs.ready();
        for (auto& one : replies) {
            auto remid = one.first;
            auto& rr = one.second;
            yamz::data_t jdat = rr.reply;
            zmq::multipart_t mmsg(jdat.dump());
            mmsg.pushmem(NULL, 0);  // delimter
            mmsg.pushstr(remid);
            mmsg.send(sock);
        }

        // wait for death signal
        zmq::message_t die;
        auto res = link.recv(die);
        res = {};                   // don't care
    }
    zmq::message_t die;
    auto res = link.send(die, zmq::send_flags::none);    
    res = {};                   // don't care.
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

    // CZMQ compatible convention, wait for actor to signal ready
    zmq::message_t msg;
    auto res = alink.recv(msg);
    if (! res) {
        throw yamz::server_error("Failed to get server actor ready");
    }
    std::cerr << "server: actor started" << std::endl;
}

bool yamz::Server::discover()
{
    std::string ol = "ONLINE";
    zmq::message_t msg(ol);
    std::cerr << "server: going into discovery mode" << std::endl;
    auto res = alink.send(msg, zmq::send_flags::none);
    if (!res) {
        throw yamz::server_error("Failed to send ONLINE");
    }

    auto sres = alink.recv(msg);
    if (!sres) {
        throw yamz::server_error("Failed to recv ONLINE ack");
    }
    std::cerr << "server: discovery mode: " << msg.to_string() << std::endl;

    return msg.to_string() == "OKAY";
}

yamz::Server::~Server()
{
    // We follow CZMQ convention to send a strangely spelled message
    // to tell actor to cease operation and exit cleanly.
    auto sres = alink.send(zmq::message_t("$TERM", 5),
                           zmq::send_flags::dontwait);
    if (sres) {                 // actor is alive
        zmq::message_t msg;
        auto res = alink.recv(msg);
        res = {};               // don't care
    }
    athread.join();
}

