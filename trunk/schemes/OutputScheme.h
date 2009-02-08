/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_OUTPUTSCHEME_H
#define  COCO_OUTPUTSCHEME_H

#ifndef  COCO_OBJECT_H
#include "Object.h"
#endif

#ifndef  COCO_GLOBALS_H
#include "Globals.h"
#endif

EXTC_BEGIN

typedef struct {
    FILE * outfp;
    int indent;
    char EOL[3];
}  CcOutput_t;

void CcPrintf(CcOutput_t * self, const char * format, ...);
void CcPrintfL(CcOutput_t * self, const char * format, ...);
void CcPrintfI(CcOutput_t * self, const char * format, ...);
void CcPrintfIL(CcOutput_t * self, const char * format, ...);
void CcSource(CcOutput_t * self, const CcsPosition_t * pos);

struct CcOutputSchemeType_s {
    CcObjectType_t base;

    /* Separated by \0, terminated by blank update name .*/
    const char * updates;

    CcsBool_t (* write)(CcOutputScheme_t * self, CcOutput_t * output,
			const char * func, const char * params);
};

struct CcOutputScheme_s {
    CcObject_t base;
    CcGlobals_t * globals;
    CcArguments_t * arguments;
};

CcOutputScheme_t *
CcOutputScheme(const CcOutputSchemeType_t * type, CcGlobals_t * globals,
	       CcArguments_t * arguments);
void CcOutputScheme_Destruct(CcObject_t * self);

CcsBool_t
CcOutputScheme_GenerateOutputs(CcOutputScheme_t * self,
			       const char * schemeName, const char * atgname);

EXTC_END

#endif  /* COCO_OUTPUTSCHEME_H */
