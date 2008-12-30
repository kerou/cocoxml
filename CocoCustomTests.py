# -*- python -*-
# Author: Charles Wang <charlesw123456@gmail.com>
# License: GPLv2

def CheckFunc0(context, func_call, headers):
    context.Message('Checking for %s ...' % func_call)
    ret = context.TryLink("""%s
int main(void) { %s; return 0; }
""" % (headers, func_call), '.c')
    context.Result(ret)
    return ret

CocoCustomTests = { 'CheckFunc0' : CheckFunc0 }

def CocoConfigure(conf):
    if conf.CheckFunc0('readdir_r((void *)0, (void *)0, (void *)0)',
                       '#include <dirent.h>'):
        conf.Define('HAVE_READDIR_R', 1)
