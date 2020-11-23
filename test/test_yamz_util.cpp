#include "yamz/util.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

int main()
{
    std::string header = "comp1 / portA : PUB= tcp://127.0.0.1:8065 ,inproc://portA; comp1/portB:SUB=tcp://127.0.0.1:8064,inproc://portb ;comp2/port:PUSH =ipc://comp2port.fifo";

    auto res = yamz::parse_header(header);
    std::string roundtrip="";

    std::string semicolon = "";
    for (const auto& one : res) {
        for (auto& cp : one.second) {
            roundtrip += semicolon;
            semicolon = ";";
            std::string key = one.first + "/" + cp.portid
                + ":" + yamz::str(cp.ztype);
            std::string val="";
            std::string comma = "";
            for (const auto& addr : cp.concs) {
                val += comma + addr;
                comma = ",";
            }
            roundtrip += key + "=" + val;
        }
    }

    std::cerr << "original:  " << header << std::endl;
    header.erase(std::remove_if(header.begin(), header.end(), isspace),
                 header.end());
    std::cerr << "spaceless: " << header << std::endl;
    std::cerr << "roundtrip: " << roundtrip << std::endl;
    
    assert(roundtrip == header);
    return 0;
}
