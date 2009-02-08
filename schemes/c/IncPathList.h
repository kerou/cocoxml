/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_INCPATHLIST_H
#define  COCO_INCPATHLIST_H

#ifndef  COCO_CDEFS_H
#include  "c/CDefs.h"
#endif

EXTC_BEGIN

struct CcsIncPathList_s {
    CcsBool_t AbsPathUsed;
    CcsBool_t IncluderUsed;
    char * start;
};

CcsIncPathList_t *
CcsIncPathList(CcsBool_t AbsPathUsed, CcsBool_t IncluderUsed,
	       const char * const * incpatharr, size_t numpathes);
CcsIncPathList_t *
CcsIncPathListV(CcsBool_t AbsPathUsed, CcsBool_t IncluderUsed,
		const char * incpath, ...);
void CcsIncPathList_Destruct(CcsIncPathList_t * self);

FILE *
CcsIncPathList_Open(const CcsIncPathList_t * self, char * fnbuf, size_t szfnbuf,
		    const char * includerFname, const char * fname);

EXTC_END

#endif  /* COCO_INCPATHLIST_H */
