/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Object.h"

CcObject_t *
CcObject(const CcObjectType_t * type)
{
    CcObject_t * self = CcMalloc(type->size);
    memset(self, 0, type->size);
    self->type = type;
    return self;
}

void
CcObject_Destruct(CcObject_t * self)
{
    CcFree(self);
}

void
CcObject_VDestruct(CcObject_t * self)
{
    self->type->destruct(self);
}
