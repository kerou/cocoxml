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

EXTC_BEGIN

struct CcsXmlScanner_s {
    CcsXmlGlobals_t * globals;
};

CcsXmlScanner_t *
CcsXmlScanner(CcsXmlScanner_t * self, CcsXmlGlobals_t * globals,
	      const char * filename);
void CcsXmlScanner_Destruct(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_GetDummy(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_Scan(CcsXmlScanner_t * self);
CcsToken_t * CcsXmlScanner_Peek(CcsXmlScanner_t * self);
void CcsXmlScanner_ResetPeek(CcsXmlScanner_t * self);

typedef struct {
    const char * name;
    int beginKind;
    int endKind;
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
    CcsBool_t options[XSO_SIZE];
    const CcsXmlTag_t * firstTag;  /* The sorted tag list. */
    const CcsXmlTag_t * lastTag;
    const CcsXmlAttr_t * firstAttr; /* The sorted attr list. */
    const CcsXmlAttr_t * lastAttr;
    const CcsXmlPInstruction_t * firstPInstruction; /* The sorted PI list. */
    const CcsXmlPInstruction_t * lastPInstruction;
} CcsXmlSpec_t;

extern const CcsXmlSpec_t firstXmlSpec[];
extern const CcsXmlSpec_t * lastXmlSpec;

EXTC_END

#endif
