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

    NLOHMANN_JSON_SERIALIZE_ENUM( ApiCommands, {
            { yamz::ApiCommands::terminate, "terminate" },
            { yamz::ApiCommands::online, "online" },
            { yamz::ApiCommands::offline, "offline" },
        })
    NLOHMANN_JSON_SERIALIZE_ENUM( ApiReply, {
            { yamz::ApiReply::fail, "fail" },
            { yamz::ApiReply::okay, "okay" },
        })
    NLOHMANN_JSON_SERIALIZE_ENUM( ClientAction, {
            { yamz::ClientAction::connect, "connect" },
            { yamz::ClientAction::disconnect, "disconnect" },
        })
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

    
    inline void to_json(data_t& j, const ApiCommand& obj) {
        j["command"] = obj.command;
    }
    
    inline void from_json(const data_t& j, ApiCommand& obj) {
        if (j.contains("command"))
            j.at("command").get_to(obj.command);    
    }
    
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
    
    inline void to_json(data_t& j, const ClientReply& obj) {
        j["portid"] = obj.portid;
        j["action"] = obj.action;
        j["address"] = obj.address;
    }
    
    inline void from_json(const data_t& j, ClientReply& obj) {
        if (j.contains("portid"))
            j.at("portid").get_to(obj.portid);    
        if (j.contains("action"))
            j.at("action").get_to(obj.action);    
        if (j.contains("address"))
            j.at("address").get_to(obj.address);    
    }
    
    inline void to_json(data_t& j, const CollProc& obj) {
    }
    
    inline void from_json(const data_t& j, CollProc& obj) {
    }
    
    inline void to_json(data_t& j, const CollReady& obj) {
    }
    
    inline void from_json(const data_t& j, CollReady& obj) {
    }
    
    inline void to_json(data_t& j, const DiscProc& obj) {
    }
    
    inline void from_json(const data_t& j, DiscProc& obj) {
    }
    
    inline void to_json(data_t& j, const DiscReady& obj) {
    }
    
    inline void from_json(const data_t& j, DiscReady& obj) {
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
    
    inline void to_json(data_t& j, const UnixTime& obj) {
        j["s"] = obj.s;
        j["ns"] = obj.ns;
    }
    
    inline void from_json(const data_t& j, UnixTime& obj) {
        if (j.contains("s"))
            j.at("s").get_to(obj.s);    
        if (j.contains("ns"))
            j.at("ns").get_to(obj.ns);    
    }
    
    inline void to_json(data_t& j, const YamzPort& obj) {
        j["portid"] = obj.portid;
        j["ztype"] = obj.ztype;
        j["idparms"] = obj.idparms;
        j["addresses"] = obj.addresses;
    }
    
    inline void from_json(const data_t& j, YamzPort& obj) {
        if (j.contains("portid"))
            j.at("portid").get_to(obj.portid);    
        if (j.contains("ztype"))
            j.at("ztype").get_to(obj.ztype);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("addresses"))
            j.at("addresses").get_to(obj.addresses);    
    }
    
    inline void to_json(data_t& j, const YamzClient& obj) {
        j["clientid"] = obj.clientid;
        j["idparms"] = obj.idparms;
        j["ports"] = obj.ports;
    }
    
    inline void from_json(const data_t& j, YamzClient& obj) {
        if (j.contains("clientid"))
            j.at("clientid").get_to(obj.clientid);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("ports"))
            j.at("ports").get_to(obj.ports);    
    }
    
    inline void to_json(data_t& j, const YamzPeer& obj) {
        j["nodeid"] = obj.nodeid;
        j["idparms"] = obj.idparms;
        j["clients"] = obj.clients;
    }
    
    inline void from_json(const data_t& j, YamzPeer& obj) {
        if (j.contains("nodeid"))
            j.at("nodeid").get_to(obj.nodeid);    
        if (j.contains("idparms"))
            j.at("idparms").get_to(obj.idparms);    
        if (j.contains("clients"))
            j.at("clients").get_to(obj.clients);    
    }
    
    inline void to_json(data_t& j, const evOffline& obj) {
        j["stamp"] = obj.stamp;
    }
    
    inline void from_json(const data_t& j, evOffline& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
    }
    
    inline void to_json(data_t& j, const evOnline& obj) {
        j["stamp"] = obj.stamp;
    }
    
    inline void from_json(const data_t& j, evOnline& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
    }
    
    inline void to_json(data_t& j, const evPeerEnter& obj) {
        j["stamp"] = obj.stamp;
        j["peer"] = obj.peer;
        j["info"] = obj.info;
    }
    
    inline void from_json(const data_t& j, evPeerEnter& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
        if (j.contains("peer"))
            j.at("peer").get_to(obj.peer);    
        if (j.contains("info"))
            j.at("info").get_to(obj.info);    
    }
    
    inline void to_json(data_t& j, const evPeerExit& obj) {
        j["stamp"] = obj.stamp;
        j["peer"] = obj.peer;
    }
    
    inline void from_json(const data_t& j, evPeerExit& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
        if (j.contains("peer"))
            j.at("peer").get_to(obj.peer);    
    }
    
    inline void to_json(data_t& j, const evRequest& obj) {
        j["stamp"] = obj.stamp;
        j["clireq"] = obj.clireq;
    }
    
    inline void from_json(const data_t& j, evRequest& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
        if (j.contains("clireq"))
            j.at("clireq").get_to(obj.clireq);    
    }
    
    inline void to_json(data_t& j, const evTerminate& obj) {
        j["stamp"] = obj.stamp;
    }
    
    inline void from_json(const data_t& j, evTerminate& obj) {
        if (j.contains("stamp"))
            j.at("stamp").get_to(obj.stamp);    
    }
    
    // fixme: add support for MessagePack serializers (at least)

} // namespace yamz

#endif // YAMZ_NLJS_HPP