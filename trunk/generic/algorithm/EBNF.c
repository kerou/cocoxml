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
#include  "EBNF.h"

static const CsNodeType_t NodeAlt = {
    { sizeof(CsNode_t), "alt" }
};
const CsNodeType_t * node_alt = &NodeAlt;

static const CsNodeType_t NodeIter = {
    { sizeof(CsNode_t), "iter" }
};
const CsNodeType_t * node_iter = &NodeIter;

static const CsNodeType_t NodeOpt = {
    { sizeof(CsNode_t), "opt" }
};
const CsNodeType_t * node_opt = &NodeOpt;

CsNode_t *
CsNode_NewWithSub(CsNode_t * self, const CsNodeType_t * type, CsNode_t * sub)
{
    self = (CsNode_t *)CsObject(self ? &self->base : NULL, &type->base);
    self->sub = sub;
    return self;
}

void
CsNode_Destruct(CsNode_t * self)
{
    CsObject_Destruct(&self->base);
}

CsGraph_t *
CsGraph(CsGraph_t * self)
{
    self = AllocObject(self, sizeof(CsGraph_t));
    self->l = self->r = NULL;
    return self;
}

CsGraph_t *
CsGraphP(CsGraph_t * self, CsNode_t * p)
{
    self = AllocObject(self, sizeof(CsGraph_t));
    self->l = self->r = p;
    return self;
}

void
CsGraph_Destruct(CsGraph_t * self)
{
    CocoFree(self);
}

void
CsGraph_MakeFirstAlt(CsGraph_t * self)
{
    self->l = CsNode_NewWithSub(NULL, node_alt, self->l);
    self->l->line = self->l->sub->line;
    self->l->next = self->r;
    self->r = self->l;
}

void
CsGraph_MakeAlternative(CsGraph_t * self, CsGraph_t * g2)
{
    CsNode_t * p;
    g2->l = CsNode_NewWithSub(NULL, node_alt, g2->l); g2->l->line = g2->l->sub->line;
    p = self->l; while (p->down != NULL) p = p->down;
    p->down = g2->l;
    p = self->r; while (p->next != NULL) p = p->next;
    /* Append alternative to self end list. */
    p->next = g2->l;
    /* Append g2 end list to self end list. */
    g2->l->next = g2->r;
}

void
CsGraph_MakeSequence(CsGraph_t * self, CsGraph_t * g2)
{
    CsNode_t * q, * p;
    p = self->r->next; self->r->next = g2->l; /* link head node */
    while (p != NULL) { /* link substructure */
	q = p->next; p->next = g2->l; p->up = TRUE;
	p = q;
    }
    self->r = g2->r;
}

void
CsGraph_MakeIteration(CsGraph_t * self)
{
    CsNode_t * p , * q;
    self->l = CsNode_NewWithSub(NULL, node_iter, self->l);
    p = self->r;
    self->r = self->l;
    while (p != NULL) {
	q = p->next; p->next = self->l; p->up = TRUE;
	p = q;
    }
}

void
CsGraph_MakeOption(CsGraph_t * self)
{
    self->l = CsNode_NewWithSub(NULL, node_opt, self->l);
    self->l->next = self->r;
    self->r = self->l;
}

void
CsGraph_Finish(CsGraph_t * self)
{
    CsNode_t * p, * q;
    p = self->r;
    while (p != NULL) {
	q = p->next; p->next = NULL; p = q;
    }
}