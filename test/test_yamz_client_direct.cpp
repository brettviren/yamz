/* Test direct connection (concrete addresses) with no server */

#include "yamz/client.hpp"
#include <cassert>

void test_two()
{
    zmq::context_t ctx;
    yamz::Client c1(ctx), c2(ctx);

    assert (c1.make_port("out", ZMQ_PUSH));
    assert (c2.make_port("in",  ZMQ_PULL));

    c1.bind("out", "tcp://*:*");
    yamz::Client::PortInfo& pi = c1.get("out");
    c2.connect("in", pi.binds[0]);

    assert ( c1.initialize() == yamz::Client::Mode::direct );
    assert ( c2.initialize() == yamz::Client::Mode::direct );

    assert ( c1.poll() == yamz::ClientAction::timeout );
    assert ( c2.poll() == yamz::ClientAction::timeout );

    auto& sout = c1.get("out").sock;
    auto& sin = c2.get("in").sock;

    const std::string str{"hello world"};
    zmq::message_t msg(str);
    auto sres = sout.send(msg, zmq::send_flags::none);
    assert ( sres );

    msg.rebuild();
    auto rres = sin.recv(msg, zmq::recv_flags::none);
    assert ( rres );
    assert ( str == msg.to_string() );
}

int main()
{
    test_two();
}
