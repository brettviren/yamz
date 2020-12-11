#include "server_zeromq.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>
#include <yamz/Nljs.hpp>

using namespace yamz::server;


remid_t yamz::server::recv(zmq::socket_t& sock, yamz::ClientConfig& cc)
{
    zmq::message_t msg;
    auto res = sock.recv(msg);
    if (!res) {
        throw yamz::server_error("failed to receive from client");
    }

    // note, this is for SERVER, if ROUTER, we take from message.
    remid_t rid = msg.routing_id();

    auto sreq = msg.to_string();
    auto jobj = yamz::data_t::parse(sreq);
    cc = jobj.get<yamz::ClientConfig>();
    return rid;
}
void send(zmq::socket_t& sock, remid_t rid, const yamz::ClientConfig& cc)
{
    yamz::data_t jobj = cc;
    zmq::message_t msg(jobj.dump());

    // note, this is for SERVER, if ROUTER, we put to message.
    msg.set_routing_id(rid);

    auto res = sock.send(msg, zmq::send_flags::none);
    if (!res) {
        std::stringstream ss;
        ss << "failed to send to client " << rid;
        throw yamz::server_error(ss.str());
    }
}
 
