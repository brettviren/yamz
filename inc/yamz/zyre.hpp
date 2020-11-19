// A minimal cppzmq wrapper around Zyre inspired by
// https://github.com/eclazi/zyrecpp
//
// Fixme: put zyre messages into schema

#ifndef YAMZ_ZYRE_HPP
#define YAMZ_ZYRE_HPP

#include "zyre.h"
#include <exception>

namespace yamz {

    struct zyre_error : public std::runtime_error {
        zyre_error(const std::string& what) : std::runtime_error(what) {}
    };

    class ZyreEvent {
        zyre_event_t* evt;
      public:
        ZyreEvent(zyre_event_t* own) : evt(own) { }
        ~ZyreEvent( ) {
            zyre_event_destory(&evt);
        }

        ZyreEvent(const ZyreEvent&) = delete;
        ZyreEvent operator=(const ZyreEvent&) = delete;

        ZyreEvent(ZyreEvent&& rhs) {
            evt = rhs.evt;
            rhs.evt = nullptr;
        }

        ZyreEvent& operator=(ZyreEvent&& rhs) {
            if (&rhs != this) {
                evt = rhs.evt;
                rhs.evt = nullptr;
            }
            return *this;
        }

        std::string type() {
            const char *s = zyre_event_type(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_name()
        {
            const char *s = zyre_event_peer_name(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_addr() {
            const char *s = zyre_event_peer_addr(m_self);
            if (s) { return s; }
            return "";
        }

        std::string group() const
        {
            const char *s = zyre_event_group(m_self);
            if (s) { return s; }
            return "";
        }

        std::string header(const std::string& key, const std::string& def="") {
            const char *s = zyre_event_header(m_self, key.c_str());
            if (s) { return s; }
            return def;
        }
    };

    class Zyre {
        zyre_t* peer{nullptr};
      public:

        Zyre(std::string nodename = "") {
            const char* nick = nodename.c_str();
            if (nodename.empty()) {
                nick = nullptr;
            }
            peer = zyre_new(nick);
            if (!peer) {
                throw zyre_error("failed to create zyre");
            }
        }

        ~Zyre() {
            zyre_destroy(&peer);
        }

        void set_header(std::string key, std::string val) {
            zyre_set_header(peer, key.c_str(), "%s", val.c_str());
        }

        zmq::socket_ref socket() {
            // downcast from zyre->zsock->void* and upcast to zmq::socket_ref.
            void* low = zsock_resolve(zyre_socket(peer));
            return zmq::socket_ref(zmq::from_handle, low);
        }

        void start() {
            int rc = zyre_start(zyre);
            if (rc) {
                throw zyre_error("failed to start zyre");
            }
        }

        ZyreEvent event() {
            return ZyreEvent(zyre_event_new(peer));
        }        

    };
}

#endif
