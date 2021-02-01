/* Test yamz termination in various ways
   - server dies, clients follow
   - clients die, server follows
 */
#include "yamz/server.hpp"
#include "yamz/client.hpp"
#include <iostream>
void test_cdt(int scenario) {
    zmq::context_t ctx;

    yamz::ServerConfig scfg{"nodename"};
    scfg.expected = {"client1", "client2"};

    // Typically one need not create on heap but want to control order
    // of death in different ways.
    yamz::Server* server = new yamz::Server(ctx, scfg);
    server->start();

    yamz::Client* client1 = new yamz::Client(ctx, "client1");
    yamz::Client* client2 = new yamz::Client(ctx, "client2");

    client1->add_server(scfg.addresses[0]);
    client1->make_port("out", ZMQ_PUSH);
    client1->bind("out", "tcp://*:*");
    auto m1 = client1->initialize();
    assert(m1 == yamz::Client::Mode::extserver);
    auto& outpi = client1->get("out");
    assert(outpi.name == "out");
    assert(outpi.ccpind == 0);
    assert(outpi.conns.empty());
    assert(outpi.binds.size() == 1);
    auto outaddr = outpi.binds[0];

    client2->add_server(scfg.addresses[0]);
    client2->make_port("in", ZMQ_PUSH);
    client2->connect("in", "yamz://*/client1/out");
    auto m2 = client2->initialize();
    assert(m2 == yamz::Client::Mode::extserver);
    auto& inpi = client2->get("in");

    // server should be online now and clients ready for polling.

    std::chrono::milliseconds delay(10);
    yamz::ClientAction ca;

    ca = client1->poll(delay);
    assert(ca == yamz::ClientAction::timeout); // no reply expected

    ca = client2->poll(delay);
    assert(ca == yamz::ClientAction::connect);
    auto lr = client2->last_reply();
    assert(lr.action == yamz::ClientAction::connect);
    assert(lr.portid == "client2");
    assert(lr.address == outaddr);
    assert(inpi.binds.empty());
    assert(inpi.conns.size() == 1);
    assert(inpi.conns[0] == outaddr);

    // at this point client1:out is linked to client2:in
    
    auto& sout = outpi.sock;
    auto& sin = inpi.sock;

    const std::string str{"hello world"};
    std::cerr << "sending: \"" << str << "\"\n";

    zmq::message_t msg(str);
    auto sres = sout.send(msg, zmq::send_flags::none);
    assert ( sres );

    std::cerr << "receiving\n";
    msg.rebuild();
    auto rres = sin.recv(msg, zmq::recv_flags::none);
    assert ( rres );
    assert ( str == msg.to_string() );


    // finally we can kill things off.

    if (scenario == 1) {         // murder server
        delete server;
        ca = client1->poll(delay);
        assert(ca == yamz::ClientAction::terminate);
        ca = client2->poll(delay);
        assert(ca == yamz::ClientAction::terminate);
        delete client1;
        delete client2;
    }
    if (scenario == 2) {        // murder client
        delete client1;
        ca = client2->poll(delay);
        assert(ca == yamz::ClientAction::disconnect);
        delete client2;
        delete server;
    }

}
int main()
{
    test_cdt(1);
    return 0;
}
