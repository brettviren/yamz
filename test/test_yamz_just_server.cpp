#include "yamz/server.hpp"

#include <iostream>

int main()
{
    zmq::context_t ctx;

    {
        yamz::Server ys(ctx);
        std::cerr << "server: start" << std::endl;
        ys.start();
        std::cerr << "server: discover" << std::endl;
        ys.discover();
        std::cerr << "server: exit" << std::endl;
    }
    
                
    return 0;
}
