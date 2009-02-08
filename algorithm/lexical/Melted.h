/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_MELTED_H
#define  COCO_LEXICAL_MELTED_H

#ifndef  COCO_BITARRAY_H
#include  "BitArray.h"
#endif

EXTC_BEGIN

struct CcMelted_s {
    CcBitArray_t set;
    CcState_t    * state;
    CcMelted_t   * next;
};

CcMelted_t * CcMelted(const CcBitArray_t * set, CcState_t * state);
void CcMelted_Destruct(CcMelted_t * self);

EXTC_END

#endif  /* COCO_LEXICAL_MELTED_H */
