/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_SYNTAX_NODES_H
#define  COCO_SYNTAX_NODES_H

#ifndef  COCO_EBNF_H
#include  "EBNF.h"
#endif

#ifndef  COCO_BITARRAY_H
#include  "BitArray.h"
#endif

typedef struct {
    CcNode_t base;
    const CcSymbol_t * sym;
    CcsPosition_t * pos;
} CcNodeT_t;
extern const CcObjectType_t * node_t;
CcNode_t * CcNodeT(int line, const CcSymbol_t * sym);

typedef struct {
    CcNode_t base;
} CcNodePR_t;
extern const CcObjectType_t * node_pr;
#define CcNodePR(line)  CcNode((const CcNodeType_t *)node_pr, line)

typedef struct {
    CcNode_t base;
    const CcSymbol_t * sym;
    CcsPosition_t * pos;
} CcNodeNT_t;
extern const CcObjectType_t * node_nt;
CcNode_t * CcNodeNT(int line, const CcSymbol_t * sym);

typedef struct {
    CcNode_t base;
    const CcSymbol_t * sym;
    CcsPosition_t * pos;
} CcNodeWT_t;
extern const CcObjectType_t * node_wt;
CcNode_t * CcNodeWT(int line, const CcSymbol_t * sym);

typedef struct {
    CcNode_t base;
    CcBitArray_t * set;
    CcBitArray_t setSpace;
} CcNodeANY_t;
extern const CcObjectType_t * node_any;
#define CcNodeAny(line)  CcNode((const CcNodeType_t *)node_any, line)

typedef struct {
    CcNode_t base;
    CcBitArray_t * set;
    CcBitArray_t setSpace;
} CcNodeSYNC_t;
extern const CcObjectType_t * node_sync;
#define CcNodeSync(line)  CcNode((const CcNodeType_t *)node_sync, line)

typedef struct {
    CcNode_t base;
    CcsPosition_t * pos;
} CcNodeSEM_t;
extern const CcObjectType_t * node_sem;
#define CcNodeSem(line)  CcNode((const CcNodeType_t *)node_sem, line)

typedef struct {
    CcNode_t base;
    CcsPosition_t * pos;
} CcNodeRSLV_t;
extern const CcObjectType_t * node_rslv;
#define CcNodeRslv(line)  CcNode((const CcNodeType_t *)node_rslv, line)
CcNode_t * CcNodeRslvP(int line, CcsPosition_t * pos);

void CcNode_SetPosition(CcNode_t * self, CcsPosition_t * pos);

#endif  /* COCO_SYNTAX_NODES_H */
