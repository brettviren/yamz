#ifndef YAMZ_SERVER_ZEROMQ_HPP
#define YAMZ_SERVER_ZEROMQ_HPP

#include "yamz/zyre.hpp"
#include "yamz/zeromq.hpp"
#include "yamz/Structs.hpp"
#include "server_data.hpp"

namespace yamz::server {



    // Parse a zyre event to fill a RemoteAddress
    std::vector<RemoteAddress> from_zyre(yamz::ZyreEvent& zev);

    // Inform zyre of the headers.
    void tell_zyre(yamz::Zyre& zyre, const YamZyreHeaders& headers);

    // Recieve from socket, filling cc, return remid
    remid_t recv(zmq::socket_t& sock, yamz::ClientConfig& cc);

    // Send to socket
    void send(zmq::socket_t& sock, remid_t rid, const yamz::ClientConfig& cc);

}

#endif
