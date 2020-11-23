#include "yamz/util.hpp"

#include <sstream>
#include <algorithm>

yamz::cportmap_t yamz::parse_header(std::string header)
{
    header.erase(std::remove_if(header.begin(), header.end(), isspace),
                 header.end());

    yamz::cportmap_t ret;       // comp to vector<ConcretePort>
    std::stringstream ss(header);
    while (true) {
        // comp1/portA:PUB = tcp://127.0.0.1:8065, inproc://portA;
        std::string cpsaddrs;
        std::getline(ss, cpsaddrs, ';');
        if (cpsaddrs.empty()) {
            break;
        }
        std::stringstream sss(cpsaddrs);

        std::string comp;
        std::getline(sss, comp, '/');

        std::string port;
        std::getline(sss, port, ':');

        std::string sock;
        std::getline(sss, sock, '=');

        std::vector<std::string> addrs;
        while (true) {
            std::string addr;
            std::getline(sss, addr, ',');
            if (addr.empty()) {
                break;
            }
            addrs.push_back(addr);
        }
        auto zsock = yamz::parse_SockType(sock);
        ret[comp].emplace_back(yamz::ConcretePort{port, zsock, addrs});
    }            
    return ret;
}
