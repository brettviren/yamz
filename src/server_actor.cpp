#include "server_actor.hpp"
#include "server_logic.hpp"

#include <yamz/server.hpp>

using namespace yamz::server;

void yamz::server::server_actor(yamz::Server::Params params)
{
    ServerLogic guts(params.cfg);
    
    
}

