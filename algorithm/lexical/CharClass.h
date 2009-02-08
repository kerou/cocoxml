/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_CHARCLASS_H
#define  COCO_LEXICAL_CHARCLASS_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

EXTC_BEGIN

struct CcCharClass_s {
    CcObject_t base;
    char * name;
    CcCharSet_t * set;
};

CcCharClass_t * CcCharClass(const char * name, CcCharSet_t * set);

EXTC_END

#endif  /* COCO_LEXICAL_CHARCLASS_H */
