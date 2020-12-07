#ifndef YAMZ_SERVER_HPP
#define YAMZ_SERVER_HPP

#include <yamz/zeromq.hpp>
#include <yamz/Structs.hpp>

#include <vector>
#include <string>
#include <thread>
#include <exception>

namespace yamz {


    struct server_error : public std::runtime_error {
        server_error(const std::string& what) : std::runtime_error(what) {}
    };

    /** 
        yamz::Server provides a synchronous API to the server actor.

        It mediates between requests from yamz clients and a Zyre peer
        in order to allow the disparate clients to participate as a
        unified collective entity in the discovery process.  

        An application is typically expected to construct one
        yamz::Server and retain it for the process lifetime.  Though,
        multiple and/or transient yamz::Server may be created.
     */
    class Server {

      public:

        /** Create a yamz server.

            Bind to given addresses and run until all expected clients
            have made a request.  If expected is empty, rely on
            explicit online command.  

            Use of inproc:// transport requires that the context be
            shared when making client sockets.
        */
        Server(zmq::context_t& ctx, const ServerConfig& cfg = {});
        ~Server();
        
        /** Replace server configuration. */
        void configure(const ServerConfig& cfg);

        /** Start the server listening to requests from clients. */
        void start();

        /** Explicitly transition from requests mode to discovery mode.

            Once called, no further client requests will be accepted.

            Return false if any expected clients have not yet made a
            request.
        */
        bool discover();

        // Bundle info to simplify sharing with actor function.
        struct Params {
            zmq::context_t& ctx;
            ServerConfig cfg;
            std::string linkname{""}; 
        };

      private:

        std::thread athread;
        zmq::socket_t alink;
        Params params;

    };

} // namespace yamz

#endif // YAMZ_SERVER_HPP
