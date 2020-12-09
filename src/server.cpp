#include "yamz/server.hpp"
#include "yamz/zyre.hpp"
#include "yamz/util.hpp"
#include "yamz/Nljs.hpp"

#include "server_data.hpp"
#include "server_actor.hpp"

#include <map>
#include <regex>

#include <iostream>             // debugging


yamz::Server::Server(zmq::context_t& ctx,
                     const ServerConfig& cfg)
    : params{ctx, cfg}
{
    std::stringstream ss;
    ss << "inproc://yamz-server-"
       << cfg.nodeid << "-" << cfg.portnum << "-"
       << std::hex << this;
    params.linkname = ss.str();
}

void yamz::Server::start()
{
    alink = zmq::socket_t(params.ctx, zmq::socket_type::pair);
    alink.bind(params.linkname);

    athread = std::thread(yamz::server::server_actor, params);

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

