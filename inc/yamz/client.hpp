#ifndef YAMZ_CLIENT_HPP
#define YAMZ_CLIENT_HPP

#include <yamz/zeromq.hpp>
#include <yamz/Structs.hpp>

#include <string>
#include <map>
#include <exception>

namespace yamz {

    
    struct client_error : public std::runtime_error {
        client_error(const std::string& what) : std::runtime_error(what) {}
    };

    /**
       yamz::Client provides no fuss access to discovery and resulting
       ready-to-use sockets.

       It is a semi-synchronous API.  See discover().
    */
    class Client {
      public:

        /** 
           Create yamz::Client connection to a yamz::Server.
           
           Context must be same as used for yamz::Server if inproc://
           is used.

           The name give the component identifier used in discovery.
        */
        Client(zmq::context_t& ctx, const ClientConfig& cfg = {});
        ~Client();

        Client(const Client&) = delete;
        Client operator=(const Client&) = delete;

        /** Check for any replies from server.
         *  
         *  Return true if a the server responded before the timeout.
         *  A negative timeout will block forever.
         *
         *  This method must be called periodically in order to
         *  discover any newly arrived peers.
         *
         *  For general thread-safe use of the sockets, this method
         *  must be called in the same thread as may later call get().
         */
        bool discover(std::chrono::milliseconds
                      timeout=std::chrono::milliseconds(0));


        /** Return reference to socket associated with named port
         *
         *  Throws client_errror if port does not exist.
         *
         *  For general thread-safe use of the socket, this method
         *  must be called in the same thread as which called
         *  discover().
         */
        zmq::socket_t& get(std::string port);

    private:
        zmq::context_t& ctx;
        ClientConfig cfg;

        zmq::socket_t clisock;
        std::map<std::string, zmq::socket_t> socks;

        // Break up client-side steps of yamz c/s protocol 
        void connect_server();
        // -->sets clisock
        void make_socks();
        // -->sets socks.size() != 0
        void do_binds();
        bool bound{false};
        void make_request();
        bool requested{false};
    };

}
#endif
