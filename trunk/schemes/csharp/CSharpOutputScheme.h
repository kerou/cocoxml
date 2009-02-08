/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CSHARPOUTPUTSCHEME_H
#define  COCO_CSHARPOUTPUTSCHEME_H

#ifndef  COCO_CSHARPBASEOUTPUTSCHEME_H
#include "CSharpBaseOutputScheme.h"
#endif

#ifndef  COCO_CcsParser_H
#include "c/Parser.h"
#endif

EXTC_BEGIN

typedef struct {
    CcCSharpBaseOutputScheme_t base;
    CcsParser_t * parser;
} CcCSharpOutputScheme_t;

CcCSharpOutputScheme_t *
CcCSharpOutputScheme(CcsParser_t * parser, CcArguments_t * arguments);

EXTC_END

#endif /* COCO_CSHARPOUTPUTSCHEME_H */
