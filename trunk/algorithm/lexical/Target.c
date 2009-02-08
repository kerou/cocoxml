/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Target.h"

CcTarget_t *
CcTarget(CcState_t * s)
{
    CcTarget_t * self = CcMalloc(sizeof(CcTarget_t));
    self->state = s;
    self->next = NULL;
    return self;
}

void
CcTarget_Destruct(CcTarget_t * self)
{
    CcFree(self);
}

void
CcTarget_ListDestruct(CcTarget_t * head)
{
    CcTarget_t * cur, * next;
    for (cur = head; cur; cur = next) {
	next = cur->next;
	CcTarget_Destruct(cur);
    }
}
