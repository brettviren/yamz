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
    struct ServerLogic {

        // Collect yamz Zyre header info from each request.  socketry part
        // will turn this into zyre message
        YamZyreHeaders headers;

        // The server configuration object;
        yamz::ServerConfig config;

        ServerLogic(const yamz::ServerConfig& cfg);


        // Main working data are in the form of client configuration
        // objects.
        using requests_t = std::map<remid_t, yamz::ClientConfig>;
        requests_t requests;

        // Intermediate data holding client requested abstract address matching
        std::vector<MatchAddress> tomatch;

        // Capture peer info
        std::vector<RemoteAddress> remotes;


        // Append address to port in client's request.
        void update(remid_t rid, std::string clid, std::string portid,
                    std::string address);

        // Compare all MatchAddreses to all RemoteAddresses.  Any
        // matches result in concrete addresses added to the corresponding
        // client port conns list.
        void process();

        // Accept a message from a Zyre peer
        void accept_peer(const RemoteAddress& ma);

        // Accept a message from a client
        void accept_client(remid_t remid, yamz::ClientConfig cc);
    };

}

#endif
