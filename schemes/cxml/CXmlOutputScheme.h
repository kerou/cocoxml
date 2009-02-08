/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CXMLOUTPUTSCHEME_H
#define  COCO_CXMLOUTPUTSCHEME_H

#ifndef COCO_CBASEOUTPUTSCHEME_H
#include "CBaseOutputScheme.h"
#endif

#ifndef  COCO_CcsXmlParser_H
#include "cxml/Parser.h"
#endif

EXTC_BEGIN

typedef struct {
    CcCBaseOutputScheme_t base;
    CcsXmlParser_t * parser;
} CcCXmlOutputScheme_t;

CcCXmlOutputScheme_t *
CcCXmlOutputScheme(CcsXmlParser_t * parser, CcArguments_t * arguments);

EXTC_END

#endif /* COCO_CXMLOUTPUTSCHEME_H */
