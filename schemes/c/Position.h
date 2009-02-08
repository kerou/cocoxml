/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_POSITION_H
#define  COCO_POSITION_H

#ifndef  COCO_CDEFS_H
#include "CDefs.h"
#endif

EXTC_BEGIN

struct CcsPosition_s {
    int beg;
    int len;
    int col;
    char * text;
};

CcsPosition_t * CcsPosition(int beg, int len, int col, const char * text);
void CcsPosition_Destruct(CcsPosition_t * self);

CcsPosition_t * CcsPosition_Clone(const CcsPosition_t * pos);

CcsPosition_t * CcsPosition_Link(CcsPosition_t * pos0, CcsPosition_t * pos1);

const char *
CcsPosition_Dump(CcsPosition_t * self, char * buf, size_t szbuf);

EXTC_END

#endif /* COCO_POSITION_H */
