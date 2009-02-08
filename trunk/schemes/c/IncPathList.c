/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#include  <limits.h>
#include  "IncPathList.h"

CcsIncPathList_t *
CcsIncPathList(CcsBool_t AbsPathUsed, CcsBool_t IncluderUsed,
	       const char * const * incpatharr, size_t numpathes)
{
    CcsIncPathList_t * self;
    const char * const * cur0;
    size_t totallen;
    char * cur1;

    totallen = 0;
    for (cur0 = incpatharr; *cur0; ++cur0)
	totallen += strlen(*cur0) + 1;
    ++totallen;
    if (!(self = CcsMalloc(sizeof(CcsIncPathList_t) + totallen)))
	return NULL;
    self->AbsPathUsed = AbsPathUsed;
    self->IncluderUsed = IncluderUsed;
    self->start = cur1 = (char *)(self + 1);
    for (cur0 = incpatharr; cur0 - incpatharr < numpathes; ++cur0) {
	strcpy(cur1, *cur0);
	cur1 += strlen(cur1) + 1;
    }
    *cur1 = 0;
    return self;
}
CcsIncPathList_t *
CcsIncPathListV(CcsBool_t AbsPathUsed, CcsBool_t IncluderUsed,
		const char * incpath, ...)
{
    CcsIncPathList_t * self;
    const char * cur0;
    va_list ap;
    size_t totallen;
    char * cur1;

    totallen = 0;
    va_start(ap, incpath);
    for (cur0 = incpath; cur0; cur0 = va_arg(ap, const char *))
	totallen = strlen(cur0) + 1;
    ++totallen;
    va_end(ap);
    if (!(self = CcsMalloc(sizeof(CcsIncPathList_t) + totallen)))
	return NULL;
    self->AbsPathUsed = AbsPathUsed;
    self->IncluderUsed = IncluderUsed;
    self->start = cur1 = (char *)(self + 1);
    va_start(ap, incpath);
    for (cur0 = incpath; cur0; cur0 = va_arg(ap, const char *)) {
	strcpy(cur1, cur0);
	cur1 += strlen(cur1) + 1;
    }
    va_end(ap);
    *cur1 = 0;
    return self;
}

void
CcsIncPathList_Destruct(CcsIncPathList_t * self)
{
    CcsFree(self);
}

FILE *
CcsIncPathList_Open(const CcsIncPathList_t * self, char * fnbuf, size_t szfnbuf,
		    const char * includerFname, const char * fname)
{
    FILE * fp;
    char tmp[PATH_MAX];
    const char * cur;
    if (!self || self->AbsPathUsed) {
	strncpy(fnbuf, fname, szfnbuf);
	if ((fp = fopen(fnbuf, "r"))) return fp;
    }
    if ((!self || self->IncluderUsed) && includerFname) {
	CcsDirname(tmp, sizeof(tmp), includerFname);
	CcsPathJoin(fnbuf, szfnbuf, tmp, fname, NULL);
	if ((fp = fopen(fnbuf, "r"))) return fp;
    }
    if (self) {
	for (cur = self->start; *cur; cur += strlen(cur) + 1) {
	    CcsPathJoin(fnbuf, szfnbuf, cur, fname, NULL);
	    if ((fp = fopen(fnbuf, "r"))) return fp;
	}
    }
    return NULL;
}
