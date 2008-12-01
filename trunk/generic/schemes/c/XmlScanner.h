/*-------------------------------------------------------------------------
 Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>

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
-------------------------------------------------------------------------*/
#ifndef  COCO_XMLSCANNER_H
#define  COCO_XMLSCANNER_H

#ifndef  COCO_CDEFS_H
#include "c/CDefs.h"
#endif

#include  <expat.h>

EXTC_BEGIN

typedef struct {
    const char * name;
    int kind;
    int kindEnd;
} CcsXmlTag_t;

typedef struct {
    const char * name;
    int kind;
} CcsXmlAttr_t;

typedef struct {
    const char * name;
    int kind;
} CcsXmlPInstruction_t;

typedef struct {
    const char * nsURI;
    CcsBool_t caseSensitive;
    int kinds[XSO_SIZE];
    const CcsXmlTag_t * firstTag;  /* The sorted tag list. */
    size_t numTags;
    const CcsXmlAttr_t * firstAttr; /* The sorted attr list. */
    size_t numAttrs;
    const CcsXmlPInstruction_t * firstPInstruction; /* The sorted PI list. */
    size_t numPInstructions;
} CcsXmlSpec_t;

#define  SZ_SPECSTACK  256

typedef int (* CcsXmlCmpFunc_t)(const void *, const void *);

struct CcsXmlScanner_s {
    CcsXmlGlobals_t * globals;

    int kindUnknownNS;
    const CcsXmlSpec_t * firstspec;
    size_t numspecs;

    XML_Parser parser;
    FILE * fp;

    CcsToken_t * dummy;
    CcsToken_t * tokens;
    CcsToken_t * peek;

    const CcsXmlSpec_t * specStack[SZ_SPECSTACK];
    const CcsXmlSpec_t ** curSpecStack;
};

CcsXmlScanner_t *
CcsXmlScanner(CcsXmlScanner_t * self, CcsXmlGlobals_t * globals,
	      const char * filename, int kindUnknownNS,
	      const CcsXmlSpec_t * firstspec, size_t numspecs);

void CcsXmlScanner_Destruct(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_GetDummy(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_Scan(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_Peek(CcsXmlScanner_t * self);
void CcsXmlScanner_ResetPeek(CcsXmlScanner_t * self);
void CcsXmlScanner_IncRef(CcsXmlScanner_t * self, CcsToken_t * token);
void CcsXmlScanner_DecRef(CcsXmlScanner_t * self, CcsToken_t * token);

EXTC_END

#endif
