/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace yamz.
 */
#ifndef YAMZ_STRUCTS_HPP
#define YAMZ_STRUCTS_HPP

#include <cstdint>

#include <vector>
#include <string>

namespace yamz {

    // @brief An unique identifier
    using Ident = std::string;

    // @brief Abstractly identify a socket address
    struct AbstractAddress {

        // @brief Identify a node
        Ident node = "";

        // @brief Identify a node's component
        Ident comp = "";

        // @brief Identify a component's port
        Ident port = "";
    };

    // @brief 
    using AbstractAddresses = std::vector<yamz::AbstractAddress>;

    // @brief All addresses to resolve for one client port
    struct AbstractPort {

        // @brief Name of the clients port that wants to connect
        Ident port = "";

        // @brief Abstract addresses for server to match
        AbstractAddresses addrs = {};
    };

    // @brief 
    using AbstractPorts = std::vector<yamz::AbstractPort>;

    // @brief Concrete address
    using ConcreteAddress = std::string;

    // @brief 
    using ConcreteAddresses = std::vector<yamz::ConcreteAddress>;

    // @brief Enumerate ZeroMQ socket names in canoncial order
    enum class SockType: unsigned {
        PAIR,
        PUB,
        SUB,
        REQ,
        REP,
        DEALER,
        ROUTER,
        PULL,
        PUSH,
        XPUB,
        XSUB,
        STREAM,
        SERVER,
        CLIENT,
        RADIO,
        DISH,
        GATHER,
        SCATTER,
        DGRAM,
        PEER,
    };
    // return a string representation of a SockType.
    inline
    const char* str(SockType val) {
        if (val == SockType::PAIR) { return "PAIR" ;}
        if (val == SockType::PUB) { return "PUB" ;}
        if (val == SockType::SUB) { return "SUB" ;}
        if (val == SockType::REQ) { return "REQ" ;}
        if (val == SockType::REP) { return "REP" ;}
        if (val == SockType::DEALER) { return "DEALER" ;}
        if (val == SockType::ROUTER) { return "ROUTER" ;}
        if (val == SockType::PULL) { return "PULL" ;}
        if (val == SockType::PUSH) { return "PUSH" ;}
        if (val == SockType::XPUB) { return "XPUB" ;}
        if (val == SockType::XSUB) { return "XSUB" ;}
        if (val == SockType::STREAM) { return "STREAM" ;}
        if (val == SockType::SERVER) { return "SERVER" ;}
        if (val == SockType::CLIENT) { return "CLIENT" ;}
        if (val == SockType::RADIO) { return "RADIO" ;}
        if (val == SockType::DISH) { return "DISH" ;}
        if (val == SockType::GATHER) { return "GATHER" ;}
        if (val == SockType::SCATTER) { return "SCATTER" ;}
        if (val == SockType::DGRAM) { return "DGRAM" ;}
        if (val == SockType::PEER) { return "PEER" ;}
        return "";                  // should not reach
    }
    inline
    SockType parse_SockType(std::string val, SockType def = SockType::PAIR) {
        if (val == "PAIR") { return SockType::PAIR; }
        if (val == "PUB") { return SockType::PUB; }
        if (val == "SUB") { return SockType::SUB; }
        if (val == "REQ") { return SockType::REQ; }
        if (val == "REP") { return SockType::REP; }
        if (val == "DEALER") { return SockType::DEALER; }
        if (val == "ROUTER") { return SockType::ROUTER; }
        if (val == "PULL") { return SockType::PULL; }
        if (val == "PUSH") { return SockType::PUSH; }
        if (val == "XPUB") { return SockType::XPUB; }
        if (val == "XSUB") { return SockType::XSUB; }
        if (val == "STREAM") { return SockType::STREAM; }
        if (val == "SERVER") { return SockType::SERVER; }
        if (val == "CLIENT") { return SockType::CLIENT; }
        if (val == "RADIO") { return SockType::RADIO; }
        if (val == "DISH") { return SockType::DISH; }
        if (val == "GATHER") { return SockType::GATHER; }
        if (val == "SCATTER") { return SockType::SCATTER; }
        if (val == "DGRAM") { return SockType::DGRAM; }
        if (val == "PEER") { return SockType::PEER; }
        return def;
    }

    // @brief An association of a port and its concrete addresses
    struct ConcretePort {

        // @brief Identify a port of the client
        Ident portid = "";

        // @brief The ZeroMQ socket type
        SockType ztype = yamz::SockType::PAIR;

        // @brief Concrete addresses associated with port
        ConcreteAddresses concs = {};
    };

    // @brief 
    using ConcretePorts = std::vector<yamz::ConcretePort>;

    // @brief YAMZ zyre header value
    using Header = std::string;

    // @brief A hierarchy path
    using Path = std::string;

    // @brief Request reply from server to client
    struct Reply {

        // @brief The requesting client component identity
        Ident comp = "";

        // @brief A set of concrete port addresses client may connect
        ConcretePorts conns = {};
    };

    // @brief The structure of a client request made to a server
    struct Request {

        // @brief Uniquely identify the client
        Ident comp = "";

        // @brief Concrete bind ports made by client
        ConcretePorts binds = {};

        // @brief Abstract connect ports to be resolved by server
        AbstractPorts conns = {};
    };

} // namespace yamz

#endif // YAMZ_STRUCTS_HPP