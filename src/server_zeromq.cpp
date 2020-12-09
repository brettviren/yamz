#include "server_zeromq.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>
#include <yamz/Nljs.hpp>

using namespace yamz::server;

std::vector<RemoteAddress> yamz::server::from_zyre(yamz::ZyreEvent& zev)
{
    std::vector<RemoteAddress> ret;
    if (zev.type() != "ENTER") {
        return ret;
    }

    auto nodeid = zev.peer_name();

    auto ports = yamz::parse_map(zev.header("YAMZ-PORTS"));
    if (ports.empty()) {
        return ret;
    }
    psetmap_t node_params;
    for (const auto& pstr : yamz::parse_list("YAMZ-PARMS")) {
        auto kv = yamz::parse_list(pstr, "=");
        node_params[kv[0]].insert(kv[1]);
    }

    // YAMZ-PORTS : comp1/portA=PUB, comp1/portB=SUB, comp2/port = PUSH
    // YAMZ-BINDS-comp1/portA : tcp://127.0.0.1:8065, inproc://portA
    // YAMZ-PARAMS-comp2/port : a=b,c=d

    for (const auto& [candp, ztype] : ports) {
        auto cps = yamz::parse_list(candp, "/");

        psetmap_t client_params = node_params;
        auto parm_strs = yamz::parse_list(zev.header("YAMZ-PARMS-" + candp));
        for (const auto& pstr : parm_strs) {
            auto kv = yamz::parse_list(pstr, "=");
            client_params[kv[0]].insert(kv[1]);
        }
        auto binds = yamz::parse_list(zev.header("YAMZ-BINDS-" + candp));
        for (const auto& addr : binds) {
            auto uri = yamz::uri::parse(addr);
            auto just_addr = yamz::str(uri, false); // strip query

            psetmap_t addr_params = client_params;
            for (const auto& q : uri.queries) {
                addr_params[q.first].insert(q.second);
            }
            ret.emplace_back(RemoteAddress{nodeid, cps[0], cps[1],
                                           addr_params, just_addr, ztype});
        }
    }
    return ret;
}

void yamz::server::tell_zyre(yamz::Zyre& zyre, const YamZyreHeaders& headers)
{
    zyre.set_header("YAMZ-PORTS", yamz::str(headers.ports));
    for (const auto& [name, parts] : headers.binds) {
        zyre.set_header("YAMZ-BINDS-" + name, yamz::str(parts));
    }
    for (const auto& [name, parts] : headers.parms) {
        std::string key = "YAMZ-PARAMS";
        if (! name.empty()) {
            key += "-";
        }
        key += name;
        zyre.set_header(key, yamz::str(parts));
    }
}

remid_t yamz::server::recv(zmq::socket_t& sock, yamz::ClientConfig& cc)
{
    zmq::message_t msg;
    auto res = sock.recv(msg);
    if (!res) {
        throw yamz::server_error("failed to receive from client");
    }

    // note, this is for SERVER, if ROUTER, we take from message.
    remid_t rid = msg.routing_id();

    auto sreq = msg.to_string();
    auto jobj = yamz::data_t::parse(sreq);
    cc = jobj.get<yamz::ClientConfig>();
    return rid;
}
void send(zmq::socket_t& sock, remid_t rid, const yamz::ClientConfig& cc)
{
    yamz::data_t jobj = cc;
    zmq::message_t msg(jobj.dump());

    // note, this is for SERVER, if ROUTER, we put to message.
    msg.set_routing_id(rid);

    auto res = sock.send(msg, zmq::send_flags::none);
    if (!res) {
        std::stringstream ss;
        ss << "failed to send to client " << rid;
        throw yamz::server_error(ss.str());
    }
}
 
