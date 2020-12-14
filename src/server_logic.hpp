/**
   This contains the yamz server "logic".  It holds the book-keeping
   and is not dependent on any message passing nor network discovery
   mechanism.  In principle it can be used with servers that make
   different choices for these functions.

 */

#ifndef YAMZ_SERVER_LOGIC_HPP
#define YAMZ_SERVER_LOGIC_HPP

#include "server_zeromq.hpp"

#include <yamz/util.hpp>
#include <yamz/zyre.hpp>
#include <yamz/Structs.hpp>

#include <string>
#include <map>

namespace yamz::server {


    class Logic {

      public:

        // This flows down the idparms to each individual concrete
        // bind address of a peer.
        struct RemoteAddress {
            std::string nodeid, clientid, portid;
            psetmap_t parms;
            std::string address;
            yamz::SockType ztype;
        };
        using server_clock = std::chrono::steady_clock;
        // map zyre uuid to information about a peer
        struct PeerInfo {
            std::string zuuid;   // zyre id
            std::string znick;   // zyre node/nick name
            std::string zaddr;   // zyre address
            using time_point = std::chrono::time_point<server_clock>;
            time_point seen;    // when we saw the peer ENTER or EXIT
            std::vector<RemoteAddress> ras;
        };

        struct MatchAddress {
            // My client's name and its port's name for one abstract address
            std::string clid, clportid; 
            // remember what client this came from
            remid_t remid; 
            // The non-query portion of the abstract address
            std::string nodeid, clientid, portid;
            // The query portion
            psetmap_t patts;
        };
        using MatchAddresses = std::vector<MatchAddress>;

        struct Clients {
            // map client request ID to info about the client
            struct Info {
                remid_t remid; // remote request id
                std::string nick; // client name
                // Any outstanding replies that need sending 
                yamz::ClientReplies tosend;
                // Any outstanding matches
                MatchAddresses tomatch;
            };
            std::vector<Info> infos;
            std::map<remid_t, size_t> remid_index;
            std::map<std::string, size_t> nick_index;

            void add(Info&& info) {
                size_t ind = infos.size();
                remid_index[info.remid] = ind;
                nick_index[info.nick] = ind;
                infos.emplace_back(info);
            }

            Info* by_remid(remid_t remid) {
                auto it = remid_index.find(remid);
                if (it == remid_index.end()) { return nullptr; }
                return &infos.at(it->second);
            }
            Info* by_nick(const std::string& nick) {
                auto it = nick_index.find(nick);
                if (it == nick_index.end()) { return nullptr; }
                return &infos.at(it->second);
            }
        };

        // For all peer infos that match, produce reply with client action
        void match_address(Logic::MatchAddress& ma,
                           yamz::ClientAction ca);

        Logic(zmq::context_t& ctx, const yamz::ServerConfig& cfg,
              const std::string& linkname);

        // guards
        bool have_clients();

        // Actions
        void go_online();
        void go_offline();
        void store_request();
        void add_peer(yamz::ZyreEvent& zev);
        void del_peer(yamz::ZyreEvent& zev);
        void notify_clients();

        // low level access
        yamz::ZyreEvent recv_zyre();
        std::string recv_link();

        // Internal "actions" called by actions
        void do_matching(yamz::ClientAction ca);

        // Accept a message from a client
        void accept_client(remid_t remid, const yamz::ClientConfig& cc);

        // data

        zmq::context_t& ctx;
        yamz::ServerConfig cfg;

        zmq::socket_t link, sock;
        yamz::Zyre zyre;
        bool zyre_online{false};

        // collect what we know about ourself and our clients
        yamz::YamzPeer us;

        // Peers seen on ENTER, removed on EXIT.
        std::map<std::string, PeerInfo> them;
        // maybe: any reason to remember the goners?
        Clients clients;

        

    };

}

#endif
