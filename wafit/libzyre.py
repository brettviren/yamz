#!/usr/bin/env waf
'''
This is a wafit tool for using zyre
'''
import util

def options(opt):
    opt.load("libczmq")
    util.generic_options(opt, "libzyre", libs=False)
    

def configure(cfg):
    cfg.load("libczmq")
    util.generic_configure_incs(cfg, "libzyre", "zyre.h", "libczmq")
    util.generic_configure_libs(cfg, "libzyre", "zyre", "libczmq")

