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

    
    inline void to_json(data_t& j, const IdentityParameter& obj) {
        j["key"] = obj.key;
        j["val"] = obj.val;
    }
    
    inline void from_json(const data_t& j, IdentityParameter& obj) {
        if (j.contains("key"))
            j.at("key").get_to(obj.key);    
        if (j.contains("val"))
            j.at("val").get_to(obj.val);    
    }
    
    inline void to_json(data_t& j, const IdentityPattern& obj) {
        j["key"] = obj.key;
        j["val"] = obj.val;
    }
    
    inline void from_json(const data_t& j, IdentityPattern& obj) {
        if (j.contains("key"))
            j.at("key").get_to(obj.key);    
        if (j.contains("val"))
            j.at("val").get_to(obj.val);    
    }
    
    inline void to_json(data_t& j, const ClientPort& obj) {
        j["portid"] = obj.portid;
        j["ztype"] = obj.ztype;
        j["idparms"] = obj.idparms;
        j["idpatts"] = obj.idpatts;
        j["binds"] = obj.binds;
        j["conns"] = obj.conns;
    }
    
    inline void from_json(const data_t& j, ClientPort& obj) {
        if (j.contains("portid"))
            j.at("portid").get_to(obj.portid);    
        if (j.contains("ztype"))
            j.at("ztype").get_to(obj.ztype);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("idpatts"))
            j.at("idpatts").get_to(obj.idpatts);    
        if (j.contains("binds"))
            j.at("binds").get_to(obj.binds);    
        if (j.contains("conns"))
            j.at("conns").get_to(obj.conns);    
    }
    
    inline void to_json(data_t& j, const ClientConfig& obj) {
        j["clientid"] = obj.clientid;
        j["servers"] = obj.servers;
        j["idparms"] = obj.idparms;
        j["idpatts"] = obj.idpatts;
        j["ports"] = obj.ports;
    }
    
    inline void from_json(const data_t& j, ClientConfig& obj) {
        if (j.contains("clientid"))
            j.at("clientid").get_to(obj.clientid);    
        if (j.contains("servers"))
            j.at("servers").get_to(obj.servers);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("idpatts"))
            j.at("idpatts").get_to(obj.idpatts);    
        if (j.contains("ports"))
            j.at("ports").get_to(obj.ports);    
    }
    
    inline void to_json(data_t& j, const ServerConfig& obj) {
        j["nodeid"] = obj.nodeid;
        j["portnum"] = obj.portnum;
        j["addresses"] = obj.addresses;
        j["idparms"] = obj.idparms;
        j["idpatts"] = obj.idpatts;
        j["expected"] = obj.expected;
    }
    
    inline void from_json(const data_t& j, ServerConfig& obj) {
        if (j.contains("nodeid"))
            j.at("nodeid").get_to(obj.nodeid);    
        if (j.contains("portnum"))
            j.at("portnum").get_to(obj.portnum);    
        if (j.contains("addresses"))
            j.at("addresses").get_to(obj.addresses);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("idpatts"))
            j.at("idpatts").get_to(obj.idpatts);    
        if (j.contains("expected"))
            j.at("expected").get_to(obj.expected);    
    }
    
    // fixme: add support for MessagePack serializers (at least)

} // namespace yamz

#endif // YAMZ_NLJS_HPP