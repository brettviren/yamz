#include "server_zeromq.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>
#include <yamz/Nljs.hpp>

using namespace yamz::server;

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
 
