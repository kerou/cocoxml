/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_EBNF_H
#define  COCO_EBNF_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

EXTC_BEGIN

typedef struct CcNodeType_s CcNodeType_t;
typedef struct CcGraph_s CcGraph_t;
extern const CcObjectType_t * node_alt;
extern const CcObjectType_t * node_iter;
extern const CcObjectType_t * node_opt;
extern const CcObjectType_t * node_eps;

struct CcNodeType_s {
    CcObjectType_t base;
    CcsBool_t (* deletable)(CcNode_t * self);
};

struct CcNode_s {
    CcObject_t   base;
    CcNode_t   * next;
    CcNode_t   * down;
    CcNode_t   * sub;
    CcsBool_t    up;
    int          line;
    CcState_t  * state; /* Used by Lexical only. */
};

CcNode_t * CcNode(const CcNodeType_t * type, int line);
void CcNode_Destruct(CcObject_t * self);
CcsBool_t CcNode_Deletable(CcNode_t * self);
CcsBool_t CcNode_NoDeletable(CcNode_t * self);

#define CcNodeAlt(line)   CcNode((const CcNodeType_t *)node_alt, (line))
#define CcNodeIter(line)  CcNode((const CcNodeType_t *)node_iter, (line))
#define CcNodeOpt(line)   CcNode((const CcNodeType_t *)node_opt, (line))
#define CcNodeEps(line)   CcNode((const CcNodeType_t *)node_eps, (line))

/* Deletablity checks */
CcsBool_t CcNode_DelGraph(CcNode_t * self);
CcsBool_t CcNode_DelSubGraph(CcNode_t * self);
CcsBool_t CcNode_DelNode(CcNode_t * self);

struct CcGraph_s {
    CcNode_t * head;
    CcNode_t * r;
};

CcGraph_t * CcGraph(void);
CcGraph_t * CcGraphP(CcNode_t * p);
void CcGraph_Append(CcGraph_t * self, CcNode_t * p);
void CcGraph_Finish(CcGraph_t * self);
void CcGraph_Destruct(CcGraph_t * self);

typedef struct {
    CcArrayList_t nodes;
}  CcEBNF_t;

CcEBNF_t * CcEBNF(CcEBNF_t * self);
void CcEBNF_Destruct(CcEBNF_t * self);

void CcEBNF_Clear(CcEBNF_t * self);

CcNode_t * CcEBNF_NewNode(CcEBNF_t * self, CcNode_t * node);

CcNode_t * CcEBNF_MakeFirstAlt(CcEBNF_t * self, CcGraph_t * g);
CcNode_t *
CcEBNF_MakeAlternative(CcEBNF_t * self, CcGraph_t * g1, CcGraph_t * g2);
void CcEBNF_MakeSequence(CcEBNF_t * self, CcGraph_t * g1, CcGraph_t * g2);
CcNode_t * CcEBNF_MakeIteration(CcEBNF_t * self, CcGraph_t * g);
CcNode_t * CcEBNF_MakeOption(CcEBNF_t * self, CcGraph_t * g);

#ifdef  NDEBUG
#define CcEBNF_DumpNodes(self)  ((void)0)
#else
void CcEBNF_DumpNodes(const CcEBNF_t * self);
#endif

EXTC_END

#endif /* COCO_EBNF_H */
