#include <yamz/client.hpp>
#include <yamz/uri.hpp>
#include <yamz/Nljs.hpp>

yamz::Client::Client(zmq::context_t& ctx, const yamz::ClientConfig& newcfg)
    : ctx(ctx), cfg(newcfg)
{
}
yamz::Client::~Client()
{
}

void yamz::Client::configure(const ClientConfig& newcfg)
{
    cfg = newcfg;
}

zmq::socket_ref yamz::Client::get(std::string portid)
{
    auto it = socks.find(portid);
    if ( it == socks.end() ) {
        return zmq::socket_ref{};
    }
    return it->second;
}

void yamz::Client::make_socks()
{
    if (! socks.empty()) { return; }
    for (auto& port : cfg.ports) {
        if (socks.find(port.portid) != socks.end()) {
            throw yamz::client_error("nonunique port name: " + port.portid);
        }

        auto ztype = static_cast<zmq::socket_type>(port.ztype);
        socks[port.portid] = zmq::socket_t(ctx, ztype);
    }
}

void yamz::Client::do_binds()
{
    if (bound) { return; }

    for (auto& port : cfg.ports) {
        auto& sock = socks[port.portid];
        for (size_t ind=0; ind<port.binds.size(); ++ind) {
            auto addr = port.binds[ind];

            auto uri = yamz::uri::parse(addr);
            // addr may be ephemeral.
            // tcp or ipc relies on zmq to resolve.  inproc needs help
            if (uri.scheme == "inproc" and uri.domain == "*") {
                addr = "inproc://" + cfg.clientid + "/" + port.portid;
            }
            else {
                addr = yamz::str(uri, false); // sans query
            }
            sock.bind(addr);    //  may throw

            // resolve ephemeral to concrete
            addr = sock.get(zmq::sockopt::last_endpoint);
            
            // add back any params
            addr += yamz::str(uri.queries);
        }
    }
    bound = true;
}

void yamz::Client::do_conns()
{
    if (conned) { return; }

    for (auto& port : cfg.ports) {
        auto& sock = socks[port.portid];
        for (auto addr : port.conns) {
            if (addr.find("*") != addr.npos) {
                throw yamz::client_error("ephemeral connect address: " + addr);
            }
            auto uri = yamz::uri::parse(addr);
            if (uri.scheme == "yamz") {
                throw yamz::client_error("abstract connect address: " + addr);
            }
            addr = yamz::str(uri, false); // sans any query
            sock.connect(addr);
        }
    }
    conned = true;
}

void yamz::Client::connect_server()
{
    if (clisock) { return; }

    clisock = zmq::socket_t(ctx, zmq::socket_type::client);
    for (const auto& addr : cfg.servers) {
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

    requested = true;
}

void yamz::Client::try_recv(std::chrono::milliseconds timeout)
{
    if (recved) { return; }

    clisock.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeout.count()));
    zmq::message_t msg;
    auto res = clisock.recv(msg, zmq::recv_flags::none);
    if (!res) {
        recved = false;
        return;
    }
    configure(yamz::data_t::parse(msg.to_string()).get<yamz::ClientConfig>());
    recved = true;
}

bool yamz::Client::discover(std::chrono::milliseconds timeout)
{
    connect_server();
    make_socks();
    do_binds();
    make_request();
    try_recv(timeout);
    if (!recved) {
        return false;           // caller must try again later
    }
    do_conns();
    return true;
}

void yamz::Client::publish()
{
    connect_server();
    make_socks();
    do_binds();
    make_request();
    do_conns();
}

void yamz::Client::direct()
{
    make_socks();
    do_binds();
    do_conns();
}
