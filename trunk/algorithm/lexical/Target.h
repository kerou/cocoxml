/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_TARGET_H
#define  COCO_LEXICAL_TARGET_H

#ifndef  COCO_DEFS_H
#include "Defs.h"
#endif

struct CcTarget_s {
    CcState_t  * state;
    CcTarget_t * next;
};

CcTarget_t * CcTarget(CcState_t * s);
void CcTarget_Destruct(CcTarget_t * self);

void CcTarget_ListDestruct(CcTarget_t * head);

#endif /* COCO_LEXICAL_TARGET_H */
