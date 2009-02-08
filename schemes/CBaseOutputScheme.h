/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CBASEOUTPUTSCHEME_H
#define  COCO_CBASEOUTPUTSCHEME_H

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

    CcsBool_t useStartOf;
    CcsBool_t useGetSS;
    CcsBool_t useExpectSS;
    CcsBool_t useExpectWeak;
    CcsBool_t useWeakSeparator;
}  CcCBaseOutputScheme_t;

CcCBaseOutputScheme_t *
CcCBaseOutputScheme(const CcOutputSchemeType_t * type, CcGlobals_t * globals,
		    CcArguments_t * arguments);

CcsBool_t
CcCBaseOutputScheme_write(CcOutputScheme_t * self, CcOutput_t * output,
			  const char * func, const char * params);

void CcCBaseOutputScheme_Destruct(CcObject_t * self);

EXTC_END

#endif /* COCO_CBASEOUTPUTSCHEME_H */
