/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CSHARPXMLOUTPUTSCHEME_H
#define  COCO_CSHARPXMLOUTPUTSCHEME_H

#ifndef  COCO_CSHARPBASEOUTPUTSCHEME_H
#include "CSharpBaseOutputScheme.h"
#endif

#ifndef  COCO_CcsXmlParser_H
#include "cxml/Parser.h"
#endif

EXTC_BEGIN

typedef struct {
    CcCSharpBaseOutputScheme_t base;
    CcsXmlParser_t * parser;
} CcCSharpXmlOutputScheme_t;

CcCSharpXmlOutputScheme_t *
CcCSharpXmlOutputScheme(CcsXmlParser_t * parser, CcArguments_t * arguments);

EXTC_END

#endif /* COCO_CSHARPOUTPUTSCHEME_H */
