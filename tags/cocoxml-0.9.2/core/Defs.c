/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the 
  Free Software Foundation; either version 2, or (at your option) any 
  later version.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.

  You should have received a copy of the GNU General Public License along 
  with this program; if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
#include  "Defs.h"

void *
_CcMalloc_(size_t size, const char * fname, int line)
{
    void * ptr = _CcsMalloc_(size, fname, line);
    if (ptr) return ptr;
    fprintf(stderr, "Malloc failed in %s#%d\n", fname, line);
    exit(-1);
}

void *
_CcRealloc_(void * ptr, size_t size, const char * fname, int line)
{
    ptr = _CcsRealloc_(ptr, size, fname, line);
    if (ptr) return ptr;
    fprintf(stderr, "Realloc failed in %s#%d\n", fname, line);
    exit(-1);
}

void
_CcFree_(void * ptr, const char * fname, int line)
{
    _CcsFree_(ptr, fname, line);
}

char *
_CcStrdup_(const char * str, const char * fname, int line)
{
    char * ptr = _CcsStrdup_(str, fname, line);
    if (ptr) return ptr;
    fprintf(stderr, "Strdup failed in %s#%d\n", fname, line);
    exit(-1);
}

char *
CcUnescape(const char * str)
{
    char * s = CcsUnescape(str);
    if (s) return s;
    _CcsAssertFailed_("Invalid character encountered or out of memory",
		      __FILE__, __LINE__);
    return NULL;
}

char *
CcEscape(const char * str)
{
    char * s = CcsEscape(str);
    if (s) return s;
    _CcsAssertFailed_("Out of memory", __FILE__, __LINE__);
    return NULL;
}
