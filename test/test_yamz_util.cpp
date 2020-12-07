#include "yamz/util.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

int main()
{
    std::string original = "comp1/portA=PUB, comp1/portB=SUB, comp2/port=PUSH";
    auto res = yamz::parse_map(original);

    // now reconstruct
    std::stringstream ss;
    std::string comma = "";
    for (const auto& [candp,ztype] : res) {
        auto path = yamz::parse_list(candp, "/");

        ss << comma << path[0] << "/" << path[1] << "=" << ztype;
        comma = ", ";
    }
    auto roundtrip = ss.str();

    std::cerr << "original:  " << original << std::endl;
    std::cerr << "roundtrip: " << roundtrip << std::endl;
    assert(roundtrip == original);
    original.erase(std::remove_if(original.begin(), original.end(), isspace),
                 original.end());
    std::cerr << "spaceless: " << original << std::endl;
    

    return 0;
}
