#include "server_logic.hpp"
#include "server_zeromq.hpp"
#include <yamz/uri.hpp>
#include <yamz/server.hpp>

#include <set>

#include <iostream>             // debug

using namespace yamz::server;


template<typename SEQ>
SEQ append(const SEQ& seq1, const SEQ& seq2)
{
    SEQ ret(seq1.begin(), seq1.end());
    ret.insert(ret.end(), seq2.begin(), seq2.end());
    return ret;    
}

#define chirp(cfg, strm) { std::stringstream ss; ss << "yamz::server::logic: " << cfg.nodeid << ": " << strm << "\n"; std::cerr << ss.str(); }

std::string yamz::server::str(const MatchAddress& ma)
{
    std::stringstream ss;
    ss << "<MatchAddress "
       << ma.nodeid << "/"
       << ma.clientid << "/"
       << ma.portid
       << ">";
    return ss.str();
}
// parse an abstract address into a MatchAddress
void yamz::server::parse_abstract(MatchAddress& ma, const std::string& addr)
{
    auto uri = yamz::uri::parse(addr);

    ma.nodeid = uri.domain;

    auto pl = yamz::parse_list(uri.path, "/");
    ma.clientid = pl[1];
    ma.portid = pl[2];

    for (const auto& [key,val] : uri.queries) {
        ma.patts[key].insert(val);
    }
}

yamz::server::Logic::Logic(zmq::context_t& ctx, const yamz::ServerConfig& cfg,
                           const std::string& linkname)

    : ctx(ctx), cfg(cfg), zyre(cfg.nodeid)
{
    // Prime "us" with what app tells us about server
    //us.nodeid = cfg.nodeid;
    us.idparms = cfg.idparms; 

    // Link back to synchronous API
    link = zmq::socket_t(ctx, zmq::socket_type::pair);
    link.connect(linkname);

    // Server socket
    sock = zmq::socket_t(ctx, zmq::socket_type::server);
    for (auto& addr : cfg.addresses) {
        chirp(cfg, "bind: " << addr);
        sock.bind(addr);
    }    

    // Link to zyre, but we don't start yet
    zyre.set_portnum(cfg.portnum);
    // fixme: make configurable
    zyre.set_verbose();

    // notify API it can continue
    std::cerr << "server actor: sending ready" << std::endl;
    link.send(zmq::message_t{}, zmq::send_flags::none);
}


void yamz::server::Logic::accept_client(remid_t remid,
                                        const yamz::ClientConfig& cc)
{
    chirp(cfg, "accept client: " << cc.clientid);

    // For talking back to client
    Clients::Info ci{remid, cc.clientid};

    // For talking to zyre
    yamz::YamzClient yc{cc.clientid, cc.idparms};

    // fill in both for each port:
    auto cli_patts = append(cfg.idpatts, cc.idpatts);
    for (auto& port : cc.ports) {

        // Remember this port's contribution to our YAMZ zyre header
        yc.ports.emplace_back(
            yamz::YamzPort{port.portid, port.ztype, port.idparms, port.binds});

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
                chirp(cfg, "tomatch for " << cc.clientid << " ? " << str(ma));

                ci.tomatch.push_back(ma);
            }
            else { 
                // Client sent us a concrete connection address which
                // we will echo back next chance.
                yamz::ClientReply rep{yamz::ClientAction::connect,
                    port.portid, addr};
                ci.tosend.push_back(rep);
            }
        }
    }

    // Remember this client's contribution to our YAMZ header. 
    us.clients.emplace_back(std::move(yc));
    clients.add(std::move(ci));
}

void Logic::match_address(MatchAddress& ma, yamz::ClientAction ca)
{
    auto* ci = clients.by_remid(ma.remid);
    if (!ci) {
        throw yamz::server_error("internal error uknown client: " + ma.clid);
    }
    for (const auto& it : them) {
        for (const auto& ra : it.second.ras) {
            chirp(cfg, "try match for " << str(ma) << " = " << ra.address);
            if (! (ma.nodeid == "*" or ma.nodeid == ra.nodeid)) {
                break;
            }
            if (! (ma.clientid == "*" or ma.clientid == ra.clientid)) {
                break;
            }
            if (! (ma.portid == "*" or ma.portid == ra.portid)) {
                break;
            }
            if (! yamz::match(ma.patts, ra.parms)) {
                break;
            }
            // a winner
            ci->tosend.emplace_back(ClientReply{ca, ma.clportid, ra.address});
        }
    }
}

void yamz::server::Logic::do_matching(yamz::ClientAction ca)
{
    // iterate clients to drain tomatch by comparing against them and
    // filling tosend
    for (auto& ci : clients.infos) {
        for (auto& ma : ci.tomatch) {
            match_address(ma, ca);
        }
    }

    notify_clients();
}
void yamz::server::Logic::notify_clients() 
{
    // Here we drain any filled "tosend" arrays of the clients by
    // sending the contents as individual replies.  

    for (auto& ci : clients.infos) {
        for (const auto& rep : ci.tosend) {
            yamz::data_t jobj = rep;
            zmq::message_t msg(jobj.dump());
            msg.set_routing_id(ci.remid);
            auto res = sock.send(msg, zmq::send_flags::none);
            if (!res) {
                throw server_error("failed to reply to client " + ci.nick);
            }
        }
        // maybe: is there any reason to remember what we sent?
        ci.tosend.clear();
    }
}

void yamz::server::Logic::go_online() 
{
    if (zyre_online) { return; }
    bool gotem = have_clients();
    yamz::data_t jobj = us;
    zyre.set_header("YAMZ", jobj.dump()); 
    zyre.online();
    zyre_online = true;
    chirp(cfg, "go online with:\n" << jobj.dump());
    std::string rep = gotem ? "OKAY" : "FAIL";
    zmq::message_t msg{rep};
    auto res = link.send(msg, zmq::send_flags::none);
    if (!res) {
        throw server_error("failed to send command online ack to API");
    }

    // maybe: inform clients
}

void yamz::server::Logic::go_offline() 
{
    if (!zyre_online) { return; }
    zyre.offline();
    zyre_online = false;
    // maybe: inform clients?
    std::string rep = "OKAY";
    zmq::message_t msg(rep);
    auto res = link.send(msg, zmq::send_flags::none);
    if (!res) {
        throw server_error("failed to send command offline ack to API");
    }
}

void yamz::server::Logic::do_terminate() 
{
    go_offline();
    chirp(cfg, "termiating");

    yamz::ClientReply rep{yamz::ClientAction::terminate};
    yamz::data_t jobj = rep;
    std::string sdat = jobj.dump();
    
    for (auto& ci : clients.infos) {
        chirp(cfg, "termiating client: " << ci.nick);
        zmq::message_t msg(sdat);
        msg.set_routing_id(ci.remid);
        auto res = sock.send(msg, zmq::send_flags::none);
        if (!res) {
            chirp(cfg, "failed to send terminate to " << ci.nick);
        }
    }
    
}

void yamz::server::Logic::store_request() 
{
    // We have a client request waiting.  Receive it and store.
    yamz::ClientConfig cc;
    auto remid = yamz::server::recv_type(sock, cc);
    accept_client(remid, cc);

    // newly added clients may already have won!
    do_matching(yamz::ClientAction::connect);
}

void yamz::server::Logic::add_peer(const yamz::ZyreEvent& zev) 
{
    
    auto znick = zev.peer_name();
    auto zuuid = zev.peer_uuid();
    auto zaddr = zev.peer_addr();

    chirp(cfg, "add peer: " << znick);

    auto text = zev.header("YAMZ");
    auto jobj = yamz::data_t::parse(text);

    auto yp = jobj.get<yamz::YamzPeer>();

    PeerInfo pi{zuuid, znick, zaddr, server_clock::now()};

    for (const auto& client : yp.clients) {
        auto client_parms = append(yp.idparms, client.idparms);
        for (const auto& port : client.ports) {
            auto port_parms = append(client_parms, port.idparms);
            for (const auto& addr : port.addresses) {
                psetmap_t addr_parms;
                auto uri = yamz::uri::parse(addr);
                for (const auto& [key,val] : uri.queries) {
                    addr_parms[key].insert(val);
                }
                auto just_addr = yamz::str(uri, false);
                RemoteAddress ra{znick, client.clientid, port.portid,
                    addr_parms, just_addr, port.ztype};
                pi.ras.emplace_back(std::move(ra));
                chirp(cfg, "add peer: " << client.clientid << "/"
                      << port.portid << " = " << just_addr);
            }
        }
    }
    them[zuuid] = pi;

    do_matching(yamz::ClientAction::connect);
}
void yamz::server::Logic::del_peer(const yamz::ZyreEvent& zev) 
{
    auto ruuid = zev.peer_uuid();
    auto it = them.find(ruuid);
    if (it == them.end()) {
        // never heard of them, somehow Zyre broke
        // maybe: add warning?
        return;                 
    }
    them.erase(it);
    do_matching(yamz::ClientAction::disconnect);
}
bool yamz::server::Logic::have_clients()
{
    if (cfg.expected.empty()) {
        chirp(cfg, "no clients expected");
        // no expectation, yeah, sure, I have all I want, whatever
        return true;
    }
    for (const auto& nick : cfg.expected) {
        if (! clients.by_nick(nick)) {
            chirp(cfg, "expect client " << nick);
            return false;
        }
    }
    chirp(cfg, "have expected clients: " << cfg.expected.size());
    return true;
}

yamz::ZyreEvent yamz::server::Logic::recv_zyre()
{
    auto zev = zyre.event();
    return zev;
}
std::string yamz::server::Logic::recv_link()
{
    zmq::message_t msg;
    auto res = link.recv(msg, zmq::recv_flags::none);
    if (!res) {
        throw yamz::server_error("recv on link failed");
    }
    return msg.to_string();
}
