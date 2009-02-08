/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_BITARRAY_H
#define  COCO_BITARRAY_H

#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif

EXTC_BEGIN

struct CcBitArray_s {
    int numbits;
    unsigned char * data;
};

CcBitArray_t * CcBitArray(CcBitArray_t * self, int numbits);
CcBitArray_t * CcBitArray1(CcBitArray_t * self, int numbits);
CcBitArray_t * CcBitArray_Clone(CcBitArray_t * self, const CcBitArray_t * value);
void CcBitArray_Destruct(CcBitArray_t * self);

/* Return -1 for error. */
int CcBitArray_getCount(const CcBitArray_t * self);
int CcBitArray_Elements(const CcBitArray_t * self);
CcsBool_t CcBitArray_Get(const CcBitArray_t * self, int index);
void CcBitArray_Set(CcBitArray_t * self, int index, CcsBool_t value);
void CcBitArray_SetAll(CcBitArray_t * self, CcsBool_t value);
CcsBool_t CcBitArray_Equal(const CcBitArray_t * self1, const CcBitArray_t * self2);
void CcBitArray_Not(CcBitArray_t * self);
int CcBitArray_And(CcBitArray_t * self, const CcBitArray_t * value);
int CcBitArray_Or(CcBitArray_t * self, const CcBitArray_t * value);
int CcBitArray_Xor(CcBitArray_t * self, const CcBitArray_t * value);
CcsBool_t CcBitArray_Intersect(const CcBitArray_t * self1, const CcBitArray_t * self2);
void CcBitArray_Subtract(CcBitArray_t * self, const CcBitArray_t * b);

EXTC_END

#endif  /* COCO_BITARRAY_H */
