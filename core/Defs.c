/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
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
