#ifndef YAMZ_UTIL_HPP
#define YAMZ_UTIL_HPP

#include <string>
#include <vector>
#include <map>

#include "yamz/Structs.hpp"

namespace yamz {


    // Map a component name to its concrete ports
    using cportmap_t = std::map<std::string, yamz::ConcretePorts>;

    // Parse a YAMZ Zyre header value.
    cportmap_t parse_header(std::string header);

}


#endif
