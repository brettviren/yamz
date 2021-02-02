#!/usr/bin/env waf
'''
This is a wafit tool for using CZMQ
'''
import util

def options(opt):
    opt.load("libzmq")
    util.generic_options(opt, "libczmq")
    

def configure(cfg):
    cfg.load("libzmq")
    util.generic_configure_incs(cfg, "libczmq", "czmq.h")
    util.generic_configure_libs(cfg, "libczmq", "czmq", "zmq")

