/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_TOKEN_H
#define  COCO_TOKEN_H

#ifndef  COCO_ERRORPOOL_H
#include  "ErrorPool.h"
#endif

EXTC_BEGIN

struct CcsToken_s
{
    CcsToken_t     * next;
    void          (* destructor)(CcsToken_t * self);
    CcsScanInput_t * input;
    int              refcnt;
    int              kind;
    CcsLocation_t    loc;
    int              pos;
    char           * val;
};

CcsToken_t *
CcsToken(CcsScanInput_t * input, int kind, const char * fname, int pos,
	 int line, int col, const char * val, size_t vallen);
void CcsToken_Destruct(CcsToken_t * self);

EXTC_END

#endif  /* COCO_TOKEN_H */
