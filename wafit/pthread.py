import util

def options(opt):
    util.generic_options(opt, "pthread", incs=False)

def configure(cfg):
    util.generic_configure_libs(cfg, "pthread", ["pthread"])    
