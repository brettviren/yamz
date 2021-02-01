#!/usr/bin/env waf
'''
This is a wafit tool for using CZMQ
'''
import util

def options(opt):
    opt.load("zmq")    
    util.generic_options(opt, "czmq")
    

