/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace yamz to be serialized via nlohmann::json.
 */
#ifndef YAMZ_NLJS_HPP
#define YAMZ_NLJS_HPP


#include "Structs.hpp"


#include <nlohmann/json.hpp>

namespace yamz {

    using data_t = nlohmann::json;

    NLOHMANN_JSON_SERIALIZE_ENUM( SockType, {
            { yamz::SockType::PAIR, "PAIR" },
            { yamz::SockType::PUB, "PUB" },
            { yamz::SockType::SUB, "SUB" },
            { yamz::SockType::REQ, "REQ" },
            { yamz::SockType::REP, "REP" },
            { yamz::SockType::DEALER, "DEALER" },
            { yamz::SockType::ROUTER, "ROUTER" },
            { yamz::SockType::PULL, "PULL" },
            { yamz::SockType::PUSH, "PUSH" },
            { yamz::SockType::XPUB, "XPUB" },
            { yamz::SockType::XSUB, "XSUB" },
            { yamz::SockType::STREAM, "STREAM" },
            { yamz::SockType::SERVER, "SERVER" },
            { yamz::SockType::CLIENT, "CLIENT" },
            { yamz::SockType::RADIO, "RADIO" },
            { yamz::SockType::DISH, "DISH" },
            { yamz::SockType::GATHER, "GATHER" },
            { yamz::SockType::SCATTER, "SCATTER" },
            { yamz::SockType::DGRAM, "DGRAM" },
            { yamz::SockType::PEER, "PEER" },
        })

    
    inline void to_json(data_t& j, const AbstractAddress& obj) {
        j["node"] = obj.node;
        j["comp"] = obj.comp;
        j["port"] = obj.port;
    }
    
    inline void from_json(const data_t& j, AbstractAddress& obj) {
        if (j.contains("node"))
            j.at("node").get_to(obj.node);    
        if (j.contains("comp"))
            j.at("comp").get_to(obj.comp);    
        if (j.contains("port"))
            j.at("port").get_to(obj.port);    
    }
    
    inline void to_json(data_t& j, const AbstractPort& obj) {
        j["port"] = obj.port;
        j["addrs"] = obj.addrs;
    }
    
    inline void from_json(const data_t& j, AbstractPort& obj) {
        if (j.contains("port"))
            j.at("port").get_to(obj.port);    
        if (j.contains("addrs"))
            j.at("addrs").get_to(obj.addrs);    
    }
    
    inline void to_json(data_t& j, const ConcretePort& obj) {
        j["port"] = obj.port;
        j["type"] = obj.type;
        j["addrs"] = obj.addrs;
    }
    
    inline void from_json(const data_t& j, ConcretePort& obj) {
        if (j.contains("port"))
            j.at("port").get_to(obj.port);    
        if (j.contains("type"))
            j.at("type").get_to(obj.type);    
        if (j.contains("addrs"))
            j.at("addrs").get_to(obj.addrs);    
    }
    
    inline void to_json(data_t& j, const Reply& obj) {
        j["comp"] = obj.comp;
        j["conns"] = obj.conns;
    }
    
    inline void from_json(const data_t& j, Reply& obj) {
        if (j.contains("comp"))
            j.at("comp").get_to(obj.comp);    
        if (j.contains("conns"))
            j.at("conns").get_to(obj.conns);    
    }
    
    inline void to_json(data_t& j, const Request& obj) {
        j["comp"] = obj.comp;
        j["binds"] = obj.binds;
        j["conns"] = obj.conns;
    }
    
    inline void from_json(const data_t& j, Request& obj) {
        if (j.contains("comp"))
            j.at("comp").get_to(obj.comp);    
        if (j.contains("binds"))
            j.at("binds").get_to(obj.binds);    
        if (j.contains("conns"))
            j.at("conns").get_to(obj.conns);    
    }
    
    // fixme: add support for MessagePack serializers (at least)

} // namespace yamz

#endif // YAMZ_NLJS_HPP