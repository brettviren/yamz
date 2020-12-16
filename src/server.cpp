#include "yamz/server.hpp"

#include "server_actor.hpp"
#include "server_logic.hpp"

#include <iostream>             // debugging


yamz::Server::Server(zmq::context_t& ctx,
                     const ServerConfig& cfg)
    : ctx{ctx}, cfg{cfg}
{
}

void yamz::Server::start()
{
    std::stringstream ss;
    ss << "inproc://yamz-server-"
       << cfg.nodeid << "-" << cfg.portnum << "-"
       << std::hex << this;
    std::string linkname = ss.str();

    alink = zmq::socket_t(ctx, zmq::socket_type::pair);
    alink.bind(linkname);

    yamz::server::ActorArgs aa {ctx, cfg, linkname};
    // athread = std::thread([aa](){
    //     yamz::server::actor(aa);
    // });
    athread = std::thread(yamz::server::actor,aa);

    
    // CZMQ compatible convention, wait for actor to signal ready
    zmq::message_t msg;
    auto res = alink.recv(msg);
    if (! res) {
        throw yamz::server_error("Failed to get server actor ready");
    }
    std::cerr << "server: actor started" << std::endl;
}

bool yamz::Server::online()
{
    auto got = command("ONLINE");
    return got == "OKAY";
}
bool yamz::Server::offline()
{
    auto got = command("OFFLINE");
    return got == "OKAY";
}

std::string yamz::Server::command(std::string cmd)
{
    zmq::message_t msg(cmd);
    auto res = alink.send(msg, zmq::send_flags::none);
    if (!res) {
        throw yamz::server_error("Failed to send " + cmd);
    }

    auto sres = alink.recv(msg);
    if (!sres) {
        throw yamz::server_error("Failed to recv ack for " + cmd);
    }
    return msg.to_string();
}

yamz::Server::~Server()
{
    std::cerr << "server: destructing" << std::endl;

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

