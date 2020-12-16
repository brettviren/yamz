#include <yamz/client.hpp>
#include <yamz/util.hpp>
#include <yamz/uri.hpp>
#include <yamz/Nljs.hpp>

#include <iostream>             // debug

yamz::Client::Client(zmq::context_t& ctx, const yamz::ClientConfig& newcfg)
    : ctx(ctx), cfg(newcfg)
{
    connect_server();
    make_ports();
    do_binds();
    make_request();
}
yamz::Client::~Client()
{
    std::cerr << "client: destructing" << std::endl;
}

#define chirp(cfg, strm) { std::stringstream ss; ss << "yamz::client: " << cfg.clientid << ": " << strm << "\n"; std::cerr << ss.str(); }


yamz::Client::PortInfo& yamz::Client::get(std::string portid)
{
    auto it = ports.find(portid);
    if ( it == ports.end() ) {
        throw yamz::client_error("client has no such port: " + portid);
    }
    return it->second;
}

void yamz::Client::make_ports()
{
    // one time call
    if (! ports.empty()) { return; }
    
    for (auto& portcfg : cfg.ports) {
        auto portid = portcfg.portid;
        if (ports.find(portid) != ports.end()) {
            throw yamz::client_error("nonunique port name: " + portid);
        }

        auto ztype = static_cast<zmq::socket_type>(portcfg.ztype);
        ports[portid] = PortInfo{portid, zmq::socket_t(ctx, ztype)};
    }
}

void yamz::Client::do_binds()
{
    if (bound) { return; }

    for (auto& portcfg : cfg.ports) {
        auto portid = portcfg.portid;
        auto& pi = get(portid);
        for (size_t ind=0; ind<portcfg.binds.size(); ++ind) {

            // fixme: add this bit of address fiddling to util

            auto addr = portcfg.binds[ind];
            auto uri = yamz::uri::parse(addr);

            // check for an resolve inproc://*
            if (uri.scheme == "inproc") {
                if (uri.domain == "*") {
                    uri.domain = cfg.clientid + "-" + portid;
                }
            }
            if (uri.scheme == "tcp") {
                if (uri.domain == "*") {
                    uri.domain = yamz::myip();
                }
            }
            addr = yamz::str(uri, false); // no params
            pi.sock.bind(addr);    //  may throw
            auto conc = pi.sock.get(zmq::sockopt::last_endpoint);

            chirp(cfg, "bind " << portcfg.binds[ind]
                  << " -> " << addr
                  << " -> " << conc);
            portcfg.binds[ind] = conc;
            pi.binds.push_back(conc);
        }
    }
    bound = true;
}

void yamz::Client::connect_server()
{
    if (clisock) { return; }

    clisock = zmq::socket_t(ctx, zmq::socket_type::client);
    for (const auto& addr : cfg.servers) {
        chirp(cfg, "connect to server at " << addr);
        clisock.connect(addr);
    }
}

void yamz::Client::make_request()
{
    if (requested) { return; }

    yamz::data_t obj = cfg;
    zmq::message_t msg(obj.dump());
    auto res = clisock.send(msg, zmq::send_flags::none);
    if (!res) {
        throw yamz::client_error("failed to send request");
    }
    chirp(cfg, "made request to server: " << obj.dump());
    requested = true;
}

yamz::ClientAction yamz::Client::discover(std::chrono::milliseconds timeout)
{
    // int timeo = static_cast<int>(timeout.count());
    // clisock.set(zmq::sockopt::rcvtimeo, timeo);
    zmq::message_t msg;

    zmq::poller_t<> poller;
    poller.add(clisock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(1);

    //chirp("polling");
    const int nevents = poller.wait_all(events, timeout);
    if (zsys_interrupted) {
        chirp(cfg, "czmq interupted");
        return yamz::ClientAction::terminate;
    }
    if (!nevents) {
        return yamz::ClientAction::timeout;
    }
    auto res = clisock.recv(msg, zmq::recv_flags::none);
    if (!res) {
        chirp(cfg, "error in receiving");
        throw client_error("error receiving reply");
    }
    auto jobj = yamz::data_t::parse(msg.to_string());
    auto rep = jobj.get<yamz::ClientReply>();

    chirp(cfg, "reply: " << jobj.dump());

    auto it = ports.find(rep.portid);
    if (it == ports.end()) {
        throw yamz::client_error("yamz server gave unknown port: " + rep.portid);
    }
    PortInfo& pi = it->second;
    if (rep.action == yamz::ClientAction::connect) {
        pi.sock.connect(rep.address);
        pi.conns.push_back(rep.address);
        chirp(cfg, "connected to: " << rep.address);
        return rep.action;
    }
    if (rep.action == yamz::ClientAction::disconnect) {
        pi.sock.disconnect(rep.address);
        // fixme: remove from conns
        chirp(cfg, "disconnected from: " << rep.address);
        return rep.action;
    }
    if (rep.action == yamz::ClientAction::terminate) {
        chirp(cfg, "server says terminate: ");
        return rep.action;
    }
    throw yamz::client_error("yamz server gave unknown reply");
}

