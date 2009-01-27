/*---- license ----*/
/*-------------------------------------------------------------------------
 Coco.ATG -- Attributed Grammar
 Compiler Generator Coco/R,
 Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
 extended by M. Loeberbauer & A. Woess, Univ. of Linz
 with improvements by Pat Terry, Rhodes University.
 ported to C by Charles Wang <charlesw123456@gmail.com>

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2, or (at your option) any 
 later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 As an exception, it is allowed to write an extension of Coco/R that is
 used as a plugin in non-free software.

 If not otherwise stated, any source code generated by Coco/R (other than 
 Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef COCO_CcsScanner_H
#define COCO_CcsScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CcsScanner_MAX_KEYWORD_LEN 11
#define CcsScanner_CASE_SENSITIVE
#define CcsScanner_KEYWORD_USED
/*---- enable ----*/

typedef struct CcsScanner_s CcsScanner_t;
struct CcsScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

CcsScanner_t *
CcsScanner(CcsScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CcsScanner_t *
CcsScanner_ByName(CcsScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CcsScanner_Destruct(CcsScanner_t * self);
CcsToken_t * CcsScanner_GetDummy(CcsScanner_t * self);

CcsToken_t * CcsScanner_Scan(CcsScanner_t * self);
void CcsScanner_TokenIncRef(CcsScanner_t * self, CcsToken_t * token);
void CcsScanner_TokenDecRef(CcsScanner_t * self, CcsToken_t * token);

const char *
CcsScanner_GetString(CcsScanner_t * self, const CcsToken_t * begin, size_t len);
CcsPosition_t *
CcsScanner_GetPosition(CcsScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CcsScanner_GetPositionBetween(CcsScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * CcsScanner_Peek(CcsScanner_t * self);
void CcsScanner_ResetPeek(CcsScanner_t * self);

#ifdef CcsScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CcsScanner_IndentLimit(CcsScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
CcsScanner_Include(CcsScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
CcsScanner_IncludeByName(CcsScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
CcsScanner_InsertExpect(CcsScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_CcsScanner_H */
