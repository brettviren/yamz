#include <yamz/client.hpp>
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
}


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
                    uri.domain = get_hostname();
                }
            }
            addr = yamz::str(uri, false); // no params
            pi.sock.bind(addr);    //  may throw
            auto conc = sock.get(zmq::sockopt::last_endpoint);

            std::cerr << "yamz client: bind " << portcfg.binds[ind]
                      << " -> " << addr
                      << " -> " << conc << std::endl;
            portcfg.binds[ind] = conc;
        }
    }
    bound = true;
}

void yamz::Client::connect_server()
{
    if (clisock) { return; }

    clisock = zmq::socket_t(ctx, zmq::socket_type::client);
    for (const auto& addr : cfg.servers) {
        std::cerr << "yamz client: connect to server at " << addr << std::endl;
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
    std::cerr << "yamz client: request " << obj.dump() << std::endl;
    requested = true;
}

bool yamz::Client::discover(std::chrono::milliseconds timeout)
{
    int timeo = static_cast<int>(timeout.count());
    clisock.set(zmq::sockopt::rcvtimeo, timeo);
    zmq::message_t msg;

    auto res = clisock.recv(msg, zmq::recv_flags::none);
    if (!res) {
        return false;           // timeout
    }
    auto jobj = yamz::data_t::parse(msg.to_string());
    auto rep = jobj.get<yamz::ClientReply>();

    std::cerr << "yamz client: reply: " << jobj.dump() << std::endl;

    auto it = ports.find(rep.portid);
    if (it == ports.end()) {
        throw yamz::client_error("yamz server gave unknown port: " + rep.portid);
    }
    auto& sock = it->second.sock;
    if (rep.action == yamz::ClientAction::connect) {
        sock.connect(rep.address);
        return true;
    }
    if (rep.action == yamz::ClientAction::disconnect) {
        sock.disconnect(rep.address);
        return true;
    }
    throw yamz::client_error("yamz server gave unknown reply");
}

