#!/usr/bin/env waf
'''A waf tool for adding unit test to a c++ package.  

Besides standard waf commands it provides adds a .unit_test() method
to the build context which should be called in order to generate the
tasks needed to build and run your unit tests.

'''

import util
from waflib.Tools import waf_unit_test

def options(opt):
    opt.load('waf_unit_test')
    opt.add_option('--quell-tests', action='store_true', default=False,
                   help='Compile but do not run the tests (default=%default)')
    

def configure(cfg):
    cfg.load('waf_unit_test')

from waflib import Context
from waflib.Configure import conf
@conf
def unit_test(bld, sources, incs=(), uses=(), csources=()):
    '''Register unit tests.

    Example usage

    >>> def build(bld):
    >>>     tsrcs=bld.path.ant_glob("test/test*.cpp")
    >>>     bld.unit_test(tsrcs, "inc src")

    The sources should list all unit test main source files.
    
    The incs may give any special include directories such as the
    source directory if tests require access to private headers.

    The uses can specify additional dependencies beyond what the
    package requires.

    The csources are like sources but for to make "check" programs
    which are not executed as unit tests and not installed but
    available under build directory to run for local checking.

    '''
    sources = util.listify(sources)
    incs = util.listify(incs)

    me = getattr(Context.g_module, 'APPNAME', None)
    uses = util.listify(uses) + util.uses(bld)
    if me:
        uses.insert(0, me)

    if bld.options.no_tests:
        return
    if not sources:
        return

    features = 'test cxx'
    if bld.options.quell_tests:
        features = 'cxx'

    rpath = util.rpath(bld) + [bld.path.find_or_declare(bld.out_dir)]

    for tmain in sources:
        bld.program(features=features,
                    source=[tmain],
                    target=tmain.name.replace('.cpp', ''),
                    ut_cwd=bld.path,
                    install_path=None,
                    includes=incs,
                    rpath=rpath,
                    use=uses)
        
    for cmain in csources:
        bld.program(features = 'cxx',
                    source=[cmain],
                    target = cmain.name.replace('.cpp',''),
                    ut_cwd=bld.path,
                    install_path=None,
                    includes=incs,
                    rpath=rpath,
                    use=uses)


    bld.add_post_fun(waf_unit_test.summary)
    
