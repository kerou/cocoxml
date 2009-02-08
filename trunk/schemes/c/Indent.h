/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef COCO_INDENT_H
#define COCO_INDENT_H

#ifndef  COCO_CDEFS_H
#include "c/CDefs.h"
#endif

EXTC_BEGIN

typedef struct {
    int  kIndentIn;
    int  kIndentOut;
    int  kIndentErr;
}   CcsIndentInfo_t;

typedef struct {
    const CcsIndentInfo_t * info;
    CcsBool_t               lineStart;
    int                   * indent;
    int                   * indentUsed;
    int                   * indentLast;
    int                     indentLimit;
}   CcsIndent_t;

CcsBool_t CcsIndent_Init(CcsIndent_t * self, const CcsIndentInfo_t * info);
void CcsIndent_Destruct(CcsIndent_t * self);

void CcsIndent_SetLimit(CcsIndent_t * self, const CcsToken_t * indentIn);

CcsToken_t *
CcsIndent_Generator(CcsIndent_t * self, CcsScanInput_t * input);

EXTC_END

#endif  /* COCO_INDENT_H */
