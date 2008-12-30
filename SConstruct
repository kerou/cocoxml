# -*- python -*-
# Author: Charles Wang <charlesw123456@gmail.com>
# License: GPLv2

import os
import os.path
import sys
from CocoCustomTests import CocoCustomTests, CocoConfigure

topsrc = os.getcwd()
cpppath = [topsrc] + map(lambda subdir: os.path.join(topsrc, subdir),
                         ['core', 'algorithm', 'schemes'])

if sys.platform == 'win32':
    # The following Environment setup is used with MSVC in Win32.
    env = Environment(ENV = os.environ,
#                      CCFLAGS = ['-g', '-Wall'],
                      CPPPATH = cpppath)
    # The following Environment setup is used with gcc in Win32.
    #env = Environment(ENV = os.environ,
    #                  tools = ['gcc', 'g++', 'gnulink', 'gas', 'ar'],
    #                  CCFLAGS = ['-g', '-Wall'],
    #                  CPPPATH = cpppath)
elif sys.platform == 'linux2':
    # The following Environemtn setup is used in Linux.
    env = Environment(CCFLAGS = ['-g', '-Wall'],
                      CPPPATH = cpppath)
#
#print "env['TOOLS'] =", env['TOOLS']
#print "env['CCFLAGS'] =", env['CCFLAGS']

conf = Configure(env, config_h = 'acconfig.h', custom_tests = CocoCustomTests)
CocoConfigure(conf)
env = conf.Finish()

Export('env')

libobjs = []
cocosrc_libobjs = []
Export('libobjs', 'cocosrc_libobjs')

Export('env')
SConscript(os.path.join('core', 'SConscript'))
SConscript(os.path.join('algorithm', 'SConscript'))
SConscript(os.path.join('schemes', 'SConscript'))

lib = env.Library('coco', libobjs)
cocosrc_lib = env.Library('cocosrc', cocosrc_libobjs)
Export('lib', 'cocosrc_lib')

env.Program('Coco', ['Coco.c', cocosrc_lib, lib])
env.Program('CocoInit', ['CocoInit.c'])

SConscript(os.path.join('applications', 'SConscript'))
SConscript(os.path.join('tests', 'SConscript'))
