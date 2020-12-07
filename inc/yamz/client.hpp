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

        /** Update the configuration with another.
         */
        void configure(const ClientConfig& cfg);

        /** Make discovery request to yamz::Server.
            
            Return true if a the server responded before the timeout.
            A negative timeout will block forever.

            This method may and should be called subsequent times
            until returning true in order to assure a reply from the
            server is received.

            This method must be called in the same thread as may later call
            get().
         */
        bool discover(std::chrono::milliseconds timeout=std::chrono::milliseconds(-1));

        /** Publish binds to server request but do not wait for any reply.

            This requires all connect addresses to be concrete.

            This should only be called once and excludes calling discover().

            This method must be called in the same thread as may later call
            get().
        */
        void publish();

        /** Eschew discovery entirely directly make sockets.
            
            This requires all connect addresses to be concrete.

            This shoudl only be called once and excludes calling discovery.

            This method must be called in the same thread as may later call
            get().
         */
        void direct();

        /** Return socket associated with named port
            
            This method must be called in the same thread as which called
            discover(), publish() or direct().
         */
        zmq::socket_ref get(std::string port);

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
        void try_recv(std::chrono::milliseconds timeout);
        bool recved{false};
        void make_request();
        bool requested{false};
        void do_conns();
        bool conned{false};
    };

}
#endif
