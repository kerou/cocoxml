/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "lexical/CharClass.h"
#include  "lexical/CharSet.h"

static void
CcCharClass_Destruct(CcObject_t * self)
{
    CcCharClass_t * ccself = (CcCharClass_t *)self;
    CcCharSet_Destruct(ccself->set);
    CcFree(ccself->name);
    CcObject_Destruct(self);
}

static const CcObjectType_t CharClass = {
    sizeof(CcCharClass_t), "char_class", CcCharClass_Destruct
};
const CcObjectType_t * char_class = &CharClass;

CcCharClass_t *
CcCharClass(const char * name, CcCharSet_t * set)
{
    CcCharClass_t * self = (CcCharClass_t *)CcObject(&CharClass);
    self->name = CcStrdup(name);
    self->set = set;
    return self;
}
