#include "yamz/util.hpp"

#include <sstream>
#include <algorithm>
#include <regex>

std::vector<std::string>
yamz::parse_list(const std::string& str, std::string comma)
{
    std::vector<std::string> ret;
    std::regex rgx("\\s*"+comma+"\\s*");
    std::sregex_token_iterator end, iter(str.begin(), str.end(), rgx, -1);
    for ( ; iter != end; ++iter) {
        ret.push_back(*iter); 
    }
    return ret;
}

std::map<std::string, std::string>
yamz::parse_map(const std::string& str, std::string comma, std::string equals)
{
    std::map<std::string, std::string> ret;
    for (const auto& one : parse_list(str, comma)) {
        auto two = parse_list(one, equals);
        ret[two[0]] = two[1];
    }
    return ret;
}


std::string
yamz::str(const std::map<std::string, std::string>& mss,
          std::string comma, std::string equals)
{
    std::stringstream ss;
    std::string mycomma = "";
    for (const auto& [key,val] : mss) {
        ss << mycomma << key << equals << val;
        mycomma = comma;
    }
    return ss.str();
}

std::string
yamz::str(const std::vector<std::string>& vs, std::string comma)
{
    std::stringstream ss;
    std::string mycomma="";
    for (const auto& one : vs) {
        ss << mycomma << one;
        mycomma = comma;
    }
    return ss.str();
}

bool
yamz::is_abstract(const std::string& addr)
{
    if (addr.size() < 5) { return false; }
    return addr.substr(0,5) == "yamz:";
}



// Return true if patterns match parameters
bool yamz::match(const psetmap_t& patts, const psetmap_t& parms)
{
    // double loop, bail asap on any failure
    for (const auto& [key, patt] : patts) {
        auto it = parms.find(key);
        if (it == parms.end()) {
            // no matching parameter
            return false;
        }
        for (const auto& res : patt) { // set of regex strnigs
            // Fixme: would like to avoid re-making these regex eg by
            // storing these in an "msetmap_t" alraedy as std::regex
            // but this type is not hashed so doesn't like to live in
            // a set.
            std::regex re(res);
            bool gotone = false;
            for (const auto& pa : it->second) {
                std::smatch m;
                if (std::regex_match(pa, m, re)) {
                    gotone = true;
                    break;
                }
            }
            if (!gotone) {
                // no parameter matches a pattern of this key.
                return false;
            }
        }
    }
    // survived, exhausted, but true
    return true;
}

