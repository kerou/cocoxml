/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_OBJECT_H
#define  COCO_OBJECT_H

#ifndef  COCO_DEFS_H
#include "Defs.h"
#endif

EXTC_BEGIN

struct CcObjectType_s {
    size_t size;
    const char * name;
    void (* destruct)(CcObject_t * self);
};

struct CcObject_s {
    const CcObjectType_t * type;
    int index;
};

CcObject_t * CcObject(const CcObjectType_t * type);
void CcObject_Destruct(CcObject_t * self);

void CcObject_VDestruct(CcObject_t * self);

EXTC_END

#endif  /* COCO_OBJECT_H */
