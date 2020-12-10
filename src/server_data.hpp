#ifndef YAMZ_SERVER_DATA_HPP
#define YAMZ_SERVER_DATA_HPP

#include <yamz/util.hpp>
#include <string>
#include <cstdint>

namespace yamz::server {

    // structures for the server actor


    // This is uint32_t for SERVER and std::string for ROUTER.  If we
    // used ZIO we wouldn't have to expose ourselves to this detail.
    using remid_t = uint32_t;


    /**
       Holds information about one remote bind address from Zyre.
     */
    struct RemoteAddress {
        std::string nodeid, clientid, portid;
        psetmap_t parms;
        std::string address;
        std::string zport;
    };

    /**
       Holds information about one abstract connect address to match
       against RemoteAddress.
     */
    struct MatchAddress {
        // My client's name and its port's name for one abstract address
        std::string clid, clportid; 
        // remember what client this came from
        remid_t remid; 
        // The non-query portion of the abstract address
        std::string nodeid, clientid, portid;
        // The query portion
        psetmap_t patts;
    };
    using MatchAddresses = std::vector<MatchAddress>;

    /**
       Intermediate storage of what goes into a set of Zyre headers
    */
    struct YamZyreHeaders {
        std::vector<std::string> ports;
        std::map<std::string, std::vector<std::string>> binds, parms;
    };
}

#endif
