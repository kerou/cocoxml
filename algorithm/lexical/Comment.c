/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Comment.h"

CcComment_t *
CcComment(const int * start, const int * stop, CcsBool_t nested)
{
    int * cur0; const int * cur1;
    CcComment_t * self = CcMalloc(sizeof(CcComment_t));
    cur0 = self->start; cur1 = start;
    while (*cur1) *cur0++ = *cur1++;
    *cur0 = 0;
    cur0 = self->stop; cur1 = stop;
    while (*cur1) *cur0++ = *cur1++;
    *cur0 = 0;
    self->nested = nested;
    self->next = NULL;
    return self;
}

void
CcComment_Destruct(CcComment_t * self)
{
    CcFree(self);
}
