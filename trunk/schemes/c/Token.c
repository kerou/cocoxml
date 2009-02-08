/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#include  "Token.h"

CcsToken_t *
CcsToken(CcsScanInput_t * input, int kind, const char * fname, int pos,
	 int line, int col, const char * val, size_t vallen)
{
    CcsToken_t * self;
    if (!(self = CcsMalloc(sizeof(CcsToken_t) + vallen + 1))) return NULL;
    self->next = NULL;
    self->destructor = NULL;
    self->input = input;
    self->refcnt = 1;
    self->kind = kind;
    self->loc.fname = fname;
    self->pos = pos;
    self->loc.col = col;
    self->loc.line = line;
    self->val = (char *)(self + 1);
    if (vallen > 0) memcpy(self->val, val, vallen);
    self->val[vallen] = 0;
    return self;
}

void
CcsToken_Destruct(CcsToken_t * self)
{
    CcsFree(self);
}
