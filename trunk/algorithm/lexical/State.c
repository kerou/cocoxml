/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "lexical/State.h"
#include  "lexical/Action.h"
#include  "lexical/CharSet.h"

static void
CcState_Destruct(CcObject_t * self)
{
    CcAction_t * cur, * next;
    CcState_t * ccself = (CcState_t *)self;

    for (cur = ccself->firstAction; cur; cur = next) {
	next = cur->next;
	CcAction_Destruct(cur);
    }
    CcObject_Destruct(self);
}

static CcObjectType_t StateType = {
    sizeof(CcState_t), "State", CcState_Destruct
};

CcState_t *
CcState(void)
{
    return (CcState_t *)CcObject(&StateType);
}

void
CcState_AddAction(CcState_t * self, CcAction_t * act)
{
    int actSize = CcAction_ShiftSize(act);
    CcAction_t * lasta = NULL, * a = self->firstAction;

    /* Collect bigger classes at the beginning gives better performance. */
    while (a != NULL && actSize <= CcAction_ShiftSize(a)) {
	lasta = a; a = a->next;
    }
    act->next = a;
    if (a == self->firstAction) self->firstAction = act;
    else  lasta->next = act;
}

void
CcState_DetachAction(CcState_t * self, CcAction_t * act)
{
    CcAction_t * lasta = NULL, * a = self->firstAction;
    while (a != NULL && a != act) { lasta = a; a = a->next; }
    if (a != NULL) {
	if (a == self->firstAction) self->firstAction = a->next;
	else lasta->next = a->next;
    }
}

void
CcState_MeltWith(CcState_t * self, const CcState_t * s)
{
    const CcAction_t * action;
    for (action = s->firstAction; action != NULL; action = action->next)
	CcState_AddAction(self, CcAction_Clone(action));
}

/* CcAction_t * b might be destructed */
static void
CcState_SplitActions(CcState_t * self, CcAction_t * a, CcAction_t * b)
{
    CcAction_t * c;
    CcCharSet_t * seta, * setb, * setc;
    CcTransition_t trans;

    seta = CcAction_GetShift(a);
    setb = CcAction_GetShift(b);
    if (CcCharSet_Equals(seta, setb)) {
	CcAction_AddTargets(a, b);
	CcState_DetachAction(self, b);
	CcAction_Destruct(b);
    } else if (CcCharSet_Includes(seta, setb)) {
	CcCharSet_Subtract(seta, setb);
	CcAction_AddTargets(b, a);
	CcAction_SetShift(a, seta);
    } else if (CcCharSet_Includes(setb, seta)) {
	CcCharSet_Subtract(setb, seta);
	CcAction_AddTargets(a, b);
	CcAction_SetShift(b, setb);
    } else {
	setc = CcCharSet_Clone(seta); CcCharSet_And(setc, setb);
	if (!CcCharSet_IsEmpty(setc)) {
	    CcCharSet_Subtract(seta, setc);
	    CcCharSet_Subtract(setb, setc);
	    CcAction_SetShift(a, seta);
	    CcAction_SetShift(b, setb);

	    CcTransition_Clone(&trans, &a->trans);
	    CcTransition_SetCharSet(&trans, setc);
	    CcTransition_SetCode(&trans, trans_normal);
	    c = CcAction(&trans);
	    CcTransition_Destruct(&trans);
	    CcAction_AddTargets(c, a);
	    CcAction_AddTargets(c, b);
	    CcState_AddAction(self, c);
	}
	CcCharSet_Destruct(setc);
    }
    CcCharSet_Destruct(seta);
    CcCharSet_Destruct(setb);
}

void
CcState_MakeUnique(CcState_t * self)
{
    CcAction_t * a, * b;

 restart:
    for (a = self->firstAction; a != NULL; a = a->next)
	for (b = a->next; b != NULL; b = b->next)
	    if (CcAction_Overlap(a, b)) {
		CcState_SplitActions(self, a, b);
		goto restart;
	    }
}

CcAction_t *
CcState_FindAction(CcState_t * self, int chr)
{
    CcAction_t * action;

    for (action = self->firstAction; action != NULL; action = action->next)
	if (CcTransition_Check(&action->trans, chr)) return action;
    return NULL;
}
