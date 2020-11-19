#!/usr/bin/env waf
import os.path as osp
from waflib.Tools import waf_unit_test
from subprocess import check_output
from waflib.Utils import subst_vars

VERSION='0.0.0'
APPNAME='yamz'


def import_scanner(task):
    deps = []
    for node in task.inputs:
        cmd = "${MOO} imports %s" % node.abspath()
        cmd = subst_vars(cmd, task.env)
        # out = task.exec_command(cmd)
        out = check_output(cmd.split()).decode()
        deps += out.split("\n")

    deps = [task.generator.bld.path.find_or_declare(d) for d in deps if d]
    print(deps)
    return (deps, [])


def options(opt):
    opt.load('compiler_cxx')
    opt.load('waf_unit_test')
    opt.add_option('--quell-tests', action='store_true', default=False,
                   help='Compile but do not run the tests (default=%default)')
    opt.add_option('--with-libzmq', default=None,
                   help='Set to libzmq install area')
    opt.add_option('--with-nljs', default=None,
                   help='Point nlohmann json install area')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('waf_unit_test')

    cfg.find_program('moo', var='MOO', mandatory=False)

    cfg.env.CXXFLAGS += ['-std=c++17', '-ggdb3', '-Wall', '-Werror']

    # nlohmann::json
    nljs = getattr(cfg.options, 'with_nljs', None)
    if nljs:
        print("using " + nljs)
        setattr(cfg.env, 'INCLUDES_NLJS', [osp.join(nljs, "include")])

    cfg.check(features='cxx cxxprogram', define_name='HAVE_NLJS',
              header_name='nlohmann/json.hpp',
              use='NLJS', uselib_store='NLJS', mandatory=True)

    # libzmq
    zmq = getattr(cfg.options, "with_libzmq", None)
    if zmq:
        setattr(cfg.env, 'RPATH_LIBZMQ', [osp.join(zmq, 'lib')])
        setattr(cfg.env, 'LIBPATH_LIBZMQ', [osp.join(zmq, 'lib')])
        setattr(cfg.env, 'INCLUDES_LIBZMQ', [osp.join(zmq, 'include')])
    cfg.check_cfg(package='libzmq', uselib_store='ZMQ',
                  mandatory=True, args='--cflags --libs')

    # libzyre
    zyre = getattr(cfg.options, "with_libzyre", None)
    if zyre:
        setattr(cfg.env, 'RPATH_LIBZYRE', [osp.join(zyre, 'lib')])
        setattr(cfg.env, 'LIBPATH_LIBZYRE', [osp.join(zyre, 'lib')])
        setattr(cfg.env, 'INCLUDES_LIBZYRE', [osp.join(zyre, 'include')])
    cfg.check_cfg(package='libzyre', uselib_store='ZYRE',
                  mandatory=True, args='--cflags --libs')

    # libczmq
    czmq = getattr(cfg.options, "with_libczmq", None)
    if czmq:
        setattr(cfg.env, 'RPATH_LIBCZMQ', [osp.join(czmq, 'lib')])
        setattr(cfg.env, 'LIBPATH_LIBCZMQ', [osp.join(czmq, 'lib')])
        setattr(cfg.env, 'INCLUDES_LIBCZMQ', [osp.join(czmq, 'include')])
    cfg.check_cfg(package='libczmq', uselib_store='CZMQ',
                  mandatory=True, args='--cflags --libs')


    cfg.check(features='cxx cxxprogram', lib=['pthread'],
              uselib_store='PTHREAD')

def build(bld):

    use = ['ZYRE', 'ZMQ', 'NLJS']

    if "MOO" in bld.env:
        print("regenerate")
        model = bld.path.find_resource("src/yamz-model.jsonnet")
        srcdir = model.parent
        struct = srcdir.make_node("Structs.hpp")
        nljs = srcdir.make_node("Nljs.hpp")
        cmd = "${MOO} render -o ${TGT} ${SRC}"
        bld(rule=cmd + " ostructs.hpp.j2",
            source=model, target=struct,
            scan=import_scanner)
        bld(rule=cmd + " onljs.hpp.j2",
            source=model, target=nljs,
            scan=import_scanner)

    sources = bld.path.ant_glob('src/*.cpp')
    bld.shlib(features='cxx',
              source=sources, target='yamz',
              uselib_store='YAMZ', use=use)

    tsources = bld.path.ant_glob('test/test*.cpp')
    if tsources and not bld.options.no_tests:
        # fixme: it would be nice to have an option that builds but doesn't run
        features = 'test cxx'
        if bld.options.quell_tests:
            features = 'cxx'

        for tmain in tsources:
            uses = use + ['PTHREAD']
            includes = ['test']
            if tmain.name.startswith("test_yamz_"):  # depends on yamz internals
                uses.insert(0, "yamz")
                includes.append('src')

            bld.program(features=features,
                        source=[tmain], target=tmain.name.replace('.cpp', ''),
                        ut_cwd=bld.path,
                        install_path=None,
                        includes=includes,
                        #rpath=rpath,
                        use=uses)

    bld.add_post_fun(waf_unit_test.summary)
