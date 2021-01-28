#include "yamz/client.hpp"
#include "yamz/server.hpp"
#include "yamz/util.hpp"
#include "yamz/uri.hpp"
#include "yamz/Nljs.hpp"

#include <iostream>             // debug

yamz::Client::Client(zmq::context_t& ctx)
    : m_ctx(ctx)
{
}
yamz::Client::Client(zmq::context_t& ctx, const std::string& clientid)
    : m_ctx(ctx), m_cfg{clientid}
{
}

yamz::Client::Client(zmq::context_t& ctx, const yamz::ClientConfig& newcfg)
    : m_ctx(ctx), m_cfg(newcfg)
{
    make_ports();
    do_binds();
}

yamz::Client::~Client()
{
    std::cerr << "client: destructing" << std::endl;
}

#define chirp(cfg, strm) { std::stringstream ss; ss << "yamz::client: " << cfg.clientid << ": " << strm << "\n"; std::cerr << ss.str(); }


yamz::Client::PortInfo& yamz::Client::get(std::string portid)
{
    auto it = m_portinfos.find(portid);
    if ( it == m_portinfos.end() ) {
        throw yamz::client_error("client has no such port: " + portid);
    }
    return it->second;
}

bool yamz::Client::make_port(std::string portid, int stype)
{
    auto it = m_portinfos.find(portid);
    if ( it != m_portinfos.end() ) {
        return false;
    }
        
    const size_t ind = m_cfg.ports.size();

    // keep cfg/ports in sync
    m_cfg.ports.emplace_back(yamz::ClientPort{
            portid,static_cast<yamz::SockType>(stype)});

    auto ztype = static_cast<zmq::socket_type>(stype);
    m_portinfos[portid] = PortInfo{portid,
        zmq::socket_t(m_ctx, ztype), ind};

    return true;
}

void yamz::Client::make_ports()
{
    // one time call
    if (! m_portinfos.empty()) { return; }
    
    const size_t nports = m_cfg.ports.size();
    for (size_t ind=0; ind<nports; ++ind) {
        auto& portcfg = m_cfg.ports.at(ind);
        auto portid = portcfg.portid;
        if (m_portinfos.find(portid) != m_portinfos.end()) {
            throw yamz::client_error("nonunique port name: " + portid);
        }

        auto ztype = static_cast<zmq::socket_type>(portcfg.ztype);
        m_portinfos[portid] = PortInfo{portid,
            zmq::socket_t(m_ctx, ztype), ind};
    }
}

// Register for address for connect
void yamz::Client::connect(std::string portid, std::string addr)
{
    auto& pi = get(portid);
    auto& portcfg = m_cfg.ports.at(pi.ccpind);
    portcfg.conns.push_back(addr);
    // fixme: want to add some kind of short circuit for concrete
    // connect addresses for when the client is configured with no
    // server.  Ie, to operate in "direct" mode.
}


// Bind address to port and update our records
void yamz::Client::bind(std::string portid, std::string addr)
{
    const std::string orig = addr;
    auto uri = yamz::uri::parse(addr);
    auto& pi = get(portid);
    auto& portcfg = m_cfg.ports.at(pi.ccpind);

    // check for an resolve inproc://*
    if (uri.scheme == "inproc") {
        if (uri.domain == "*") {
            uri.domain = m_cfg.clientid + "-" + portid;
        }
    }
    if (uri.scheme == "tcp") {
        if (uri.domain == "*") {
            uri.domain = yamz::myip();
        }
    }
    addr = yamz::str(uri, false); // no params
    pi.sock.bind(addr);           // may throw
    auto conc = pi.sock.get(zmq::sockopt::last_endpoint);
    chirp(m_cfg, "bind " << orig << " -> " << addr << " -> " << conc);
    portcfg.binds.push_back(conc);
    pi.binds.push_back(conc);
}

// Walk through existing configuration and apply any binds
void yamz::Client::do_binds()
{
    for (auto& portcfg : m_cfg.ports) {
        auto portid = portcfg.portid;

        auto to_bind = portcfg.binds;

        // temporary clear this vector, bind() fills it up again
        // with resolved bind.
        portcfg.binds.clear();
        for (auto& addr : to_bind) {
            bind(portid, addr);
        }
    }
}

yamz::Client::Mode yamz::Client::initialize()
{
    if (m_mode != Mode::uninitialized) {
        return m_mode;          // been here
    }

    if (m_cfg.servers.empty()) {
        if (m_cfg.clientid.empty() or m_cfg.clientid == "") {
            // direct mode
            m_mode = Mode::direct;
            for (auto& portcfg : m_cfg.ports) {
                for (auto& addr : portcfg.conns) {
                    // fixme: warn if yamz:// found
                    auto& pi = m_portinfos[portcfg.portid];
                    pi.sock.connect(addr);
                    pi.conns.push_back(addr);
                    chirp(m_cfg, portcfg.portid << " connected to: " << addr);
                }
            }
            return m_mode;
        }

        // self-serve mode, start our server
        m_mode = Mode::selfserve;
        m_cfg.servers.push_back("inproc://" + m_cfg.clientid + "-selfserv");
        yamz::ServerConfig scfg{m_cfg.clientid, m_cfg.servers};
        scfg.expected.push_back(m_cfg.clientid); // just us
        m_server = std::make_unique<yamz::Server>(m_ctx, scfg);
        m_server->start();
        // // we are sole client, go online already
        // m_server->online();
    }
    else {
        // external / app-level server mode
        m_mode = Mode::extserver;
    }
    
    // both selfserve and extserver talk to the server identically

    m_clisock = zmq::socket_t(m_ctx, zmq::socket_type::client);
    for (const auto& addr : m_cfg.servers) {
        chirp(m_cfg, "connect to server at " << addr);
        m_clisock.connect(addr);
    }

    yamz::data_t obj = m_cfg;
    zmq::message_t msg(obj.dump());
    auto res = m_clisock.send(msg, zmq::send_flags::none);
    if (!res) {
        throw yamz::client_error("failed to send request");
    }
    chirp(m_cfg, "made request to server: " << obj.dump());
    return m_mode;
}

yamz::ClientAction yamz::Client::poll(std::chrono::milliseconds timeout)
{
    if (m_mode == Mode::uninitialized or m_mode == Mode::direct) {
        return yamz::ClientAction::timeout;
    }
    // other two modes work through the client socket.

    // int timeo = static_cast<int>(timeout.count());
    // clisock.set(zmq::sockopt::rcvtimeo, timeo);
    zmq::message_t msg;

    zmq::poller_t<> poller;
    poller.add(m_clisock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(1);

    //chirp("polling");
    const int nevents = poller.wait_all(events, timeout);
    if (!nevents) {
        return yamz::ClientAction::timeout;
    }
    auto res = m_clisock.recv(msg, zmq::recv_flags::none);
    if (!res) {
        chirp(m_cfg, "error in receiving");
        throw client_error("error receiving reply");
    }
    auto jobj = yamz::data_t::parse(msg.to_string());
    m_last_reply = jobj.get<yamz::ClientReply>();

    chirp(m_cfg, "reply: " << jobj.dump());

    auto it = m_portinfos.find(m_last_reply.portid);
    if (it == m_portinfos.end()) {
        throw yamz::client_error("yamz server gave unknown port: "
                                 + m_last_reply.portid);
    }
    PortInfo& pi = it->second;
    if (m_last_reply.action == yamz::ClientAction::connect) {
        pi.sock.connect(m_last_reply.address);
        pi.conns.push_back(m_last_reply.address);
        chirp(m_cfg, "connected to: " << m_last_reply.address);
        return m_last_reply.action;
    }
    if (m_last_reply.action == yamz::ClientAction::disconnect) {
        pi.sock.disconnect(m_last_reply.address);
        // fixme: remove from conns
        chirp(m_cfg, "disconnected from: " << m_last_reply.address);
        return m_last_reply.action;
    }
    if (m_last_reply.action == yamz::ClientAction::terminate) {
        chirp(m_cfg, "server says terminate: ");
        return m_last_reply.action;
    }
    throw yamz::client_error("yamz server gave unknown reply");
}

