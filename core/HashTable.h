/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_HASHTABLE_H
#define  COCO_HASHTABLE_H

#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif

EXTC_BEGIN

typedef struct CcHTEntry_s CcHTEntry_t;

typedef struct {
    CcHTEntry_t ** first, ** cur, ** last;
} CcHTIterator_t;

struct CcHashTable_s {
    CcHTEntry_t ** first, ** last;
};

CcHashTable_t * CcHashTable(CcHashTable_t * self, size_t size);
void CcHashTable_Destruct(CcHashTable_t * self);

int CcHashTable_Num(const CcHashTable_t * self);

CcsBool_t
CcHashTable_Set(CcHashTable_t * self, const char * key, CcObject_t * value);
CcObject_t * CcHashTable_Get(CcHashTable_t * self, const char * key);

CcHTIterator_t *
CcHashTable_GetIterator(const CcHashTable_t * self, CcHTIterator_t * iter);

CcsBool_t CcHTIterator_Forward(CcHTIterator_t * self);
const char * CcHTIterator_Key(CcHTIterator_t * iter);
CcObject_t * CcHTIterator_Value(CcHTIterator_t * iter);

EXTC_END

#endif  /* COCO_HASHTABLE_H */
