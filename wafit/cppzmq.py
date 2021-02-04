#!/usr/bin/env waf
'''
This is a wafit tool for using cppzmq
'''
import util

def options(opt):
    opt.load("libzmq")
    util.generic_options(opt, "cppzmq", libs=False)

def configure(cfg):
    cfg.load("libzmq")
    util.generic_configure_incs(cfg, "cppzmq",
                                ["zmq.hpp", "zmq_addon.hpp"], "libzmq")

