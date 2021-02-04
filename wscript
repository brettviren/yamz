#!/usr/bin/env waf
from wafit import WafIT, util
proj = WafIT("compiler_cxx render unit_test moo sml nljs cppzmq libzyre")

VERSION="0.0.0"
APPNAME="yamz"
DESCRIPTION="Network discovery with you and me, Zyre"

# def options(opt):
#     opt.load("does-not-exist")
def options(opt):
    proj.options(opt)

def configure(cfg):
    cfg.env.CXXFLAGS += ['-std=c++17', '-ggdb3', '-Wall', '-Werror']
    proj.configure(cfg)
    for inc in "sml nljs cppzmq libzmq libczmq libzyre".split():
        upper=inc.upper()
        assert(cfg.env[f"INCLUDES_{upper}"])

    for lib in "libzmq libczmq libzyre".split():
        upper=lib.upper()
        if not cfg.env[f"LIBPATH_{upper}"]:
            raise RuntimeError(f"No libraries for {upper}")

from waflib.Utils import subst_vars
from subprocess import check_output
def import_scanner(task):
    deps = []
    for node in task.inputs:
        cmd = "${MOO} imports %s" % node.abspath()
        cmd = subst_vars(cmd, task.env)
        # out = task.exec_command(cmd)
        out = check_output(cmd.split()).decode()
        deps += out.split("\n")

    deps = [task.generator.bld.path.find_or_declare(d) for d in deps if d]
    return (deps, [])

def build(bld):
    proj.build(bld)

    if "MOO" in bld.env:
        print("regenerate")
        model = bld.path.find_resource("src/yamz-model.jsonnet")
        incdir = bld.path.find_resource("inc/yamz/util.hpp").parent
        struct = incdir.make_node("Structs.hpp")
        nljs = incdir.make_node("Nljs.hpp")
        cmd = "${MOO} render -o ${TGT} ${SRC}"
        bld(rule=cmd + " ostructs.hpp.j2",
            source=model, target=struct,
            scan=import_scanner)
        bld(rule=cmd + " onljs.hpp.j2",
            source=model, target=nljs,
            scan=import_scanner)

    sources = bld.path.ant_glob('src/*.cpp')
    bld.shlib(features='cxx',
              includes='inc',
              rpath=util.rpath(bld),
              source=sources, target='yamz',
              uselib_store='YAMZ', use=util.uses(bld))

    bld.install_files('${PREFIX}/include/yamz', 
                      bld.path.ant_glob("inc/yamz/**/*.hpp"),
                      cwd=bld.path.find_dir('inc/yamz'),
                      relative_trick=True)

    bld(source='libyamz.la.in', target='libyamz.la',
        install_path = '${LIBDIR}/lib/',
        features='render subst')
    bld(source='libyamz.pc.in', target='libyamz.pc',
        install_path = '${LIBDIR}/pkgconfig/',
        features='render subst')
    

    bld.unit_test(bld.path.ant_glob("test/test*.cpp"), "inc src")

