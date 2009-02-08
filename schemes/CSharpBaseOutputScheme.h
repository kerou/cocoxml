/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CSHARPBASEOUTPUTSCHEME_H
#define  COCO_CSHARPBASEOUTPUTSCHEME_H

#ifndef  COCO_OUTPUTSCHEME_H
#include  "OutputScheme.h"
#endif

#ifndef  COCO_GLOBALS_H
#incldue "Globals.h"
#endif

EXTC_BEGIN

typedef struct {
    CcOutputScheme_t base;
    const char * prefix;
    CcSyntaxSymSet_t symSet;
    const CcSymbol_t * curSy;
}  CcCSharpBaseOutputScheme_t;

CcCSharpBaseOutputScheme_t *
CcCSharpBaseOutputScheme(const CcOutputSchemeType_t * type,
			 CcGlobals_t * globals, CcArguments_t * arguments);

CcsBool_t
CcCSharpBaseOutputScheme_write(CcOutputScheme_t * self, CcOutput_t * output,
			       const char * func, const char * params);

void CcCSharpBaseOutputScheme_Destruct(CcObject_t * self);

EXTC_END

#endif  /* COCO_CSHARPBASEOUTPUTSCHEME_H */
