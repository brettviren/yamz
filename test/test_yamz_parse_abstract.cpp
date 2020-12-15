#include "server_logic.hpp"

#include <iostream>

using namespace yamz::server;
int main()
{
    MatchAddress ma;
    parse_abstract(ma, "yamz://*/*/*");
    std::cerr << str(ma) << std::endl;
}
