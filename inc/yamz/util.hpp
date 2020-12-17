#ifndef YAMZ_UTIL_HPP
#define YAMZ_UTIL_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>

namespace yamz {

    // a set of "parameters" or "patterns"
    using pset_t = std::set<std::string>;
    // map a key to a set of parameters
    using psetmap_t = std::map<std::string, pset_t>;

    // Return true if patterns match parameters
    bool match(const psetmap_t& patterns, const psetmap_t& parameters);


    // Parse a string as if it holds an array of key=val pairs.
    // Keys and values have leading and trailing spaces removed.
    // Eg: "k1=v1, k2 = v2" -> {{"k1","v1"}, {"k2","v2"}}
    std::map<std::string, std::string>
    parse_map(const std::string& str, 
              std::string comma=",",
              std::string equals="=");

    std::string str(const std::map<std::string, std::string>& mss,
                    std::string comma=",",
                    std::string equals="=");


    // Parse a string as if it holds an array
    // Values have leading and trailing spaces removed.
    // Eg: "v1, v2" -> {"v1", "v2"}
    std::vector<std::string>
    parse_list(const std::string& str,
               std::string comma=",");

    std::string str(const std::vector<std::string>& vs,
                    std::string comma=",");

    // Return true if the address is abstract
    bool is_abstract(const std::string& addr);


    // Divine "the" IP address that peers would likly see the host.
    // This uses the "UDP trick".  It may return the "wrong" answer on
    // hosts with multiple gateways.
    std::string myip();

    /*! Return true when a signal has been sent to the application.
     *
     * Exit main loop if ever true.
     *
     * The catch_signals() function must be called in main() for
     * this to ever return true.
     */
    bool interrupted();

    /*! Catch signals and set interrupted to true.
     *
     *  This must be called from main() if interrupted() wil ever
     *  return true.  */
    void catch_signals();


}


#endif
