import os.path as osp

def listify(lst, delim=' '):
    if isinstance(lst, str):
        return lst.split(delim)
    return list(lst)

def keys_of_type(env, typ):
    keys = set()
    for hh in env:
        if hh.startswith(typ.upper() + "_"):
            h = hh.split("_",1)
            if h[1] == "ST":
                continue
            keys.add(h[1])
    keys = list(keys)
    keys.sort()
    return keys

def uses(ctx):
    uses = keys_of_type(ctx.env, "INCLUDES")
    uses += keys_of_type(ctx.env, "LIBPATH")
    uses = list(uses)
    uses.sort()
    return uses

def rpath(ctx):
    paths = keys_of_type(ctx.env, "LIBPATH")
    paths.append(ctx.env["PREFIX"] + '/lib')
    paths = list(set(paths))
    paths.sort()
    return paths
    

def generic_options(opt, name, libs=True, incs=True):
    ldash = name.lower().replace("_", "-")
    opt = opt.add_option_group(f'{name} Options')
    opt.add_option(f'--with-{ldash}', type='string', default=None,
                   help=f"{name} installation location")
    if incs:
        opt.add_option(f'--with-{ldash}-include', type='string', 
                       help=f'give {name} include location')
    if libs:
        opt.add_option(f'--with-{ldash}-lib', type='string', 
                       help=f'{name} lib location')    

def generic_configure_incs(cfg, name, incs, deps=()):
    incs = listify(incs)
    deps = listify(deps)

    lunder = name.lower()
    ldash = lunder.replace("_", "-")
    upper = name.upper().replace("-", "_")

    incpath = []
    incdir = getattr(cfg.options, f'with_{lunder}_include', None)
    if incdir:
        incpath = [incdir]
    if not incpath:
        withdir = getattr(cfg.options, f'with_{lunder}', None)
        if withdir:
            incpath = [osp.join(withdir, "include")]
    if not incpath:
        pp = getattr(cfg.options, 'prefix_path', None)
        for p in pp:
            for maybe in ['include', lunder+'/include', name+'/include']:
                adir = osp.join(p, maybe)
                if osp.exists(adir) and adir not in incpath:
                    incpath.append(adir)

    setattr(cfg.env, f'INCLUDES_{upper}', incpath)
    cfg.start_msg(f"Checking for {name} headers")
    for header in incs:
        cfg.check(features='cxx cxxprogram',
                  header_name=header,
                  use=[upper] + deps, uselib_store=upper)
    cfg.end_msg(cfg.env[f'INCLUDES_{upper}'])

def generic_configure_libs(cfg, name, libs, deps=()):
    libs = listify(libs)
    deps = listify(deps)

    lunder = name.lower()
    ldash = lunder.replace("_", "-")
    upper = name.upper().replace("-", "_")

    if isinstance(libs, str):
        libs = [libs]

    libpath = []
    libdir = getattr(cfg.options, f'with_{lunder}_lib', None)
    if libdir:
        libpath = [libdir]
    if not libpath:
        withdir = getattr(cfg.options, f'with_{lunder}', None)
        if withdir:
            for ll in ['lib', 'lib64']:
                mll = osp.join(withdir, ll)
                if osp.exists(mll) and mll not in libpath:
                    libpath.append(mll)
    if not libpath:
        pp = getattr(cfg.options, 'prefix_path', None)
        for p in pp:
            for maybe in ['lib', 'lib64',
                          lunder+'/lib', lunder+'/lib64',
                          name+'/lib', name+'/lib64']:
                adir = osp.join(p, maybe)
                if osp.exists(adir) and adir not in libpath:
                    libpath.append(adir)

    setattr(cfg.env, f'LIBPATH_{upper}', libpath)
    cfg.start_msg(f'Checking for {name} libs')
    for tryl in libs:
        cfg.check_cxx(lib=tryl,
                      use=[upper] + deps,
                      uselib_store=upper)
    cfg.end_msg(cfg.env[f'LIBPATH_{upper}'])
