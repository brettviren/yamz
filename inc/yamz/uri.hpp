/**
 * @file yamz/uri.hpp
 */
#ifndef YAMZ_URI_HPP
#define YAMZ_URI_HPP

#include <string>
#include <vector>

namespace yamz {

    namespace uri {

        using query_t = std::pair<std::string, std::string>;
        using queries_t = std::vector<query_t>;
        struct Parts {
            std::string scheme;
            std::string domain;
            std::string port;
            std::string path;
            queries_t queries;
        };
        /**
           Parse many but not all URI forms given by
           https://www.ietf.org/rfc/rfc2396.txt

           A file is specified as with a URI with a "file:" scheme or as a path.

           With "file:" scheme:

           - absolute :: file:///data/foo.json
           - relative :: file:foo.json

           That relative notation is an extension to rfc2396.
           O.w. the file scheme places after the "://" a
           "authoritative" aka "domain.  Thus two or four or more
           slashes are illegal:

           - illegal :: file://foo.json    # this makes "foo.json" a "domain"
           - illegal :: file:////foo.json  # we are not MicroSoft

           As a schemeless path:

           - absolute :: /data/foo.json
           - relative :: foo.json

           Both "file:" scheme and schemeless forms may take a URI
           query parameter string.  

           - example :: file:///data/foo.json?fmt=jstream
        */
        Parts parse(std::string uri);

    }

    // Restringify Parts w/ or w/out queries 
    std::string str(const uri::Parts& parts, bool withq=true);
    std::string str(const uri::queries_t& queries);

} // namespace yamz


#endif
