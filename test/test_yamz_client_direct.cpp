/* Test using direct connections but with and without server */

#include "yamz/client.hpp"
#include <cassert>

#include <iostream>

void do_it(yamz::Client& c1, yamz::Client& c2, yamz::Client::Mode mode)
{
    assert (c1.make_port("out", ZMQ_PUSH));
    assert (c2.make_port("in",  ZMQ_PULL));

    c1.bind("out", "tcp://*:*");
    yamz::Client::PortInfo& pi = c1.get("out");
    c2.connect("in", pi.binds[0]);

    std::cerr << "initialize client 1\n" ;
    assert ( mode == c1.initialize() );
    std::cerr << "initialized client 1\n" ;
    if (&c1 != &c2) {
        std::cerr << "initialize client 2\n" ;
        assert ( mode == c2.initialize() );
    }

    std::cerr << "poll clients\n" ;
    assert ( c1.poll() == yamz::ClientAction::timeout );
    assert ( c2.poll() == yamz::ClientAction::timeout );

    auto& sout = c1.get("out").sock;
    auto& sin = c2.get("in").sock;

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
}


void test_one(yamz::Client::Mode mode)
{
    zmq::context_t ctx;
    yamz::Client c1(ctx);
    if (mode == yamz::Client::Mode::selfserve) {
        c1.set_clientid("c1");
    }
    do_it(c1, c1, mode);
}
void test_two(yamz::Client::Mode mode)
{
    zmq::context_t ctx;
    yamz::Client c1(ctx), c2(ctx);
    if (mode == yamz::Client::Mode::selfserve) {
        c1.set_clientid("c1");
        c2.set_clientid("c2");
    }
    do_it(c1, c2, mode);
}

int main()
{
    // test_one(yamz::Client::Mode::direct);
    // test_two(yamz::Client::Mode::direct);
    // test_one(yamz::Client::Mode::selfserve);
    test_two(yamz::Client::Mode::selfserve);
}
