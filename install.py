#!/usr/bin/python
#  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the 
#  Free Software Foundation; either version 2, or (at your option) any 
#  later version.
#
#  This program is distributed in the hope that it will be useful, but 
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
#  for more details.
#
#  You should have received a copy of the GNU General Public License along 
#  with this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import glob
import os.path
import shutil
import string
import sys

cfgfile = open('config.status')
cfgmap = eval(cfgfile.read())
cfgfile.close()

destdir = ''
for arg in sys.argv:
    if arg[:8] == 'DESTDIR=': destdir = arg[8:]

if destdir != '':
    for k in cfgmap.keys():
        if cfgmap[k] is None: continue
        if k[-6:] != 'prefix' and k[-3:] != 'dir': continue
        if cfgmap[k][0] == '/': cfgmap[k] = cfgmap[k][1:]
        cfgmap[k] = os.path.join(destdir, cfgmap[k])

def execname(name):
    bname = os.path.basename(name)
    if cfgmap['program-prefix']: bname = cfgmap['program-prefix'] + bname
    if cfgmap['program-suffix']: bname = bname + cfgmap['program-suffix']
    return os.path.join(os.path.dirname(name), bname)

def install_lines(tgtfile, lines):
    tgtdir = os.path.dirname(tgtfile)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtf = open(tgtfile, 'w')
    tgtf.write(string.join(lines, '\n'))
    tgtf.close()

def install(tgtdir, srcfile):
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtfile = os.path.join(tgtdir, os.path.basename(srcfile))
    shutil.copyfile(srcfile, tgtfile)
    shutil.copystat(srcfile, tgtfile)

# Real installations.
install(cfgmap['bindir'], execname('Coco'))
install(cfgmap['bindir'], execname('CocoInit'))

install(cfgmap['docdir'], 'README')

tempdir = os.path.join(cfgmap['datadir'], 'Coco')

tgtdir = os.path.join(tempdir, 'dump')
for f in glob.glob(os.path.join('schemes', 'dump', '*.html')):
    install(tgtdir, f)

tgtdir = os.path.join(tempdir, 'c')
for f in ['Buffer', 'CDefs', 'ErrorPool', 'Position', 'Token',
          'Scanner', 'Parser']:
    install(tgtdir, os.path.join('schemes', 'c', f + '.h'))
    install(tgtdir, os.path.join('schemes', 'c', f + '.c'))
install_lines(os.path.join(tgtdir, 'PREFIX'), ['Ccs'])

tgtdir = os.path.join(tempdir, 'cxml')
for f in ['CDefs', 'ErrorPool', 'Token', 'Scanner4Xml', 'Parser4Xml']:
    install(tgtdir, os.path.join('schemes', 'c', f + '.h'))
    install(tgtdir, os.path.join('schemes', 'c', f + '.c'))
install_lines(os.path.join(tgtdir, 'PREFIX'), ['Ccx'])
