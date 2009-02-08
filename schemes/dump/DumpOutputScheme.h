/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_DUMPOUTPUTSCHEME_H
#define  COCO_DUMPOUTPUTSCHEME_H

#ifndef  COCO_OUTPUTSCHEME_H
#include "OutputScheme.h"
#endif

#ifndef  COCO_CcsParser_H
#include "c/Parser.h"
#endif

#ifndef  COCO_CcsXmlParser_H
#include "cxml/Parser.h"
#endif

EXTC_BEGIN

typedef struct {
    CcOutputScheme_t base;
} CcDumpOutputScheme_t;

CcDumpOutputScheme_t *
CcDumpOutputScheme(CcsParser_t * parser, CcsXmlParser_t * xmlparser,
		   CcArguments_t * arguments);

EXTC_END

#endif  /* COCO_DUMPOUTPUTSCHEME_H */
