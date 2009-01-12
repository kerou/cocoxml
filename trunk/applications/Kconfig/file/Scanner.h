/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_CfScanner_H
#define COCO_CfScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_BUFFER_H
#include "c/Buffer.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CfScanner_MAX_KEYWORD_LEN 0
#define CfScanner_CASE_SENSITIVE
/*---- enable ----*/

typedef struct CfScanInput_s CfScanInput_t;
typedef struct CfScanner_s CfScanner_t;
struct CfScanner_s {
    CcsErrorPool_t * errpool;
    int              eofSym;
    int              noSym;
    int              maxT;
    CcsToken_t     * dummyToken;
    CfScanInput_t * cur;
};

CfScanner_t *
CfScanner(CfScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CfScanner_t *
CfScanner_ByName(CfScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CfScanner_Destruct(CfScanner_t * self);
CcsToken_t * CfScanner_GetDummy(CfScanner_t * self);
CcsToken_t * CfScanner_Scan(CfScanner_t * self);
CcsToken_t * CfScanner_Peek(CfScanner_t * self);
void CfScanner_ResetPeek(CfScanner_t * self);
void CfScanner_IncRef(CfScanner_t * self, CcsToken_t * token);
void CfScanner_DecRef(CfScanner_t * self, CcsToken_t * token);
#ifdef CfScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CfScanner_IndentLimit(CfScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsPosition_t *
CfScanner_GetPosition(CfScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CfScanner_GetPositionBetween(CfScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsBool_t CfScanner_Include(CfScanner_t * self, FILE * fp);
CcsBool_t
CfScanner_IncludeByName(CfScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn);

EXTC_END

#endif  /* COCO_CfScanner_H */
