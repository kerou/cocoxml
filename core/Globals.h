/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_GLOBALS_H
#define  COCO_GLOBALS_H

#ifndef  COCO_SYMBOLTABLE_H
#include  "SymbolTable.h"
#endif

#ifndef  COCO_LEXICAL_H
#include  "Lexical.h"
#endif

#ifndef  COCO_XMLSPECMAP_H
#include  "XmlSpecMap.h"
#endif

#ifndef  COCO_SYNTAX_H
#include  "Syntax.h"
#endif

EXTC_BEGIN

struct CcGlobals_s {
    CcsErrorPool_t  * errpool;
    const char      * templatePrefix;
    CcSymbolTable_t   symtab;
    CcLexical_t     * lexical;
    CcXmlSpecMap_t  * xmlspecmap;
    union {
	CcLexical_t    lexicalSpace;
	CcXmlSpecMap_t xmlspecmapSpace;
    } u;
    CcSyntax_t        syntax;

    CcArrayList_t     sections;
};

CcGlobals_t * CcGlobals(CcGlobals_t * self, CcsErrorPool_t * errpool);
CcGlobals_t * CcGlobalsXml(CcGlobals_t * self, CcsErrorPool_t * errpool);
void CcGlobals_Destruct(CcGlobals_t * self);

CcsBool_t CcGlobals_Finish(CcGlobals_t * self);

void CcGlobals_NewSection(CcGlobals_t * self, const char * secname,
			  CcsPosition_t * pos);
const CcsPosition_t *
CcGlobals_FirstSection(const CcGlobals_t * self, const char * secname,
		       CcArrayListIter_t * iter);
const CcsPosition_t *
CcGlobals_NextSection(const CcGlobals_t * self, const char * secname,
		      CcArrayListIter_t * iter);

EXTC_END

#endif  /* COCO_GLOBALS_H */
