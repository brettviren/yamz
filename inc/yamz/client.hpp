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

        /** Check for, recv and process replies queued from server.
         *  
         *  A negative timeout will block until at one message is processed.
         *
         *  Return the client action as given by the server. 
         *
         * - connect :: a new connection has been made
         * - disconnect :: a disconnection has been made
         * - terminate :: the server has terminated (probably client should too)
         * - timeout :: timeout occured with no messages processed
         *
         *  This method must be called in order for the client to
         *  react to any changes in the network of peers.  
         *
         *  May throw client_error.
         */
        yamz::ClientAction
        discover(std::chrono::milliseconds
                 timeout=std::chrono::milliseconds(0));

        /** Access the client socket.
         *
         * The application may use this socket in a poller to know
         * precisely when a message from the server is delivered.  The app may
         * then call discover() to process that event.  App shall not
         * directly recv() the message.  The app may instead of polling elect to
         * call discover() periodically and rely on a timeout.
         */
        zmq::socket_t& socket() { return clisock; }


        struct PortInfo {
            // The portid 
            std::string name;
            // The socket object
            zmq::socket_t sock;
            // remember concrete addresses of binds and connects
            std::vector<std::string> binds, conns;
        };

        /** Return named port
         *
         *  Throws client_errror if port does not exist.
         *
         *  For general thread-safe use of the socket, this method
         *  must be called in the same thread as which called
         *  discover().
         */
        PortInfo& get(std::string port);

    private:
        zmq::context_t& ctx;
        ClientConfig cfg;

        zmq::socket_t clisock;
        std::map<std::string, PortInfo> ports;

        // Break up client-side steps of yamz c/s protocol 
        void connect_server();
        void make_ports();
        void do_binds();
        bool bound{false};
        void make_request();
        bool requested{false};
    };

}
#endif
