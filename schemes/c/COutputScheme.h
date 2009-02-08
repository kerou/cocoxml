/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_COUTPUTSCHEME_H
#define  COCO_COUTPUTSCHEME_H

#ifndef  COCO_CBASEOUTPUTSCHEME_H
#include "CBaseOutputScheme.h"
#endif

#ifndef  COCO_CcsParser_H
#include "c/Parser.h"
#endif

EXTC_BEGIN

typedef struct {
    CcCBaseOutputScheme_t base;
    CcsParser_t * parser;
} CcCOutputScheme_t;

CcCOutputScheme_t *
CcCOutputScheme(CcsParser_t * parser, CcArguments_t * arguments);

EXTC_END

#endif /* COCO_COUTPUTSCHEME_H */
