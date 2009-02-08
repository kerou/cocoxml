/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_STATE_H
#define  COCO_LEXICAL_STATE_H

#ifndef  COCO_OBJECT_H
#include "Object.h"
#endif

EXTC_BEGIN

struct CcState_s {
    CcObject_t   base;
    CcAction_t * firstAction;
    CcSymbol_t * endOf;
    int          ctx;
};
CcState_t * CcState(void);

void CcState_AddAction(CcState_t * self, CcAction_t * act);
void CcState_DetachAction(CcState_t * self, CcAction_t * act);
void CcState_MeltWith(CcState_t * self, const CcState_t * s);

void CcState_MakeUnique(CcState_t * self);
CcAction_t * CcState_FindAction(CcState_t * self, int ch);

EXTC_END

#endif /* COCO_LEXICAL_STATE_H */
