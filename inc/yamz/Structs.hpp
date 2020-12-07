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

    // @brief Abstract Address
    using AbstractAddress = std::string;

    // @brief 
    using AbstractAddresses = std::vector<yamz::AbstractAddress>;

    // @brief An identifier or name
    using Ident = std::string;

    // @brief Concrete address
    using ConcreteAddress = std::string;

    // @brief 
    using ConcreteAddresses = std::vector<yamz::ConcreteAddress>;

    // @brief A key name
    using Key = std::string;

    // @brief An identity parameter value
    using ParameterValue = std::string;

    // @brief An identity parameter
    struct IdentityParameter {

        // @brief The key name of the parameter
        Key key = "";

        // @brief The value of the parameter
        ParameterValue val = "";
    };

    // @brief 
    using IdentityParameters = std::vector<yamz::IdentityParameter>;

    // @brief An identity pattern value
    using PatternValue = std::string;

    // @brief An identity pattern
    struct IdentityPattern {

        // @brief The key name of the pattern
        Key key = "";

        // @brief The pattern string
        PatternValue val = "";
    };

    // @brief 
    using IdentityPatterns = std::vector<yamz::IdentityPattern>;

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

    // @brief Ephemeral Address
    using EphemeralAddress = std::string;

    // @brief 
    using EphemeralAddresses = std::vector<yamz::EphemeralAddress>;

    // @brief Describe one client port
    struct ClientPort {

        // @brief Identify port to network and application
        Ident portid = "";

        // @brief The ZeroMQ socket type
        SockType ztype = yamz::SockType::PAIR;

        // @brief Any port-level identity parameters
        IdentityParameters idparms = {};

        // @brief Any port-level identity patterns
        IdentityPatterns idpatts = {};

        // @brief Ephemeral bind addresses
        EphemeralAddresses binds = {};

        // @brief Abstract connect addresses
        AbstractAddresses conns = {};
    };

    // @brief 
    using ClientPorts = std::vector<yamz::ClientPort>;

    // @brief A yamz client configuration object
    struct ClientConfig {

        // @brief The name by which the client is known in the node
        Ident clientid = "";

        // @brief The server addresses to which the client shall connect
        ConcreteAddresses servers = {"inproc://yamz"};

        // @brief Any client-level identity parameters
        IdentityParameters idparms = {};

        // @brief Any client-level identity patterns
        IdentityPatterns idpatts = {};

        // @brief Describe ports the client shall provide to application
        ClientPorts ports = {};
    };

    // @brief 
    using Idents = std::vector<yamz::Ident>;

    // @brief A hierarchy path
    using Path = std::string;

    // @brief A IP port number
    using PortNum = int32_t;

    // @brief A yamz server configuration object
    struct ServerConfig {

        // @brief The name by which this yamz node is known on the network
        Ident nodeid = "";

        // @brief The IP port number on which Zyre operates
        PortNum portnum = 5670;

        // @brief The addresses to which the server shall bind
        ConcreteAddresses addresses = {"inproc://yamz"};

        // @brief Any node-level identity parameters
        IdentityParameters idparms = {};

        // @brief Any node-level identity patterns
        IdentityPatterns idpatts = {};

        // @brief A set of peer nodes to expect to discover
        Idents expected = {};
    };

} // namespace yamz

#endif // YAMZ_STRUCTS_HPP