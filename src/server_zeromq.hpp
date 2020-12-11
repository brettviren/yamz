#ifndef YAMZ_SERVER_ZEROMQ_HPP
#define YAMZ_SERVER_ZEROMQ_HPP

#include "yamz/zyre.hpp"
#include "yamz/zeromq.hpp"
#include "yamz/Nljs.hpp"
#include "yamz/server.hpp"

namespace yamz::server {

    using remid_t = uint32_t;

    template<typename Type>
    remid_t recv_type(zmq::socket_t& sock, Type& obj) {
        zmq::message_t msg;
        auto res = sock.recv(msg, zmq::recv_flags::none);
        if (!res) { throw yamz::server_error("failed to receive"); }

        // note, we assume SERVER, not ROUTER.
        remid_t rid = msg.routing_id();

        auto sreq = msg.to_string();
        auto jobj = yamz::data_t::parse(sreq);
        obj = jobj.get<Type>();
        return rid;
    }

    // Recieve from socket, filling cc, return remid
    remid_t recv(zmq::socket_t& sock, yamz::ClientConfig& cc);

    // Send to socket
    void send(zmq::socket_t& sock, remid_t rid, const yamz::ClientConfig& cc);

}

#endif
