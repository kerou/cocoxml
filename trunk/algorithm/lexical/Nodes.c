/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "lexical/Nodes.h"

static void
CcNodeTrans_Destruct(CcObject_t * self)
{
    CcNodeTrans_t * ccself = (CcNodeTrans_t *)self;
    CcTransition_Destruct(&ccself->trans);
    CcNode_Destruct(self);
}
static const CcNodeType_t NodeTrans = {
    { sizeof(CcNodeTrans_t), "node_trans", CcNodeTrans_Destruct },
    CcNode_NoDeletable
};
const CcObjectType_t * node_trans = (const CcObjectType_t *)&NodeTrans;

CcNode_t *
CcNodeTrans(int line, const CcTransition_t * trans)
{
    CcNodeTrans_t * self = (CcNodeTrans_t *)CcNode(&NodeTrans, line);
    CcTransition_Clone(&self->trans, trans);
    return (CcNode_t *)self;
}
