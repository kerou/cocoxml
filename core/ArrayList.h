/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_ARRAYLIST_H
#define  COCO_ARRAYLIST_H

#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif

EXTC_BEGIN

struct CcArrayList_s {
    int Count;
    int Capacity;
    CcObject_t ** Objects;
};

CcArrayList_t * CcArrayList(CcArrayList_t * self);
void CcArrayList_Destruct(CcArrayList_t * self);

CcObject_t * CcArrayList_New(CcArrayList_t * self, CcObject_t * object);

CcObject_t * CcArrayList_Get(CcArrayList_t * self, int index);
const CcObject_t * CcArrayList_GetC(const CcArrayList_t * self, int index);

void CcArrayList_Clear(CcArrayList_t * self);

typedef struct {
    int index;
}  CcArrayListIter_t;
CcObject_t * CcArrayList_First(CcArrayList_t * self, CcArrayListIter_t * iter);
CcObject_t * CcArrayList_Next(CcArrayList_t * self, CcArrayListIter_t * iter);
const CcObject_t *
CcArrayList_FirstC(const CcArrayList_t * self, CcArrayListIter_t * iter);
const CcObject_t *
CcArrayList_NextC(const CcArrayList_t * self, CcArrayListIter_t * iter);

CcArrayListIter_t *
CcArrayListIter_Copy(CcArrayListIter_t * self, const CcArrayListIter_t * orig);

/* If return NULL, the object has to be destructed. */
typedef CcObject_t *
(* CcArrayList_FilterFunc_t)(CcObject_t * object, int curidx, int newidx,
			     void * data);
void CcArrayList_Filter(CcArrayList_t * self, CcArrayList_FilterFunc_t func,
			void * data);

EXTC_END

#endif /* COCO_ARRAYLIST_H */
