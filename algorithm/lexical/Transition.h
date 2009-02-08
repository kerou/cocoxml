/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_TRANSITION_H
#define  COCO_LEXICAL_TRANSITION_H

#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif

EXTC_BEGIN

typedef enum {
    trans_normal = 0,
    trans_context = 1
} CcTransitionCode_t;

struct CcTransition_s {
    CcArrayList_t * classes;
    CcTransitionCode_t code;
    CcsBool_t single;
    union {
	int chr; /* single is TRUE. */
	const CcCharSet_t * set; /* single is FALSE.
				    Point into CharClass in classes. */
    } u;
};

CcTransition_t *
CcTransition(CcTransition_t * self, int chr, CcTransitionCode_t code,
	     CcArrayList_t * classes);
CcTransition_t *
CcTransition_FromCharSet(CcTransition_t * self, const CcCharSet_t * s,
			 CcTransitionCode_t code, CcArrayList_t * classes);
CcTransition_t *
CcTransition_Clone(CcTransition_t * self, const CcTransition_t * t);

int CcTransition_Size(const CcTransition_t * self);
int CcTransition_First(const CcTransition_t * self);

/* The returned CcCharSet_t must be destructed. */
CcCharSet_t * CcTransition_GetCharSet(const CcTransition_t * self);
void CcTransition_SetCharSet(CcTransition_t * self, const CcCharSet_t * trans);

void CcTransition_SetCode(CcTransition_t * self, CcTransitionCode_t code);

CcsBool_t CcTransition_Check(const CcTransition_t * self, int chr);

CcsBool_t
CcTransition_Overlap(const CcTransition_t * a,const CcTransition_t * b);

void CcTransition_Destruct(CcTransition_t * self);

EXTC_END

#endif  /* COCO_LEXICAL_TRANSITION_H */
