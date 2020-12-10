#include "server_logic.hpp"
#include "server_zeromq.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>

#include <set>

#include <iostream>             // debug

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

yamz::server::Logic::Logic(zmq::context_t& ctx, const yamz::ServerConfig& cfg,
                           const std::string& linkname)

    : ctx(ctx), cfg(cfg), zyre(cfg.nodeid)
{
    // Prime "us" with what app tells us about server
    us.nodeid = cfg.nodeid;
    us.idparms = cfg.idparms; 

    // Link back to synchronous API
    link = zmq::socket_t(ctx, zmq::socket_type::pair);
    link.connect(linkname);

    // Server socket
    sock = zmq::socket_t(ctx, zmq::socket_type::server);
    for (auto& addr : cfg.addresses) {
        std::cerr << "server actor: bind: " << addr << std::endl;
        sock.bind(addr);
    }    

    // Link to zyre, but we don't start yet
    zyre.set_portnum(cfg.portnum);

    // notify API it can continue
    std::cerr << "server actor: sending ready" << std::endl;
    link.send(zmq::message_t{}, zmq::send_flags::none);
}



void yamz::server::Logic::update(remid_t rid, std::string clid, std::string portid, std::string address) {
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


void yamz::server::Logic::accept_peer(const RemoteAddress& ma)
{
    remotes.push_back(ma);
}

void yamz::server::Logic::accept_client(remid_t remid,
                                        const yamz::ClientConfig& cc)
{

    yamz::YamzClient yc{cc.clientid, cc.idparms};

    // take one pass through each port to collect bind-related info
    // for zyre and conn-related info for later matching.

    auto cli_patts = append(cfg.idpatts, cc.idpatts);

    for (auto& port : cc.ports) {

        // remember contribution to Zyre
        yamz::YamzPort yp{port.portid, port.ztype, port.idparms, port.binds};

        // remember connect parts with patts rolled down to the
        // address level.
        auto port_patts = append(cli_patts, port.idpatts);

        for (auto& addr : port.conns) {
            if (yamz::is_abstract(addr)) {
                MatchAddress ma{cc.clientid, port.portid, remid};
                // Add any port-level and above patts:
                for (const auto& pp : port_patts) {
                    ma.patts[pp.key].insert(pp.val);
                }
                // and finally any info in the abstract address itself
                parse_abstract(ma, addr);
                tomatch[remid].push_back(ma);
            }
            else { 
                // Client sent us a concrete connection address which
                // we will echo back next chance.
                yamz::ClientReply rep{port.portid,
                    yamz::ClientAction::connect, addr};
                tosend[remid].push_back(rep);
            }
        }
    }
    requests[remid] = cc;
}

void yamz::server::Logic::do_matching()
{
    // if offline then return
    // check tomatch against zyre, update tosend, purge tomatch

    // for (const auto& [remid, mas] : tomatch) {
    //     for (const auto& ra : remotes) {
    //         if (! match(ma, ra)) {
    //             update(ma.remid, ma.clid, ma.clportid, ra.address);
    //         }
    //     }
    // }

}
void yamz::server::Logic::send_ready()
{
    // drain tosend
}

void yamz::server::Logic::go_online() 
{
}
void yamz::server::Logic::go_offline() 
{
}
void yamz::server::Logic::store_request() 
{
    // We have a client request waiting.  Receive it and store.
    yamz::ClientConfig cc;
    auto remid = yamz::server::recv_type(sock, cc);
    accept_client(remid, cc);
    do_matching();
    send_ready();
}
void yamz::server::Logic::add_peer() 
{
}
void yamz::server::Logic::del_peer() 
{
}
void yamz::server::Logic::notify_clients() 
{
}
bool yamz::server::Logic::have_clients()
{
    // if offline, return false
    // apply "expected" criteria
    return true;
}
