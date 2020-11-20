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

    // @brief An association of a port and its concrete addresses
    struct ConcretePort {

        // @brief Identify a port of the client
        Ident port = "";

        // @brief The ZeroMQ socket type
        SockType type = yamz::SockType::PAIR;

        // @brief Concrete addresses associated with port
        ConcreteAddresses addrs = {};
    };

    // @brief 
    using ConcretePorts = std::vector<yamz::ConcretePort>;

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