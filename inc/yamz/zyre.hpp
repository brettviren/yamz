// A minimal cppzmq wrapper around Zyre inspired by
// https://github.com/eclazi/zyrecpp
//
// Fixme: put zyre messages into schema

#ifndef YAMZ_ZYRE_HPP
#define YAMZ_ZYRE_HPP

#include "yamz/zeromq.hpp"
#include "zyre.h"
#include <exception>
#include <map>


namespace yamz {

    struct zyre_error : public std::runtime_error {
        zyre_error(const std::string& what) : std::runtime_error(what) {}
    };

    class ZyreEvent {
        zyre_event_t* evt;
      public:
        ZyreEvent(zyre_event_t* own) : evt(own) { }
        ~ZyreEvent( ) {
            zyre_event_destroy(&evt);
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

        std::string peer_uuid()
        {
            const char *s = zyre_event_peer_uuid(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_addr() {
            const char *s = zyre_event_peer_addr(evt);
            if (s) { return s; }
            return "";
        }

        std::string group() const
        {
            const char *s = zyre_event_group(evt);
            if (s) { return s; }
            return "";
        }

        std::string header(const std::string& key, const std::string& def="") {
            const char *s = zyre_event_header(evt, key.c_str());
            if (s) { return s; }
            return def;
        }
    };

    // A memoizing wrapper around the underlying Zyre.  Going offline
    // will remember the node name, port and headers.
    class Zyre {
        zyre_t* _peer{nullptr};
        std::string _nick{""};

        using headers_t = std::map<std::string, std::string>;
        headers_t _headers;
        int _portnum{5670};
      public:

        Zyre(std::string nodename = "") : _nick(nodename) {
        }

        Zyre(const Zyre&) = delete;
        Zyre operator=(const Zyre&) = delete;

        ~Zyre() {
            offline();
        }

        // Configure zyre.  If online, this requires a reboot().
        void set_header(std::string key, std::string val) {
            _headers[key] = val;
        }
        void clear_headers() {
            _headers.clear();
        }
        void set_portnum(int portnum = 5670) {
            _portnum = portnum;
        }

        // Get underlying zyre socket if online
        zmq::socket_ref socket() {
            if (!_peer) { return nullptr; }
            // downcast from zyre->zsock->void* and upcast to zmq::socket_ref.
            void* low = zsock_resolve(zyre_socket(_peer));
            return zmq::socket_ref(zmq::from_handle, low);
        }

        void reboot() {
            offline();
            online();
        }

        void offline() {
            if (!_peer) { return; }
            zyre_destroy(&_peer);
        }

        void online() {
            if (_peer) { return; }

            _peer = zyre_new(_nick.empty() ? nullptr : _nick.c_str());
            if (!_peer) {
                throw zyre_error("failed to create zyre");
            }

            for (const auto& [key, val] : _headers) {
                zyre_set_header(_peer, key.c_str(), "%s", val.c_str());
            }
            zyre_set_port(_peer, _portnum);
            int rc = zyre_start(_peer);
            if (rc) {
                throw zyre_error("failed to start zyre");
            }
        }

        ZyreEvent event() {
            if (_peer) {
                return ZyreEvent(zyre_event_new(_peer));
            }
            throw zyre_error("can not get blood from a stone");
        }        

    };
}

#endif
