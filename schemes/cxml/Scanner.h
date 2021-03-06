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
#ifndef COCO_CcsXmlScanner_H
#define COCO_CcsXmlScanner_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

/*---- defines ----*/
#define CcsXmlScanner_MAX_KEYWORD_LEN 23
#define CcsXmlScanner_CASE_SENSITIVE
#define CcsXmlScanner_KEYWORD_USED
/*---- enable ----*/

typedef struct CcsXmlScanner_s CcsXmlScanner_t;
struct CcsXmlScanner_s {
    CcsErrorPool_t * errpool;
    CcsToken_t     * dummyToken;
    CcsScanInput_t * cur;
};

CcsXmlScanner_t *
CcsXmlScanner(CcsXmlScanner_t * self, CcsErrorPool_t * errpool, FILE * fp);
CcsXmlScanner_t *
CcsXmlScanner_ByName(CcsXmlScanner_t * self, CcsErrorPool_t * errpool,
		  const char * infn);
void CcsXmlScanner_Destruct(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_GetDummy(CcsXmlScanner_t * self);

CcsToken_t * CcsXmlScanner_Scan(CcsXmlScanner_t * self);
void CcsXmlScanner_TokenIncRef(CcsXmlScanner_t * self, CcsToken_t * token);
void CcsXmlScanner_TokenDecRef(CcsXmlScanner_t * self, CcsToken_t * token);

long
CcsXmlScanner_StringTo(CcsXmlScanner_t * self, size_t * len, const char * needle);
const char * CcsXmlScanner_GetString(CcsXmlScanner_t * self, long start, size_t len);
void CcsXmlScanner_Consume(CcsXmlScanner_t * self, long start, size_t len);

CcsPosition_t *
CcsXmlScanner_GetPosition(CcsXmlScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end);
CcsPosition_t *
CcsXmlScanner_GetPositionBetween(CcsXmlScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end);

CcsToken_t * CcsXmlScanner_Peek(CcsXmlScanner_t * self);
void CcsXmlScanner_ResetPeek(CcsXmlScanner_t * self);

#ifdef CcsXmlScanner_INDENTATION
/* If the col >= indentIn->col, not any IndentIn/IndentOut/IndentErr is generated.
 * Useful when we need to collect ANY text by indentation. */
void CcsXmlScanner_IndentLimit(CcsXmlScanner_t * self, const CcsToken_t * indentIn);
#endif

CcsBool_t
CcsXmlScanner_Include(CcsXmlScanner_t * self, FILE * fp, CcsToken_t ** token);
CcsBool_t
CcsXmlScanner_IncludeByName(CcsXmlScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token);
CcsBool_t
CcsXmlScanner_InsertExpect(CcsXmlScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token);

EXTC_END

#endif  /* COCO_CcsXmlScanner_H */
