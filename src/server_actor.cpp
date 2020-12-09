#include "server_actor.hpp"
#include "server_zeromq.hpp"
#include "server_logic.hpp"

#include <yamz/server.hpp>

#include <iostream>

using namespace yamz::server;

static
std::string link_hit(ServerLogic& guts, zmq::socket_t& link)
{
    zmq::message_t msg;
    auto res = link.recv(msg, zmq::recv_flags::none);
    if (!res) {
        throw yamz::server_error("Failed to recv on link");
    }
    auto cmd = msg.to_string();
    std::cerr << "server actor: command: " << cmd << std::endl;
    if (cmd == "ONLINE") {
        std::string rep = "OKAY";
        if (guts.outstanding()) {
            rep = "FAIL";
        }
        guts.set_expected();    // link tells us so
        std::cerr << "server actor: send phase 1 ONLINE ack: "
                  << rep << std::endl;
        auto sres = link.send(zmq::message_t(rep),
                              zmq::send_flags::none);
        if (!sres) {
            throw yamz::server_error("Failed to ack ONLINE " + rep);
        }
        return "ONLINE";
    }
    // we should terminate asap
    return "$TERM";
}

static
void request_hit(ServerLogic& guts, zmq::socket_t& sock)
{
    // yamz::ClientConfig cc;
    // auto remid = yamz::server::recv(sock, cc);
    // guts.accept_peer(remid, cc);
}

// returns false to terminate
static
std::string request_phase(ServerLogic& guts,
                          zmq::socket_t& link, zmq::socket_t& sock)
{
    zmq::poller_t<> poller;
    poller.add(link, zmq::event_flags::pollin);
    poller.add(sock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(2);
    bool receiving = true;
    while (receiving) {
        const int nevents = poller.wait_all(events, std::chrono::milliseconds{-1});
        for (int iev = 0; iev < nevents; ++iev) {

            if (events[iev].socket == link) {
                // link hit always kills request phase
                // it's either go ONLINE or terminate.
                return link_hit(guts, link);
             }

            if (events[iev].socket == sock) {
                request_hit(guts, sock);
            }
        }
        receiving = guts.outstanding();
    }
    return "DONE";
}

static void terminate(zmq::socket_t& link, bool wait)
{
    // should wait unles it was API that told us to die
    if (wait) {               
        zmq::message_t die;
        auto res = link.recv(die);
        res = {};               // don't care
    }

    // ack
    zmq::message_t die;
    auto res = link.send(die, zmq::send_flags::none);    
    res = {};                   // don't care.
}

static void zyre_hit(ServerLogic& guts, yamz::Zyre& zyre)
{
    auto zev = zyre.event();
    auto ras = yamz::server::from_zyre(zev);
    if (ras.empty()) {
        return;
    }

    for (auto& ra : ras) {
        guts.accept_peer(ra);
    }
}

static std::string discovery_phase(ServerLogic& guts, zmq::socket_t& link)
{
    yamz::Zyre zyre(guts.config.nodeid);
    yamz::server::tell_zyre(zyre, guts.headers);
    auto zsock = zyre.socket();

    zmq::poller_t<> poller;
    poller.add(link, zmq::event_flags::pollin);
    poller.add(zsock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(2);

    zyre.start();

    bool receiving = true;
    while (receiving) {
        const int nevents = poller.wait_all(events, std::chrono::milliseconds{-1});
        for (int iev = 0; iev < nevents; ++iev) {

            if (events[iev].socket == link) {
                // link hit always kills request phase
                // it's either go ONLINE or terminate.
                auto got = link_hit(guts, link);
                if (got == "$TERM") {
                    return got;
                }
                // ONLINE was just a query at this point
            }

            if (events[iev].socket == zsock) {
                zyre_hit(guts, zyre);
            }

            
        }
    }
    return "";
}

static std::string reply_phase(ServerLogic& guts,
                               zmq::socket_t& link, zmq::socket_t& sock)
{
    return "";
}

void yamz::server::server_actor(yamz::Server::Params params)
{
    ServerLogic guts(params.cfg);

    // Link back to synchronous API
    zmq::socket_t link(params.ctx, zmq::socket_type::pair);
    link.connect(params.linkname);

    // Server socket
    zmq::socket_t sock(params.ctx, zmq::socket_type::server);
    for (auto& addr : params.cfg.addresses) {
        std::cerr << "server actor: bind: " << addr << std::endl;
        sock.bind(addr);
    }    

    // notify API it can continue
    std::cerr << "server actor: sending ready" << std::endl;
    link.send(zmq::message_t{}, zmq::send_flags::none);

    {
        auto got = request_phase(guts, link, sock);
        if (got == "$TERM") {
            terminate(link, false);
            return;
        }
        // ONLINE or DONE, just keep going
    }

    {
        auto got = discovery_phase(guts, link);
        if (got == "$TERM") {
            terminate(link, false);
            return;
        }
    }        

    reply_phase(guts, link, sock);
    
}

