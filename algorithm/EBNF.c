/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "EBNF.h"

CcNode_t *
CcNode(const CcNodeType_t * type, int line)
{
    CcNode_t * self = (CcNode_t *)CcObject(&type->base);
    self->line = line;
    return self;
}
void CcNode_Destruct(CcObject_t * self)
{
    CcObject_Destruct(self);
}
CcsBool_t
CcNode_Deletable(CcNode_t * self)
{
    return TRUE;
}
CcsBool_t
CcNode_NoDeletable(CcNode_t * self)
{
    return FALSE;
}

static CcsBool_t
CcNodeAlt_Deletable(CcNode_t * self)
{
    return CcNode_DelSubGraph(self->sub) ||
	(self->down != NULL && CcNode_DelSubGraph(self->down));
}

static const CcNodeType_t NodeAlt = {
    { sizeof(CcNode_t), "node_alt", CcNode_Destruct }, CcNodeAlt_Deletable
};
const CcObjectType_t * node_alt = (const CcObjectType_t *)&NodeAlt;

static const CcNodeType_t NodeIter = {
    { sizeof(CcNode_t), "node_iter", CcNode_Destruct }, CcNode_Deletable
};
const CcObjectType_t * node_iter = (const CcObjectType_t *)&NodeIter;

static const CcNodeType_t NodeOpt = {
    { sizeof(CcNode_t), "node_opt", CcNode_Destruct }, CcNode_Deletable
};
const CcObjectType_t * node_opt = (const CcObjectType_t *)&NodeOpt;

static const CcNodeType_t NodeEps = {
    { sizeof(CcNode_t), "node_eps", CcNode_Destruct }, CcNode_Deletable
};
const CcObjectType_t * node_eps = (const CcObjectType_t *)&NodeEps;

CcsBool_t
CcNode_DelGraph(CcNode_t * self)
{
    return self == NULL ||
	(CcNode_DelNode(self) && CcNode_DelGraph(self->next));
}

CcsBool_t
CcNode_DelSubGraph(CcNode_t * self)
{
    return self == NULL ||
	(CcNode_DelNode(self) && CcNode_DelSubGraph(self->next));
}

CcsBool_t
CcNode_DelNode(CcNode_t * self)
{
    const CcNodeType_t * type = (const CcNodeType_t *)self->base.type;
    return type->deletable(self);
}

CcGraph_t *
CcGraph(void)
{
    CcGraph_t * self = CcMalloc(sizeof(CcGraph_t));
    self->head = self->r = NULL;
    return self;
}

CcGraph_t *
CcGraphP(CcNode_t * p)
{
    CcGraph_t * self = CcMalloc(sizeof(CcGraph_t));
    self->head = self->r = p;
    return self;
}

void
CcGraph_Append(CcGraph_t * self, CcNode_t * p)
{
    if (self->r == NULL) self->head = p;
    else self->r->next = p;
    self->r = p;
}

void
CcGraph_Finish(CcGraph_t * self)
{
    CcNode_t * p, * q;
    p = self->r;
    while (p != NULL) {
	q = p->next; p->next = NULL; p = q;
    }
}

void
CcGraph_Destruct(CcGraph_t * self)
{
    CcFree(self);
}

CcEBNF_t *
CcEBNF(CcEBNF_t * self)
{
    CcArrayList(&self->nodes);
    return self;
}

void
CcEBNF_Destruct(CcEBNF_t * self)
{
    CcArrayList_Destruct(&self->nodes);
}

void
CcEBNF_Clear(CcEBNF_t * self)
{
    CcArrayList_Clear(&self->nodes);
}

CcNode_t *
CcEBNF_NewNode(CcEBNF_t * self, CcNode_t * node)
{
    return (CcNode_t *)CcArrayList_New(&self->nodes, &node->base);
}

static CcNode_t *
CcEBNF_NewNodeWithSub(CcEBNF_t * self, CcNode_t * node, CcNode_t * sub)
{
    node = CcEBNF_NewNode(self, node);
    node->sub = sub;
    node->line = sub->line;
    return node;
}

CcNode_t *
CcEBNF_MakeFirstAlt(CcEBNF_t * self, CcGraph_t * g)
{
    g->head = CcEBNF_NewNodeWithSub(self, CcNodeAlt(0), g->head);
    g->head->next = g->r;
    g->r = g->head;
    return g->head;
}

CcNode_t *
CcEBNF_MakeAlternative(CcEBNF_t * self, CcGraph_t * g1, CcGraph_t * g2)
{
    CcNode_t * p;

    g2->head = CcEBNF_NewNodeWithSub(self, CcNodeAlt(0), g2->head);
    p = g1->head; while (p->down != NULL) p = p->down;
    p->down = g2->head;
    p = g1->r; while (p->next != NULL) p = p->next;
    /* Append alternative to self end list. */
    p->next = g2->head;
    /* Append g2 end list to self end list. */
    g2->head->next = g2->r;
    return g2->head;
}

void
CcEBNF_MakeSequence(CcEBNF_t * self, CcGraph_t * g1, CcGraph_t * g2)
{
    CcNode_t * q, * p;
    p = g1->r->next; g1->r->next = g2->head; /* link head node */
    while (p != NULL) { /* link substructure */
	q = p->next; p->next = g2->head; p->up = TRUE;
	p = q;
    }
    g1->r = g2->r;
}

CcNode_t *
CcEBNF_MakeIteration(CcEBNF_t * self, CcGraph_t * g)
{
    CcNode_t * p , * q;
    g->head = CcEBNF_NewNodeWithSub(self, CcNodeIter(0), g->head);
    p = g->r;
    g->r = g->head;
    while (p != NULL) {
	q = p->next; p->next = g->head; p->up = TRUE;
	p = q;
    }
    return g->head;
}

CcNode_t *
CcEBNF_MakeOption(CcEBNF_t * self, CcGraph_t * g)
{
    g->head = CcEBNF_NewNodeWithSub(self, CcNodeOpt(0), g->head);
    g->head->next = g->r;
    g->r = g->head;
    return g->head;
}

#ifndef NDEBUG
void
CcEBNF_DumpNodes(const CcEBNF_t * self)
{
    CcArrayListIter_t iter;
    const CcNode_t * node;
    for (node = (const CcNode_t *)CcArrayList_FirstC(&self->nodes, &iter);
	 node; node = (const CcNode_t *)CcArrayList_NextC(&self->nodes, &iter)) {
	fprintf(stderr, "Node(%d):%16s %4d %4d %4d %5s\n",
		node->base.index, node->base.type->name,
		node->next ? node->next->base.index : -1,
		node->down ? node->down->base.index : -1,
		node->sub ? node->sub->base.index : -1,
		node->up ? "TRUE" : "FALSE");
    }
}
#endif
