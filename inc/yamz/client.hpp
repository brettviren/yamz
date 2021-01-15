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
           Create a yamz::Client

           This will immediately connect to servers (which of course
           need not be around yet) as per ClientConfig.
           
           ZeroMQ context must be same as used for yamz::Server if
           inproc:// is used.

           Sockets for ports described in ClientConfig will be made
           immediately.
        */
        Client(zmq::context_t& ctx, const ClientConfig& cfg = {});
        ~Client();

        Client(const Client&) = delete;
        Client operator=(const Client&) = delete;

        /** Make a port.
         *
         * Adds port entry to the ClientConfig that will be sent as
         * server request.
         *
         * Return true if new port is made, false if it existed.
         *
         * A co responding @ref PortInfo object may be retrieved via
         * @ref get().
         */
        bool make_port(std::string portid, int stype);

        /** Register a bind address to an existing port.
         *
         * Address may be ephemeral or concrete.
         *
         * A socket bind() is performed immediately and the resulting
         * concrete port is added to the PortInfo.
         */
        void bind(std::string portid, std::string addr);

        /** Register a connect address to an existing port.
         *
         * Address may be abstract or concrete.
         *
         * Socket connect will be performed based on server replies.
         *
         */
        void connect(std::string portid, std::string addr);

        /** Make request to server.  
         *
         * This call is idempotent and will only make a single request
         * if called multiple times.
         *
         * Any subseqent bind or connect will not make it to the server.
         */
        void make_request();

        /** Check for, recv and process a reply queued from server.
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

        /** Access the socket possibly linked to yamz::Servers.
         *
         * The application may use this socket in a poller to know
         * precisely when a message from the server is delivered.  The app may
         * then call discover() to process that event.  App shall not
         * directly recv() the message.  The app may instead of polling elect to
         * call discover() periodically and rely on a timeout.
         */
        zmq::socket_t& socket() { return clisock; }

        /** A PortInfo collects information about a socket and its
         * concrete address. */
        struct PortInfo {
            // The portid 
            std::string name;
            // The socket object
            zmq::socket_t sock;
            // Index into ClientConfig::ports
            size_t ccpind;
            // Remember concrete addresses of binds and connects
            std::vector<std::string> binds, conns;
        };

        /** Return named port
         *
         *  Throws client_error if port does not exist.
         */
        PortInfo& get(std::string portid);

    private:
        zmq::context_t& ctx;
        ClientConfig cfg;

        zmq::socket_t clisock;
        std::map<std::string, PortInfo> portinfos;

        // Break up some steps but these are all called in constructor.
        void connect_server();
        void make_ports();
        void do_binds();

        bool requested{false};
    };

}
#endif
