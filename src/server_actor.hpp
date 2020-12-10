/**
   This holds the yamz::Server actor function.

   It makes use of the otherwise stand-alone server_logic.hpp.

   It is responsible for communicating to yamz::Client instances and
   Zyre.
 */

#ifndef YAMZ_SERVER_ACTOR_HPP
#define YAMZ_SERVER_ACTOR_HPP

#include <yamz/server.hpp>

namespace yamz::server {

    /*
     * The actor function providing asynchronous service to the
     * yamz::Server API.
     */
    struct ActorArgs {
        zmq::context_t& ctx;
        const yamz::ServerConfig& cfg;
        const std::string& linkname;
    };
    void actor(ActorArgs& aa);

}

#endif
