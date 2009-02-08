/*---- license ----*/
/*-------------------------------------------------------------------------
  c-expr.atg -- atg for c expression input
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_CExprScanner_H
#define COCO_CExprScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CExprScanner_MAX_KEYWORD_LEN 0
#define CExprScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct CExprScanner_s CExprScanner_t;
struct CExprScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

CExprScanner_t *
CExprScanner(CExprScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CExprScanner_t *
CExprScanner_ByName(CExprScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CExprScanner_Destruct(CExprScanner_t * self);

void CExprScanner_Warning(CExprScanner_t * self, const char * format, ...);
void CExprScanner_Error(CExprScanner_t * self, const char * format, ...);
void CExprScanner_Fatal(CExprScanner_t * self, const char * format, ...);

CcsToken_t * CExprScanner_GetDummy(CExprScanner_t * self);

CcsToken_t * CExprScanner_Scan(CExprScanner_t * self);
void CExprScanner_TokenIncRef(CExprScanner_t * self, CcsToken_t * token);
void CExprScanner_TokenDecRef(CExprScanner_t * self, CcsToken_t * token);

long
CExprScanner_StringTo(CExprScanner_t * self, size_t * len, const char * needle);
const char * CExprScanner_GetString(CExprScanner_t * self, long start, size_t len);
void CExprScanner_Consume(CExprScanner_t * self, long start, size_t len);

CcsPosition_t *
CExprScanner_GetPosition(CExprScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CExprScanner_GetPositionBetween(CExprScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * CExprScanner_Peek(CExprScanner_t * self);
void CExprScanner_ResetPeek(CExprScanner_t * self);

#ifdef CExprScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CExprScanner_IndentLimit(CExprScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
CExprScanner_Include(CExprScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
CExprScanner_IncludeByName(CExprScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
CExprScanner_InsertExpect(CExprScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_CExprScanner_H */
