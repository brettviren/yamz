#!/usr/bin/env waf
'''
This is a wafit tool for using ZeroMQ libzmq
'''
import util

def options(opt):
    opt.load("pthread")
    util.generic_options(opt, "zmq")

def configure(cfg):
    cfg.load("pthread")
    util.generic_configure_incs(cfg, "zmq", ["zmq.h"])
    util.generic_configure_libs(cfg, "zmq", ["zmq"])    
    

