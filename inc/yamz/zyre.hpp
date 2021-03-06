// A minimal cppzmq wrapper around Zyre inspired by
// https://github.com/eclazi/zyrecpp
//
// Fixme: put zyre messages into schema

#ifndef YAMZ_ZYRE_HPP
#define YAMZ_ZYRE_HPP

#include "yamz/zeromq.hpp"
#include "zyre.h"
#include "czmq.h"               // for zsys_interrupted
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

        ZyreEvent(ZyreEvent&& rhs)
        {
            evt = rhs.evt;
            rhs.evt = nullptr;
        }

        ZyreEvent& operator=(ZyreEvent&& rhs)
        {
            if (&rhs != this) {
                evt = rhs.evt;
                rhs.evt = nullptr;
            }
            return *this;
        }

        std::string type() const
        {
            const char *s = zyre_event_type(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_name() const
        {
            const char *s = zyre_event_peer_name(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_uuid() const
        {
            const char *s = zyre_event_peer_uuid(evt);
            if (s) { return s; }
            return "";
        }

        std::string peer_addr() const
        {
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

        std::string header(const std::string& key, const std::string& def="") const
        {
            const char *s = zyre_event_header(evt, key.c_str());
            if (s) { return s; }
            return def;
        }
    };

    // Simple C++ wrapper to Zyre.
    class Zyre {
        zyre_t* _peer{nullptr};
        std::string _nick{""};

        using headers_t = std::map<std::string, std::string>;
        headers_t _headers;
        int _portnum{5670};
      public:

        Zyre(std::string nodename = "") : _nick(nodename) {
            _peer = zyre_new(_nick.empty() ? nullptr : _nick.c_str());
            if (!_peer) {
                throw zyre_error("failed to create zyre");
            }
        }

        Zyre(const Zyre&) = delete;
        Zyre operator=(const Zyre&) = delete;

        ~Zyre() {
            offline();
            zyre_destroy(&_peer);
        }

        void set_verbose() { zyre_set_verbose(_peer); }

        // Configure zyre.  If online, this requires a reboot().
        void set_header(std::string key, std::string val) {
            zyre_set_header(_peer, key.c_str(), "%s", val.c_str());
        }
        void set_portnum(int portnum = 5670) {
            zyre_set_port(_peer, _portnum);
        }

        std::string uuid() {
            return zyre_uuid(_peer);
        }

        std::string nick() {
            return zyre_name(_peer);
        }

        // Get underlying zyre socket if online
        zmq::socket_ref socket() {
            if (!_peer) { return nullptr; }
            // downcast from zyre->zsock->void* and upcast to zmq::socket_ref.
            void* low = zsock_resolve(zyre_socket(_peer));
            return zmq::socket_ref(zmq::from_handle, low);
        }

        void offline() {
            zyre_stop(_peer);
        }

        void online() {
            int rc = zyre_start(_peer);
            if (rc) {
                throw zyre_error("failed to start zyre");
            }
        }

        ZyreEvent event() {
            if (!_peer) {
                throw zyre_error("can not get blood from a stone");
            }
            zyre_event_t* evt = zyre_event_new(_peer);
            if (!evt) {
                throw zyre_error("failed to get zyre event");
            }
            return ZyreEvent(evt);
        }        

    };
}

#endif
