/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "XmlSpecMap.h"
#include  "XmlSpec.h"
#include  "Globals.h"
#include  "c/ErrorPool.h"

#define  SZ_XMLSPECMAP  127

CcXmlSpecMap_t *
CcXmlSpecMap(CcXmlSpecMap_t * self, CcGlobals_t * globals)
{
    self->globals = globals;
    self->kindUnknownNS = -1;
    CcArrayList(&self->storage);
    CcHashTable(&self->map, SZ_XMLSPECMAP);
    return self;
}

void
CcXmlSpecMap_Destruct(CcXmlSpecMap_t * self)
{
    CcHashTable_Destruct(&self->map);
    CcArrayList_Destruct(&self->storage);
}

CcsBool_t
CcXmlSpecMap_Add(CcXmlSpecMap_t * self, const char * nsURI, CcXmlSpec_t * xmlspec)
{
    if (CcHashTable_Get(&self->map, nsURI)) return FALSE;
    xmlspec = (CcXmlSpec_t *)
	CcArrayList_New(&self->storage, (CcObject_t *)xmlspec);
    CcHashTable_Set(&self->map, nsURI, (CcObject_t *)xmlspec);
    return TRUE;
}

void
CcXmlSpecMap_MakeTerminals(const CcXmlSpecMap_t * self, CcGlobals_t * globals)
{
    CcBitArray_t options; CcsXmlSpecOption_t option;
    CcArrayListIter_t iter; const CcXmlSpec_t * xmlspec;
    CcSymbolTable_t * symtab = &globals->symtab;

    CcBitArray(&options, XSO_SIZE);
    for (xmlspec = (const CcXmlSpec_t *)CcArrayList_FirstC(&self->storage, &iter);
	 xmlspec; xmlspec = (const CcXmlSpec_t *)CcArrayList_NextC(&self->storage, &iter)) {
	CcXmlSpec_MakeTerminals(xmlspec, globals);
	CcBitArray_Or(&options, &xmlspec->options);
    }
    for (option = XSO_UnknownTag; option < XSO_SIZE; ++option) {
	if (!CcBitArray_Get(&options, option)) continue;
	if (!CcSymbolTable_NewTerminalWithCheck(symtab, CcsXmlSpecOptionNames[option], 0))
	    CcsErrorPool_Error(globals->errpool, 0, 0,
			       "Symbol %s is defined twice.\n", CcsXmlSpecOptionNames[option]);
    }
    CcBitArray_Destruct(&options);
}

CcsBool_t
CcXmlSpecMap_Finish(CcXmlSpecMap_t * self)
{
    return TRUE;
}

void
CcXmlSpecMap_GetOptionKinds(const CcXmlSpecMap_t * self, int * kinds,
			    const CcGlobals_t * globals)
{
    int * cur; CcsXmlSpecOption_t option; const CcSymbol_t * sym;
    CcArrayListIter_t iter; const CcXmlSpec_t * xmlspec;
    const CcSymbolTable_t * symtab = &globals->symtab;

    for (cur = kinds; cur - kinds < XSO_SIZE; ++cur) *cur = -1;
    for (xmlspec = (const CcXmlSpec_t *)CcArrayList_FirstC(&self->storage, &iter);
	 xmlspec; xmlspec = (const CcXmlSpec_t *)CcArrayList_NextC(&self->storage, &iter)) {
	for (option = XSO_UnknownTag; option < XSO_SIZE; ++option) {
	    if (kinds[option] != -1) continue;
	    if (!CcBitArray_Get(&xmlspec->options, option)) continue;
	    sym = CcSymbolTable_CheckTerminal(symtab, CcsXmlSpecOptionNames[option]);
	    kinds[option] = sym->base.index;
	}
    }
}
