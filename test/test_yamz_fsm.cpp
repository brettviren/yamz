#include "boost/sml.hpp"

#include <cassert>
#include <string>
#include <deque>
#include <queue>
#include <iostream>

int nclients{0};

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
        std::string zev;
    };
    struct PeerExit{
        std::string zev;
    };
    struct SayHi {};

    // // states
    // auto Cready = sml::state<class Cready>;
    // auto Cproc = sml::state<class CProc>;
    // auto Dready = sml::state<class Dready>;
    // auto Dproc = sml::state<class Dproc>;

    const auto go_online = []() {
        std::cerr << "action: go online\n";
    };
    const auto go_online2 = []() {
        std::cerr << "action: go online 2\n";
    };
    const auto go_offline = []() {
        std::cerr << "action: go offline\n";
    };
    const auto store_request = []() {
        ++nclients;
        std::cerr << "action: store request, have " << nclients << std::endl;
    };
    const auto add_peer = [](const PeerEnter& pe) {
        std::cerr << "action: add peer: " << pe.zev << std::endl;
    };
    const auto del_peer = [](const PeerExit& pe) {
        std::cerr << "action: del peer: " << pe.zev << std::endl;
    };
    const auto notify_clients = []() {
        std::cerr << "action: notify clients\n";
    };
    const auto say_hi = []() {
        std::cerr << "action: hi there\n";
    };

    // forward guards to server::Logic 
    const auto have_clients = []() {
        std::cerr << "guard: have clients: " << nclients << std::endl;
        return nclients >= 2;
    };

    struct Discovery {
        auto operator()() const noexcept {
            using namespace boost::sml;
            return make_transition_table(
// clang-format: off
* "Dready"_s + event<ClientRequest> / store_request = "Dproc"_s
, "Dready"_s + event<PeerEnter> / add_peer = "Dproc"_s
, "Dready"_s + event<PeerExit> / del_peer = "Dproc"_s
, "Dready"_s + event<SayHi> / say_hi = "Dready"_s
, "Dproc"_s / notify_clients = "Dready"_s
// clang-format: on
                );
        }
    };


    struct Collecting {
        auto operator()() const noexcept {
            using namespace sml;
            return make_transition_table(
// clang-format: off
* "Cready"_s + event<ClientRequest> / store_request = "Cproc"_s
, "Cproc"_s [ ! have_clients ] = "Cready"_s
, "Cproc"_s [ have_clients ] / process(ServerOnline{}) = "Cready"_s
// clang-format: on
                );
        }
    };

    struct Running {
        auto operator()() const noexcept {
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

int main()
{
    using namespace sml;

    sm<Running, defer_queue<std::deque>, process_queue<std::queue>> fsm;

    assert(fsm.is(state<Collecting>));
    assert(fsm.is<decltype(state<Collecting>)>("Cready"_s));

    fsm.process_event(ClientRequest{});
    assert(fsm.is(state<Collecting>));
    assert(fsm.is<decltype(state<Collecting>)>("Cready"_s));

    fsm.process_event(ClientRequest{}); // should kick into discovery
    assert(fsm.is<decltype(state<Discovery>)>("Dready"_s));
    assert(fsm.is(state<Discovery>));

    fsm.process_event(SayHi{});
    assert(fsm.is<decltype(state<Discovery>)>("Dready"_s));
    assert(fsm.is(state<Discovery>));

    fsm.process_event(PeerEnter{"newpeer"});
    assert(fsm.is<decltype(state<Discovery>)>("Dready"_s));
    assert(fsm.is(state<Discovery>));

    fsm.process_event(PeerExit{"newpeer"});
    assert(fsm.is<decltype(state<Discovery>)>("Dready"_s));
    assert(fsm.is(state<Discovery>));

    return 0;
}
