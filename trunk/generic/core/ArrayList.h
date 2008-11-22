/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the 
  Free Software Foundation; either version 2, or (at your option) any 
  later version.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.

  You should have received a copy of the GNU General Public License along 
  with this program; if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
#ifndef  COCO_ARRAYLIST_H
#define  COCO_ARRAYLIST_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
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
