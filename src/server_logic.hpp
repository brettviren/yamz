/**
   This contains the yamz server "logic".  It holds the book-keeping
   and is not dependent on any message passing nor network discovery
   mechanism.  In principle it can be used with servers that make
   different choices for these functions.

 */

#ifndef YAMZ_SERVER_LOGIC_HPP
#define YAMZ_SERVER_LOGIC_HPP

#include "server_data.hpp"

#include <yamz/util.hpp>
#include <yamz/zyre.hpp>
#include <yamz/Structs.hpp>

#include <string>
#include <map>

namespace yamz::server {


    // Return true if ra matches ma.
    bool match(const MatchAddress& ma, const RemoteAddress& ra);

    // Return a new I.P. vector which has idp2 appended to idp1.
    yamz::IdentityPatterns append(const yamz::IdentityPatterns& idp1,
                                  const yamz::IdentityPatterns& idp2);

    // parse an abstract address into a MatchAddress
    void parse_abstract(MatchAddress& ma, const std::string& addr);

    // This struct holds server logic, separate from any socketry
    struct Logic {

        zmq::context_t& ctx;
        yamz::ServerConfig cfg;

        zmq::socket_t link, sock;
        yamz::Zyre zyre;

        // collect what we know about ourself and our clients
        yamz::YamzPeer us;

        // associate a client to any outstanding replies
        std::map<remid_t, yamz::ClientReplies> tosend;

        // associate a client to its requests.
        std::map<remid_t, MatchAddresses> tomatch;

        // // Collect yamz Zyre header info from each request.  socketry part
        // // will turn this into zyre message
        // YamZyreHeaders headers;


        Logic(zmq::context_t& ctx, const yamz::ServerConfig& cfg,
              const std::string& linkname);


        // guards
        bool have_clients();

        // Actions
        void go_online();
        void go_offline();
        void store_request();
        void add_peer();
        void del_peer();
        void notify_clients();

        // Internal actions
        void do_matching();
        void send_ready();

        // Main working data are in the form of client configuration
        // objects.
        using requests_t = std::map<remid_t, yamz::ClientConfig>;
        requests_t requests;

        // Capture peer info
        std::vector<RemoteAddress> remotes;


        // Append address to port in client's request.
        void update(remid_t rid, std::string clid, std::string portid,
                    std::string address);

        // Accept a message from a Zyre peer
        void accept_peer(const RemoteAddress& ma);

        // Accept a message from a client
        void accept_client(remid_t remid, const yamz::ClientConfig& cc);

        // Return true if the expected request criteria is not yet met.
        bool outstanding() const { return false; }

        // Force the expected request criteria as met.
        void set_expected() { };

        // Return true if we think we've discovered all we can.
        bool discovered();
    };

}

#endif
