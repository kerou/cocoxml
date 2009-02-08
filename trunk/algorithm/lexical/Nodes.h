/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_NODES_H
#define  COCO_LEXICAL_NODES_H

#ifndef  COCO_EBNF_H
#include  "EBNF.h"
#endif

#ifndef  COCO_LEXICAL_TRANSITION_H
#include  "Transition.h"
#endif

EXTC_BEGIN

typedef struct {
    CcNode_t       base;
    CcTransition_t trans;
} CcNodeTrans_t;
extern const CcObjectType_t * node_trans;

CcNode_t * CcNodeTrans(int line, const CcTransition_t * trans);

EXTC_END

#endif  /* COCO_LEXICAL_NODES_H */
