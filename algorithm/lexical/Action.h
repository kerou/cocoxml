/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_ACTION_H
#define  COCO_LEXICAL_ACTION_H

#ifndef  COCO_LEXICAL_TRANSITION_H
#include "lexical/Transition.h"
#endif

EXTC_BEGIN

struct CcAction_s {
    CcAction_t     * next;
    CcTarget_t     * target;
    CcTransition_t   trans;
};

CcAction_t *
CcAction(const CcTransition_t * trans);

CcAction_t * CcAction_Clone(const CcAction_t * action);

void CcAction_Destruct(CcAction_t * self);

int CcAction_ShiftSize(CcAction_t * self);

/* The returned CcCharSet_t must be destructed */
CcCharSet_t * CcAction_GetShift(const CcAction_t * self);

void CcAction_SetShift(CcAction_t * self, const CcCharSet_t * s);

void CcAction_AddTargets(CcAction_t * self, const CcAction_t * action);

CcsBool_t CcAction_Overlap(const CcAction_t * a, const CcAction_t * b);

EXTC_END

#endif /* COCO_LEXICAL_ACTION_H */
