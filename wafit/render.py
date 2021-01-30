#!/usr/bin/env waf

from waflib import Context
from waflib.TaskGen import feature, before_method

@feature('render')
@before_method('process_subst')
def process_render(self):
    '''Provide the "render" feature.

    It actually relies on "subst" feature to do the heavy lifting.
    This feature simply adds more variables to the model.

    '''
    def set_maybe(attr, default=None):
        if not hasattr(self, attr):
            setattr(self, attr, getattr(Context.g_module, attr, default))

    set_maybe('APPNAME')
    set_maybe('VERSION')
    set_maybe('DESCRIPTION', self.APPNAME)
    mypcname = None
    pcnames = list()
    for key, val in self.env.items:

        if key == 'PCNAME_' + self.APPNAME.upper():
            mypcname = val
            continue
        if key.startswith("PCNAME_"):
            pcnames.append(val)
            continue

        set_maybe(key, val)

    set_maybe('PCNAME', mypcname)
    set_maybe('PCREQUIRES', ' '.join(pcnames))
    set_maybe('LLIBS', '-l'+self.APPNAME)

    #print(dir(self))
