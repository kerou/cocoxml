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

import os.path
import sys

package = 'Coco'
default_prefix = '/usr'

argmap = { 'prefix' : [default_prefix],
           'exec-prefix' : ['@prefix'],
           'bindir' : ['@exec-prefix', 'bin'],
           'sbindir' : ['@exec-prefix', 'sbin'],
           'libexecdir' : ['@exec-prefix', 'libexec'],
           'sysconfdir' : ['@prefix', 'etc'],
           'sharedstatedir' : ['@prefix', 'com'],
           'localstatedir' : ['@prefix', 'var'],
           'libdir' : ['@exec-prefix', 'lib'],
           'includedir' : ['@prefix', 'include'],
           'datarootdir' : ['@prefix', 'share'],
           'datadir' : ['@datarootdir'],
           'infodir' : ['@datarootdir', 'info'],
           'localedir' : ['@datarootdir', 'locale'],
           'mandir' : ['@datarootdir', 'man'],
           'docdir' : ['@datarootdir', 'doc', package],
           'htmldir' : ['@docdir'],
           'dvidir' : ['@docdir'],
           'pdfdir' : ['@docdir'],
           'psdir' : ['@docdir'],
           'program-prefix' : None,
           'program-suffix' : None }

valuemap = {}

for arg in sys.argv:
    if arg[:2] != '--': continue
    argsplit = arg[2:].split('=')
    if len(argsplit) != 2: continue
    if argsplit[0] not in argmap: continue
    valuemap[argsplit[0]] = argsplit[1]

def fillvalue(arg):
    value = argmap[arg]
    if value == None:
        valuemap[arg] = None
        return
    if value[0][0] == '@':
        refname = value[0][1:]
        if refname not in valuemap: fillvalue(refname)
        value = [valuemap[refname]] + value[1:]
    valuemap[arg] = os.path.join(*value)

for arg in argmap.keys():
    if arg in valuemap: continue
    fillvalue(arg)

output = open('config.status', 'w')
output.write(repr(valuemap))
output.close()
