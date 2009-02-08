/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  <stdlib.h>
#include  "Melted.h"
#include  "BitArray.h"
#include  "State.h"

CcMelted_t *
CcMelted(const CcBitArray_t * set, CcState_t * state)
{
    CcMelted_t * self = CcMalloc(sizeof(CcMelted_t));
    CcBitArray_Clone(&self->set, set);
    self->state = state;
    self->next = NULL;
    return self;
}

void
CcMelted_Destruct(CcMelted_t * self)
{
    CcBitArray_Destruct(&self->set);
    CcFree(self);
}
