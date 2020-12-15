#include "server_actor.hpp"
#include "server_logic.hpp"
#include <yamz/server.hpp>
#include <yamz/Structs.hpp>

#include <boost/sml.hpp>

#include <iostream>             // debug

#include <deque>
#include <queue>

namespace sml = boost::sml;

// define the server FSM
namespace {

    // All the events and states are just markers

    // events
    struct ServerOnline{};
    struct ServerOffline{};
    struct ServerTerminate{};
    struct ClientRequest{};
    struct PeerEnter{
        yamz::ZyreEvent& zev;
    };
    struct PeerExit{
        yamz::ZyreEvent zev;
    };
    struct SayHi {};

    // states
    auto Cready = sml::state<class Cready>;
    auto Cproc = sml::state<class CProc>;
    auto Dready = sml::state<class Dready>;
    auto Dproc = sml::state<class Dproc>;

    // forward actions to server::Logic
    const auto go_online = [](yamz::server::Logic& guts) {
        std::cerr << "server actor: go online\n";
        guts.go_online();
    };
    const auto go_offline = [](yamz::server::Logic& guts) {
        std::cerr << "server actor: go offline\n";
        guts.go_offline();
    };
    const auto store_request = [](yamz::server::Logic& guts) {
        std::cerr << "server actor: store request\n";
        guts.store_request();
    };
    const auto add_peer = [](yamz::server::Logic& guts, const PeerEnter& pe) {
        std::cerr << "server actor: add peer\n";
        guts.add_peer(pe.zev);
    };
    const auto del_peer = [](yamz::server::Logic& guts, const PeerExit& pe) {
        std::cerr << "server actor: del peer\n";
        guts.del_peer(pe.zev);
    };
    const auto notify_clients = [](yamz::server::Logic& guts) {
        std::cerr << "server actor: notify clients\n";
        guts.notify_clients();
    };
    const auto say_hi = [](yamz::server::Logic& guts) {
        std::cerr << "server actor: hi there\n";
    };

    // forward guards to server::Logic 
    const auto have_clients = [](yamz::server::Logic& guts) {
        return guts.have_clients();
    };

    struct Discovery {
        auto operator()() {
            using namespace boost::sml;
            return make_transition_table(
// clang-format: off
* Dready + event<ClientRequest> / store_request = Dproc
, Dready + event<PeerEnter> / add_peer = Dproc
, Dready + event<PeerExit> / del_peer = Dproc
, Dready + event<SayHi> / say_hi = Dready
, Dproc / notify_clients = Dready
// clang-format: on
                );
        }
    };


    struct Collecting {
        auto operator()() {
            using namespace boost::sml;
            return make_transition_table(
// clang-format: off
* Cready + event<ClientRequest> / store_request = Cproc
, Cproc [ ! have_clients ] = Cready
, Cproc [ have_clients ] / process(ServerOnline{}) = Cready
// clang-format: on
                );
        }
    };

    struct Running {
        auto operator()() {
            using namespace boost::sml;

            return make_transition_table(
// clang-format: off
* state<Collecting> + event<ServerOnline> / go_online = state<Discovery>
, state<Discovery> + event<ServerOffline> / go_offline = state<Collecting>
// clang-format: on
                );
        }
    };
}

using FSM = sml::sm<Running,
                    sml::defer_queue<std::deque>,
                    sml::process_queue<std::queue>>;

static void
handle_link(FSM& fsm, yamz::server::Logic& guts)
{
    auto cmd = guts.recv_link();
    std::cerr << "server actor: link hit: " << cmd << std::endl;
    if (cmd == "ONLINE") {
        fsm.process_event(ServerOnline{});
        return;
    }
    if (cmd == "OFFLINE") {
        fsm.process_event(ServerOffline{});
        return;
    }
    fsm.process_event(ServerTerminate{});
}

static void
handle_sock(FSM& fsm, yamz::server::Logic& guts)
{
    std::cerr << "server actor: sock hit" << std::endl;
    fsm.process_event(ClientRequest{});
}

static void
handle_zyre(FSM& fsm, yamz::server::Logic& guts)
{
    auto zev = guts.recv_zyre();
    std::cerr << "server actor: zyre hit: " << zev.type() << std::endl;

    if (zev.type() == "ENTER") {
        std::cerr << "server actor: zyre ENTER: peer: "
                  << zev.peer_name() << " [" << zev.peer_uuid() << "]"
                  << std::endl;
        std::cerr << zev.header("YAMZ") << std::endl;
        
        fsm.process_event(SayHi{});
        fsm.process_event(PeerEnter{zev});
        std::cerr << "server actor: PeerEnter done\n";
        return;
    }
    if (zev.type() == "EXIT") {
        fsm.process_event(PeerExit{std::move(zev)});
        return;
    }
    // any other event types, we just quietly ignore
}

void yamz::server::actor(ActorArgs aa)
{
    yamz::server::Logic guts(aa.ctx, aa.cfg, aa.linkname);
    FSM fsm{guts};

    // We now sit in a loop polling sockets for messages, converting
    // messages to events and processing events into the FSM.

    zmq::poller_t<> poller;
    poller.add(guts.link, zmq::event_flags::pollin);
    poller.add(guts.sock, zmq::event_flags::pollin);
    auto zsock = guts.zyre.socket();
    poller.add(zsock, zmq::event_flags::pollin);
    std::vector<zmq::poller_event<>> events(3);

    std::cerr << "server actor: entering loop" << std::endl;
    while (true) {              // fixme: make way to break
        std::cerr << "server actor: poll" << std::endl;
        const int nevents = poller.wait_all(events, std::chrono::milliseconds{-1});
        for (int iev = 0; iev < nevents; ++iev) {
            if (events[iev].socket == guts.link) {
                handle_link(fsm, guts);
                continue;
            }
            if (events[iev].socket == guts.sock) {
                handle_sock(fsm, guts);
                continue;
            }
            if (events[iev].socket == zsock) {
                if (fsm.is(sml::state<Collecting>)) {
                    std::cerr << "server actor: IN COLLECING STATE NOT DISCOVERY\n";
                }
                handle_zyre(fsm, guts);
                continue;
            }
        }    
    }
}

