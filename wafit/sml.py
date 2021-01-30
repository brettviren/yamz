#!/usr/bin/env waf
'''This is a wafit tool for using boost.sml.  Desptie the name, it
does not depend on boost.
'''
import util

def options(opt):
    util.generic_options(opt, "sml", libs=False)

def configure(cfg):
    util.generic_configure_incs(cfg, "sml", ["boost/sml.hpp"])

