#!/usr/bin/env python3

import os
import sys

from .util import *
from waflib import Context

# so ctx.load("tool") finds our tools
sys.path.append(os.path.dirname(__file__))

class WafIT:
    def __init__(self, tools = []):
        self.tools = listify(tools)

    @property
    def options(self):
        def options_f(opt):
            opt.add_option('--prefix-path', action="append",
                           help="Path to search for installed files")
            for tool in self.tools:
                opt.load(tool)
        return options_f

    @property
    def configure(self):
        def configure_f(cfg):
            for tool in self.tools:
                cfg.load(tool)
        return configure_f

    @property
    def build(self):
        def build_f(bld):
            for tool in self.tools:
                bld.load(tool)
        return build_f

