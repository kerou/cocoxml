/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#include  "Position.h"

CcsPosition_t *
CcsPosition(int beg, int len, int col, const char * text)
{
    CcsPosition_t * self;
    if (!(self = CcsMalloc(sizeof(CcsPosition_t) + len + 1))) return NULL;
    self->beg = beg;
    self->len = len;
    self->col = col;
    self->text = (char *)(self + 1);
    memcpy(self->text, text, len);
    self->text[len] = 0;
    return self;
}

void
CcsPosition_Destruct(CcsPosition_t * self)
{
    CcsFree(self);
}

CcsPosition_t *
CcsPosition_Clone(const CcsPosition_t * pos)
{
    CcsPosition_t * self;
    if (pos == NULL) return NULL;
    if (!(self = CcsMalloc(sizeof(CcsPosition_t) + pos->len + 1))) return NULL;
    self->beg = pos->beg;
    self->len = pos->len;
    self->col = pos->col;
    self->text = (char *)(self + 1);
    memcpy(self->text, pos->text, self->len + 1);
    return self;
}

CcsPosition_t *
CcsPosition_Link(CcsPosition_t * pos0, CcsPosition_t * pos1)
{
    CcsPosition_t * self;
    if (pos1 == NULL) return pos0;
    if (pos0 == NULL) return pos1;
    if (!(self = CcsMalloc(sizeof(CcsPosition_t) + pos0->len + pos1->len + 1))) {
	CcsFree(pos0); CcsFree(pos1); return NULL;
    }
    self->beg = pos0->beg;
    self->len = pos0->len + pos1->len;
    self->col = pos0->col;
    self->text = (char *)(self + 1);
    memcpy(self->text, pos0->text, pos0->len);
    memcpy(self->text + pos0->len, pos1->text, pos1->len + 1);
    CcsFree(pos0); CcsFree(pos1);
    return self;
}
