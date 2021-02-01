#!/usr/bin/env waf
'''
This is a wafit tool for using zyre
'''
import util

def options(opt):
    opt.load("czmq")    
    util.generic_options(opt, "zyre", libs=False)
    

def configure(cfg):
    cfg.load("czmq")
    util.generic_configure_incs(cfg, "zyre", "zyre.h")
    util.generic_configure_libs(cfg, "zyre", "zyre")

