#ifndef YAMZ_SERVER_HPP
#define YAMZ_SERVER_HPP

#include <yamz/zeromq.hpp>

#include <vector>
#include <string>
#include <thread>
#include <exception>

namespace yamz {


    struct server_error : public std::runtime_error {
        server_error(const std::string& what) : std::runtime_error(what) {}
    };

    class Server {

      public:

        // Create the yamz server, bind to given addresses and run
        // until all expected are found.  If no expected are given,
        // rely on explicit online command.
        Server(zmq::context_t& ctx,
               std::vector<std::string> tobind = { "inproc://yamz" });

        ~Server();
        
        // If it is not convenient to set the naem at construction, it
        // can be set with this method but before start() is called.
        void set_name(std::string name);

        // The Zyre port may be changed from its default before
        // start() in order to have distinct yamz/zyre networks
        // coexist.
        void set_port(int port=5670);

        // Start the server listening to requests from clients.  If
        // any expected are given the server will automatically switch
        // over to discovery when all listed clients have made a
        // request.
        void start(std::vector<std::string> expected = {});

        // Call to explicitly transition from accepting requests to
        // bringing the Zyre peer online and begin satisfying the
        // requests.  Once called, no client requests will be
        // processed.  If an "expected" list was given and not all
        // expected were found, false is returned.  Note, if
        // "expected" are satisfied the server may already be in
        // discovery mode in which case, this merely reports if all
        // expected were seen.  If an empty "expected" list was given
        // this returns true.
        bool discover();

        // Bundle this info to simplify sharing with actor function.
        struct Params {
            // user provided context
            zmq::context_t& ctx;
            // name we announce in discovery
            std::string nodename{""};
            // internal bind addresses for server.  Clients must know
            // and use at least one.  Only need to change or add to
            // this is we have multiple yamz servers in one app or if
            // inproc:// is not sufficient (eg, multiple
            // zmq::context_t in play).
            std::vector<std::string> tobind{ "inproc://yamz" };
            // see start()
            std::vector<std::string> expected{};
            // see set_port()
            int port{5670};
            // internal, generated name for link to actor function
            std::string linkname{""}; 
        };
        Params params;

      private:

        std::thread athread;
        zmq::socket_t alink;

    };

} // namespace yamz

#endif // YAMZ_SERVER_HPP
