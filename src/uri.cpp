#include "yamz/uri.hpp"
#include <regex>

using namespace yamz;

// Helper to parse URI query string, eg "foo=bar&baz=quax"
static 
uri::queries_t parse_queries(std::string qs)
{
    uri::queries_t ret;
    if (qs.empty()) {
        return ret;
    }
    while(true) {
        auto sep = qs.find_first_of("&");
        std::string one;
        if (sep == qs.npos) {
            one = qs;
        }
        else {
            one = qs.substr(0, sep);
        }
        auto kitr = one.find_first_of("=");
        if (kitr == one.npos) {
            ret.emplace_back(one, "");
        }
        else {
            auto key = one.substr(0,kitr);
            auto val = one.substr(kitr+1);
            ret.emplace_back(key, val);
        }
        if (sep == qs.npos) {
            break;
        }
        qs = qs.substr(sep+1);
    }
    return ret;
}

std::string yamz::str(const uri::queries_t& queries)
{
    std::string ret = "";
    std::string comma = "?";
    for (const auto& [key,val] : queries) {
        ret += comma + key + "=" + val;
        comma = "&";
    }
    return ret;
}
// as rediculous as this function is, it probably is still not correct.
std::string yamz::str(const uri::Parts& p, bool withq) 
{
    std::string ret;
    if (! p.scheme.empty()) {
        ret += p.scheme + ":";
    }
    if (p.scheme == "file" or p.scheme.empty()) {
        if (p.domain.empty()) {
            // legal
            if (p.scheme == "file" and p.path[0] == '/') {
                ret += "//";
            }                        
            ret += p.path;
        }
        else {
            // illegal case of file://file-as-domain
            ret += "//" + p.domain + p.path;
        }
    }
    else {
        if (p.scheme != "mailto") {
            ret += "//" + p.domain;
        }
        if (! p.port.empty()) {
            ret += ":" + p.port;
        }
        ret += p.path;
    }
    if (withq) {
        ret += yamz::str(p.queries);
    }
    return ret;
}

static
std::string get_match(const std::cmatch& what, size_t ind) {
    return std::string(what[ind].first, what[ind].second);
}

uri::Parts uri::parse(std::string uristr)
{
    // taken from random SE's plus add port finding
    std::regex ex (
        R"(^(([^:\/?#]+):)?(//([^\/:?#]*)(:([0-9]+))?)?([^?#]*)(\?([^#]*))?(#(.*))?)",
        std::regex::extended
        );
    std::cmatch what;

    if(regex_match(uristr.c_str(), what, ex)) {
        return uri::Parts{
            get_match(what, 2), // scheme
            get_match(what, 4), // domain
            get_match(what, 6), // port
            get_match(what, 7), // path
            parse_queries(get_match(what, 9))};
    }
    throw std::runtime_error("failed to parse: " + uristr);
}
