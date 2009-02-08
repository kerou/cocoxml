/*---- license ----*/
/*-------------------------------------------------------------------------
  c-expr.atg -- atg for c expression input
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_CExprParser_H
#define  COCO_CExprParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_CExprScanner_H
#include "Scanner.h"
#endif

/*---- hIncludes ----*/
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
#define CExprParser_USE_StartOf
/*---- enable ----*/

typedef struct CExprParser_s CExprParser_t;
struct CExprParser_s {
    CcsErrorPool_t    errpool;
    CExprScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    int value;
    /*---- enable ----*/
};

CExprParser_t * CExprParser(CExprParser_t * self, FILE * infp, FILE * errfp);
CExprParser_t *
CExprParser_ByName(CExprParser_t * self, const char * infn, FILE * errfp);
void CExprParser_Destruct(CExprParser_t * self);
void CExprParser_Parse(CExprParser_t * self);

void CExprParser_SemErr(CExprParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void CExprParser_SemErrT(CExprParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_PARSER_H */
