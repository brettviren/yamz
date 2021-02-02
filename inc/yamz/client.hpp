#ifndef YAMZ_CLIENT_HPP
#define YAMZ_CLIENT_HPP

#include <yamz/zeromq.hpp>
#include <yamz/Structs.hpp>

#include <string>
#include <map>
#include <exception>

namespace yamz {

    
    class Server;

    struct client_error : public std::runtime_error {
        client_error(const std::string& what) : std::runtime_error(what) {}
    };

    /**
       yamz::Client provides no fuss access to discovery and resulting
       ready-to-use sockets.

       When used with a server the client provides a semi-synchronous
       API.  See poll().
    */
    class Client {
      public:

        /** 
           Create a yamz::Client with configuration.  

           May operate in any mode (see below) depending on
           configuration content.
        */
        Client(zmq::context_t& ctx, const ClientConfig& cfg);
        /** 
           Create a yamz::Client with clientid / name.

           With no other later configuration would operate in
           selfserve mode.
        */
        Client(zmq::context_t& ctx, const std::string& clientid);
        /** 
           Create an "empty" yamz::Client.

           With no other later configuration would operate in direct
           mode.
        */
        Client(zmq::context_t& ctx);

        ~Client();

        Client(const Client&) = delete;
        Client operator=(const Client&) = delete;

        /** Set client ID name.
         *
         * Non-empty string upgrades mode to selfserve or extserver.
         *
         * If used, must call before initialize().
         */
        void set_clientid(const std::string& clientid) {
            m_cfg.clientid = clientid;
        }

        /** Add a YAMZ server address.
         *
         * Non-empty server list upgrades to extserver mode.
         *
         * If used, must call before initialize().
         */
        void add_server(const std::string& server_address) {
            m_cfg.servers.push_back(server_address);
        }

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

        /** Initiate the client process.
         *
         * The operational mode is returned.
         *
         * This call is idempotent.  And any subsequent calls to
         * bind() or connect() may be ignored.
         *
         * The operation mode is determined by the configuration state
         * at the time of call.  The modal behavior is largely not
         * exposed to the caller but has implications to the network.
         * 
         * The three operation modes are:
         *
         * - direct :: no yamz::Server is involved and only concrete
         *   addresses are servicable.  This behavior is followed
         *   neither server nor clientid are configured.
         *
         * - selfserve :: a yamz::Server is created inside and
         *   dedicated to the client, the clientid serves as the Zyre
         *   node name.  This behavior is followed if a clientid and
         *   no servers are configured.
         *
         * - extserver :: a yamz::Server is supplied external to
         *   the client, typically by some application code near
         *   main().  This behavior is followed if both clientid and
         *   servers are configured.
         *
         */
        enum class Mode { uninitialized=0, direct, selfserve, extserver };
        Mode initialize();

        /** Access the mode.  
         *
         * This will "uninitialized" prior to call of initialize()
         * after which it will be and remain one of the other allowed
         * values for Mode.
         */
        Mode mode() const { return m_mode; }

        /** Poll for, recv and process at most one reply from server.
         *  
         * The return value is a yamz::ClientAction:
         *
         * - connect :: a new connection has been made
         * - disconnect :: a disconnection has been made
         * - terminate :: the server has terminated (probably client should too)
         * - timeout :: timeout occured with no messages processed
         *
         *  In mode "uninitialized" or "direct", this immediately and
         *  always returns "timeout".  In the case of "direct" mode,
         *  any direct connects will have already been applied.
         *
         *  In modes "selfserve" or "extserver":
         *
         *  A negative timeout will block until at one message is
         *  processed.
         *
         *  When returned action is "connect" or "disconnect" the
         *  client will have just applied the action to the
         *  coresponding socket.
         *
         *  See also last_reply().
         *
         *  The poll() method must be called in order for the
         *  client to react to any changes in the network of peers.
         *
         *  May throw client_error.
         */
        yamz::ClientAction
        poll(std::chrono::milliseconds
             timeout=std::chrono::milliseconds(0));

        /** Return the full content of last received reply */
        yamz::ClientReply last_reply() const { return m_last_reply; }

        /** Access the socket possibly linked to yamz::Servers.
         *
         * The application may use this socket in a poller to know
         * precisely when a message from the server is delivered.  The
         * app may then call poll() to process that event.  App
         * shall not directly recv() the message.  The app may instead
         * of polling elect to call poll() periodically and rely
         * on a timeout.
         */
        zmq::socket_t& socket() { return m_clisock; }

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

        /** Return info about named port
         *
         *  Throws client_error if port does not exist.
         */
        PortInfo& get(std::string portid);
        const PortInfo& get(std::string portid) const;

        using PortInfos = std::map<std::string, PortInfo>;
        const PortInfos& portinfos() const { return m_portinfos; }

        std::vector<std::string> portnames() const {
            std::vector<std::string> ret;
            for (const auto& pi : m_portinfos) {
                ret.push_back(pi.first);
            }
            return ret;
        }

        const ClientConfig& config() const { return m_cfg; }
        ClientConfig& config() { return m_cfg; }

    private:
        zmq::context_t& m_ctx;
        ClientConfig m_cfg;

        zmq::socket_t m_clisock;
        std::map<std::string, PortInfo> m_portinfos;

        yamz::ClientReply m_last_reply{};

        std::unique_ptr<yamz::Server> m_server;
        Mode m_mode{Mode::uninitialized};


        // Break up some steps but these are all called in constructor.
        void make_ports();
        void do_binds();


    };

}
#endif
