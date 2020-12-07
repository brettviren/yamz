#include "server_logic.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>

#include <set>

using namespace yamz::server;

// Return true if ra matches ma.

bool yamz::server::match(const MatchAddress& ma, const RemoteAddress& ra)
{
    if (! (ma.nodeid == "*" or ma.nodeid == ra.nodeid)) {
        return false;
    }
    if (! (ma.clientid == "*" or ma.clientid == ra.clientid)) {
        return false;
    }
    if (! (ma.portid == "*" or ma.portid == ra.portid)) {
        return false;
    }
    return yamz::match(ma.patts, ra.parms);
}

yamz::IdentityPatterns
yamz::server::append(const yamz::IdentityPatterns& idp1,
                     const yamz::IdentityPatterns& idp2)
{
    yamz::IdentityPatterns ret(idp1.begin(), idp1.end());
    ret.insert(ret.end(), idp2.begin(), idp2.end());
    return ret;    
}

// parse an abstract address into a MatchAddress
void
yamz::server::parse_abstract(yamz::server::MatchAddress& ma,
                             const std::string& addr)
{
    auto uri = yamz::uri::parse(addr);

    // path = "//client/port?foo=bar"
    auto s1 = uri.path.find_first_not_of("/");
    ++s1;
    auto s2 = uri.path.find_first_not_of("/", s1);
    ma.clientid = uri.path.substr(s1, s2-s1);
    ++s2;
    auto s3 = uri.path.find_first_not_of("/?", s2);
    ma.portid = uri.path.substr(s2, s3-s2);

    for (const auto& [key,val] : uri.queries) {
        ma.patts[key].insert(val);
    }
}

ServerLogic::ServerLogic(const yamz::ServerConfig& cfg)
    : config(cfg)
{
    // YAMZ-PARAMS : a=b,c=d
    for (const auto& [key,val] : config.idparms) {
        headers.parms[""].push_back(key + "=" + val);
    }
}

void ServerLogic::update(remid_t rid, std::string clid, std::string portid, std::string address) {
    auto it = requests.find(rid);
    if (it == requests.end()) {
        throw server_error("internal logic error, no client " + clid);
    }
    auto& cc = it->second;
    for (auto& port : cc.ports) {
        if (port.portid != portid) {
            continue;
        }
        port.conns.push_back(address);
        return;
    }
    std::stringstream ss;
    ss << "internal logic error, no client " << clid
       << " with port " << portid;
    throw server_error(ss.str());
}

void ServerLogic::process()
{
    for (const auto& ma : tomatch) {
        for (const auto& ra : remotes) {
            if (! match(ma, ra)) {
                update(ma.remid, ma.clid, ma.clportid, ra.address);
            }
        }
    }
}


void ServerLogic::accept_peer(const RemoteAddress& ma)
{
    remotes.push_back(ma);
}

void ServerLogic::accept_client(remid_t remid, yamz::ClientConfig cc)
{

    // YAMZ-PARAMS-<clientid> : a=b,c=d
    for (const auto& cp : cc.idparms) {
        std::stringstream ss;
        ss << cp.key << "=" << cp.val;
        headers.parms[cc.clientid].push_back(ss.str());
    }

    // take one pass through each port to collect bind related
    // info to send out to zyre and abstract conn related info for
    // later matching.

    for (auto& port : cc.ports) {

        // <clientid>/<portid>
        std::string cpid = cc.clientid + "/" + port.portid;

        /* First, the bind parts */

        // YAMZ-PARAMS-comp2/port : a=b,c=d
        for (const auto& pp : port.idparms) {
            std::stringstream ss;
            ss << pp.key << "=" << pp.val;
            headers.parms[cpid].push_back(ss.str());
        }

        // YAMZ-PORTS : comp1/portA=PUB,comp1/portB=SUB,comp2/port=PUSH
        headers.ports.push_back(cpid + "=" + yamz::str(port.ztype) );

        // YAMZ-BINDS-comp1/portA : tcp://127.0.0.1:8065, inproc://portA
        for (auto& addr : port.binds) {
            headers.binds[cpid].push_back(addr);
        }

        /* Then, the connect parts */

        auto cli_patts = append(config.idpatts, cc.idpatts);

        auto port_patts = append(cli_patts, port.idpatts);
        std::vector<std::string> tokeep;
        for (auto& addr : port.conns) {
            if (yamz::is_abstract(addr)) {
                MatchAddress ma{cc.clientid, port.portid, remid};
                // Add any port-level and above patts:
                for (const auto& pp : port_patts) {
                    ma.patts[pp.key].insert(pp.val);
                }
                // and finally any info in the abstract address itself
                parse_abstract(ma, addr);
                tomatch.push_back(ma);
            }
            else { 
                tokeep.push_back(addr);
            }
        }
        port.conns = tokeep;
    }
    requests[remid] = cc;
}

