#!/usr/bin/env waf
'''
This is a wafit tool for using nlohmann::json
'''
import util

def options(opt):
    util.generic_options(opt, "nljs", libs=False)

def configure(cfg):
    util.generic_configure_incs(cfg, "nljs", "nlohmann/json.hpp")

