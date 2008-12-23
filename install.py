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

destroot = ''
for arg in sys.argv:
    if arg[:9] == 'DESTROOT=': destroot = arg[9:]

def execname(name):
    bname = os.path.basename(name)
    if cfgmap['program-prefix']: bname = cfgmap['program-prefix'] + bname
    if cfgmap['program-suffix']: bname = bname + cfgmap['program-suffix']
    return os.path.join(os.path.dirname(name), bname)

def rootjoin(destroot, destpath):
    if destroot == '': return destpath
    if destpath[0] == '/': destpath = destpath[1:]
    return os.path.join(destroot, destpath)

def install_lines(destroot, tgtfile, lines):
    tgtfile = rootjoin(destroot, tgtfile)
    tgtdir = os.path.dirname(tgtfile)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtf = open(tgtfile, 'w')
    tgtf.write(string.join(lines, '\n'))
    tgtf.close()

def install(destroot, tgtdir, srcfile):
    tgtdir = rootjoin(destroot, tgtdir)
    if not os.path.isdir(tgtdir): os.makedirs(tgtdir)
    tgtfile = os.path.join(tgtdir, os.path.basename(srcfile))
    shutil.copyfile(srcfile, tgtfile)
    shutil.copystat(srcfile, tgtfile)

# Real installations.
install(destroot, cfgmap['bindir'], execname('Coco'))
install(destroot, cfgmap['bindir'], execname('CocoInit'))
install(destroot, cfgmap['libdir'], 'libcoco.a')
for hdr in ['Buffer.h', 'CDefs.h', 'ErrorPool.h', 'Position.h',
            'Token.h', 'XmlScanOper.h']:
    install(destroot,
            os.path.join(cfgmap['includedir'], 'Coco', 'c'),
            os.path.join('schemes', 'c', hdr))
pclines = []
pclines.append('prefix=%s' % cfgmap['prefix'])
pclines.append('includedir=%s' % cfgmap['includedir'])
pclines.append('')
pclines.append('Name: CocoXml')
pclines.append('Description: Coco/R & CocoXml in C.')
pclines.append('Version: 0.9.1')
pclines.append('Libs: -lcoco')
pclines.append('Cflags: -I${includedir}/Coco')
install_lines(destroot,
              os.path.join(cfgmap['libdir'], 'pkgconfig', 'cocoxml.pc'),
              pclines)

install(destroot, cfgmap['docdir'], 'README')

tempdir = os.path.join(cfgmap['datadir'], 'Coco')

tgtdir = os.path.join(tempdir, 'dump')
for f in glob.glob(os.path.join('schemes', 'dump', '*.html')):
    install(destroot, tgtdir, f)

tgtdir = os.path.join(tempdir, 'c')
for f in ['Scanner', 'Parser']:
    install(destroot, tgtdir, os.path.join('schemes', 'c', f + '.h'))
    install(destroot, tgtdir, os.path.join('schemes', 'c', f + '.c'))
install_lines(destroot, os.path.join(tgtdir, 'PREFIX'), ['Ccs'])

tgtdir = os.path.join(tempdir, 'cxml')
for f in ['Scanner4Xml', 'Parser4Xml']:
    install(destroot, tgtdir, os.path.join('schemes', 'c', f + '.h'))
    install(destroot, tgtdir, os.path.join('schemes', 'c', f + '.c'))
install_lines(destroot, os.path.join(tgtdir, 'PREFIX'), ['Ccx'])

tgtdir = os.path.join(tempdir, 'csharp')
for f in ['Buffer', 'ErrorPool', 'Position', 'Token', 'Scanner', 'Parser']:
    install(destroot, tgtdir, os.path.join('schemes', 'csharp', f + '.cs'))
install_lines(destroot, os.path.join(tgtdir, 'PREFIX'), ['Ccs'])
