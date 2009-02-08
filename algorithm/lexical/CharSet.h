/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_CHARSET_H
#define  COCO_LEXICAL_CHARSET_H

#ifndef  COCO_DEFS_H
#include "Defs.h"
#endif

EXTC_BEGIN

typedef struct CcRange_s CcRange_t;
struct CcRange_s {
    int from, to;
    CcRange_t * next;
};
struct CcCharSet_s {
    CcRange_t * head;
};

CcCharSet_t * CcCharSet(void);
void CcCharSet_Destruct(CcCharSet_t * self);
CcsBool_t CcCharSet_Get(const CcCharSet_t * self, int i);
void CcCharSet_Set(CcCharSet_t * self, int i);
CcCharSet_t * CcCharSet_Clone(const CcCharSet_t * s);
CcsBool_t CcCharSet_IsEmpty(const CcCharSet_t * self);
CcsBool_t CcCharSet_Equals(const CcCharSet_t * self, const CcCharSet_t * s);
int CcCharSet_Elements(const CcCharSet_t * self);
int CcCharSet_First(const CcCharSet_t * self);
int CcCharSet_NumRange(const CcCharSet_t * self);
void CcCharSet_Or(CcCharSet_t * self, const CcCharSet_t * s);
void CcCharSet_And(CcCharSet_t * self, const CcCharSet_t * s);
void CcCharSet_Subtract(CcCharSet_t * self, const CcCharSet_t * s);
CcsBool_t CcCharSet_Includes(const CcCharSet_t * self, const CcCharSet_t * s);
CcsBool_t CcCharSet_Intersects(const CcCharSet_t * self, const CcCharSet_t * s);
void CcCharSet_Clear(CcCharSet_t * self);
void CcCharSet_Fill(CcCharSet_t * self, int maxchar);
/*
void CcCharSet_Dump(const CcCharSet_t * self, DumpBuffer_t * buf);
void CcCharSet_DumpInt(const CcCharSet_t * self, DumpBuffer_t * buf);
*/
EXTC_END

#endif  /* COCO_LEXICAL_CHARSET_H */
