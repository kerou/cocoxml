/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "ArrayList.h"
#include  "lexical/Transition.h"
#include  "lexical/CharSet.h"
#include  "lexical/CharClass.h"

CcTransition_t *
CcTransition(CcTransition_t * self, int chr, CcTransitionCode_t code,
	     CcArrayList_t * classes)
{
    self->classes = classes;
    self->code = code;
    self->single = TRUE;
    self->u.chr = chr;
    return self;
}

CcTransition_t *
CcTransition_FromCharSet(CcTransition_t * self, const CcCharSet_t * s,
			 CcTransitionCode_t code, CcArrayList_t * classes)
{

    self->classes = classes;
    self->code = code;
    CcTransition_SetCharSet(self, s);
    return self;
}

CcTransition_t *
CcTransition_Clone(CcTransition_t * self, const CcTransition_t * t)
{
    self->classes = t->classes;
    self->code = t->code;
    self->single = t->single;
    if (self->single) self->u.chr = t->u.chr;
    else self->u.set = t->u.set;
    return self;
}

int
CcTransition_Size(const CcTransition_t * self)
{
    if (self->single) return 1;
    return CcCharSet_Elements(self->u.set);
}

int
CcTransition_First(const CcTransition_t * self)
{
    if (self->single) return self->u.chr;
    return CcCharSet_First(self->u.set);
}

CcCharSet_t *
CcTransition_GetCharSet(const CcTransition_t * self)
{
    CcCharSet_t * s;
    if (self->single) {
	s = CcCharSet();
	CcCharSet_Set(s, self->u.chr);
    } else {
	s = CcCharSet_Clone(self->u.set);
    }
    return s;
}

void
CcTransition_SetCharSet(CcTransition_t * self, const CcCharSet_t * s)
{
    CcCharClass_t * c; CcArrayListIter_t iter;
    if (CcCharSet_Elements(s) == 1) {
	self->single = TRUE;
	self->u.chr = CcCharSet_First(s);
	return;
    }
    self->single = FALSE;
    for (c = (CcCharClass_t *)CcArrayList_First(self->classes, &iter);
	 c; c = (CcCharClass_t *)CcArrayList_Next(self->classes, &iter))
	if (CcCharSet_Equals(s, c->set)) break;
    if (!c)
	c = (CcCharClass_t *)
	    CcArrayList_New(self->classes, (CcObject_t *)
			    CcCharClass("#", CcCharSet_Clone(s)));
    self->u.set = c->set;
}

void
CcTransition_SetCode(CcTransition_t * self, CcTransitionCode_t code)
{
    self->code = code;
}

CcsBool_t
CcTransition_Check(const CcTransition_t * self, int chr)
{
    if (self->single) return chr == self->u.chr;
    return CcCharSet_Get(self->u.set, chr);
}

CcsBool_t
CcTransition_Overlap(const CcTransition_t * a,const CcTransition_t * b)
{
    if (a->single) {
	if (b->single) return a->u.chr == b->u.chr;
	return CcCharSet_Get(b->u.set, a->u.chr);
    } else {
	if (b->single) return CcCharSet_Get(a->u.set, b->u.chr);
	return CcCharSet_Intersects(a->u.set, b->u.set);
    }
}

void
CcTransition_Destruct(CcTransition_t * self)
{
    /* NOTHING IS OK */
}
