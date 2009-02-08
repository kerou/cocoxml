/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "lexical/Action.h"
#include  "lexical/CharClass.h"
#include  "lexical/CharSet.h"
#include  "lexical/Target.h"
#include  "lexical/State.h"

CcAction_t *
CcAction(const CcTransition_t * trans)
{
    CcAction_t * self = CcMalloc(sizeof(CcAction_t));
    self->next = NULL;
    self->target = NULL;
    CcTransition_Clone(&self->trans, trans);
    return self;
}

CcAction_t *
CcAction_Clone(const CcAction_t * action)
{
    CcAction_t * self = CcMalloc(sizeof(CcAction_t));
    self->next = NULL;
    self->target = NULL;
    CcTransition_Clone(&self->trans, &action->trans);
    CcAction_AddTargets(self, action);
    return self;
}

void
CcAction_Destruct(CcAction_t * self)
{
    CcTarget_t * cur, * next;

    for (cur = self->target; cur; cur = next) {
	next = cur->next;
	CcTarget_Destruct(cur);
    }
    CcTransition_Destruct(&self->trans);
    CcFree(self);
}

int
CcAction_ShiftSize(CcAction_t * self)
{
    return CcTransition_Size(&self->trans);
}

CcCharSet_t *
CcAction_GetShift(const CcAction_t * self)
{
    return CcTransition_GetCharSet(&self->trans);
}

void
CcAction_SetShift(CcAction_t * self, const CcCharSet_t * s)
{
    CcTransition_SetCharSet(&self->trans, s);
}

static void
CcAction_AddTarget(CcAction_t * self, CcTarget_t * t)
{
    CcTarget_t * last = NULL;
    CcTarget_t * p = self->target;
    while (p != NULL && t->state->base.index >= p->state->base.index) {
	if (t->state == p->state) { CcTarget_Destruct(t); return; }
	last = p; p = p->next;
    }
    t->next = p;
    if (p == self->target)  self->target = t;
    else  last->next = t;
}

void
CcAction_AddTargets(CcAction_t * self, const CcAction_t * action)
{
    CcTarget_t * p;
    for (p = action->target; p != NULL; p = p->next)
	CcAction_AddTarget(self, CcTarget(p->state));
    if (action->trans.code == trans_context) self->trans.code = trans_context;
}

CcsBool_t
CcAction_Overlap(const CcAction_t * a, const CcAction_t * b)
{
    return CcTransition_Overlap(&a->trans, &b->trans);
}
