/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "XmlSpec.h"
#include  "Globals.h"
#include  "SymbolTable.h"
#include  "c/ErrorPool.h"
#include  "c/Token.h"

static void CcXmlSpec_Destruct(CcObject_t * self);

static const CcObjectType_t XmlSpecType = {
    sizeof(CcXmlSpec_t), "XmlSpec", CcXmlSpec_Destruct
};

CcXmlSpec_t *
CcXmlSpec(CcGlobals_t * globals)
{
    CcXmlSpec_t * self = (CcXmlSpec_t *)CcObject(&XmlSpecType);
    self->globals = globals;
    self->caseSensitive = TRUE;
    CcBitArray(&self->options, XSO_SIZE);
    CcArrayList(&self->Tags);
    CcArrayList(&self->Attrs);
    CcArrayList(&self->PInstructions);
    return self;
}

static void
CcXmlSpec_Destruct(CcObject_t * self)
{
    CcXmlSpec_t * ccself = (CcXmlSpec_t *)self;
    CcArrayList_Destruct(&ccself->PInstructions);
    CcArrayList_Destruct(&ccself->Attrs);
    CcArrayList_Destruct(&ccself->Tags);
    CcBitArray_Destruct(&ccself->options);
    CcObject_Destruct(self);
}

void
CcXmlSpec_SetCaseSensitive(CcXmlSpec_t * self, CcsBool_t caseSensitive)
{
    self->caseSensitive = caseSensitive;
}

void
CcXmlSpec_SetOption(CcXmlSpec_t * self, const CcsToken_t * token)
{
    CcsXmlSpecOption_t option;
    for (option = XSO_UnknownTag; option < XSO_SIZE; ++option)
	if (!strcmp(token->val, CcsXmlSpecOptionNames[option])) {
	    CcBitArray_Set(&self->options, option, TRUE);
	    return;
	}
    CcsErrorPool_Error(self->globals->errpool, &token->loc,
		       "Unrecognized option '%s' encountered.", token->val);
}

typedef struct {
    CcObject_t base;
    char * tokenName;
    char * name;
    CcsLocation_t loc;
}  CcXmlData_t;

static void
CcXmlData_Destruct(CcObject_t * self)
{
    CcXmlData_t * ccself = (CcXmlData_t *)self;
    if (ccself->name) CcFree(ccself->name);
    if (ccself->tokenName) CcFree(ccself->tokenName);
    CcObject_Destruct(self);
}

static const CcObjectType_t XmlDataType = {
    sizeof(CcXmlData_t), "XmlData", CcXmlData_Destruct
};

static CcObject_t *
CcXmlData(const char * tokenName, const CcsToken_t * token)
{
    CcXmlData_t * self = (CcXmlData_t *)CcObject(&XmlDataType);
    self->tokenName = CcUnescape(tokenName);
    self->name = CcUnescape(token->val);
    memcpy(&self->loc, &token->loc, sizeof(token->loc));
    return (CcObject_t *)self;
}

void
CcXmlSpec_AddTag(CcXmlSpec_t * self, const char * tokenName,
		 const CcsToken_t * token)
{
    CcArrayList_New(&self->Tags, CcXmlData(tokenName, token));
}

void
CcXmlSpec_AddAttr(CcXmlSpec_t * self, const char * tokenName,
		  const CcsToken_t * token)
{
    CcArrayList_New(&self->Attrs, CcXmlData(tokenName, token));
}

void
CcXmlSpec_AddProcessInstruction(CcXmlSpec_t * self, const char * tokenName,
				const CcsToken_t * token)
{
    CcArrayList_New(&self->PInstructions, CcXmlData(tokenName, token));
}

void
CcXmlSpec_MakeTerminals(const CcXmlSpec_t * self, CcGlobals_t * globals)
{
    char EndTag[128];
    CcArrayListIter_t iter; const CcXmlData_t * data;
    CcSymbolTable_t * symtab = &globals->symtab;

    for (data = (const CcXmlData_t *)CcArrayList_FirstC(&self->Tags, &iter);
	 data; data = (const CcXmlData_t *)CcArrayList_NextC(&self->Tags, &iter)) {
	if (!CcSymbolTable_NewTerminalWithCheck(symtab, data->tokenName, data->loc.line))
	    CcsErrorPool_Error(globals->errpool, &data->loc,
			       "Symbol %s is defined twice.\n", data->tokenName);
	snprintf(EndTag, sizeof(EndTag), "END_%s", data->tokenName);
	if (!CcSymbolTable_NewTerminalWithCheck(symtab, EndTag, data->loc.line))
	    CcsErrorPool_Error(globals->errpool, &data->loc,
			       "Symbol %s is defined twice.\n", EndTag);
    }

    for (data = (const CcXmlData_t *)CcArrayList_FirstC(&self->Attrs, &iter);
	 data; data = (const CcXmlData_t *)CcArrayList_NextC(&self->Attrs, &iter))
	if (!CcSymbolTable_NewTerminalWithCheck(symtab, data->tokenName, data->loc.line))
	    CcsErrorPool_Error(globals->errpool, &data->loc,
			       "Symbol %s is defined twice.\n", data->tokenName);
	
    for (data = (const CcXmlData_t *)CcArrayList_FirstC(&self->PInstructions, &iter);
	 data; data = (const CcXmlData_t *)CcArrayList_NextC(&self->PInstructions, &iter))
	if (!CcSymbolTable_NewTerminalWithCheck(symtab, data->tokenName, data->loc.line))
	    CcsErrorPool_Error(globals->errpool, &data->loc,
			       "Symbol %s is defined twice.\n", data->tokenName);
}

static int
cmpXSData(const void * data0, const void * data1)
{
    return strcmp(((const CcXmlSpecData_t *)data0)->name,
		  ((const CcXmlSpecData_t *)data1)->name);
}

static CcXmlSpecData_t *
CcXmlSpec_GetSortedList(const CcArrayList_t * array, CcsBool_t useEnd,
			const CcGlobals_t * globals, size_t * retNum)
{
    CcXmlSpecData_t * retlist, * curret;
    const CcXmlData_t * data; CcArrayListIter_t iter;
    const CcSymbol_t * sym; char EndTag[128];
    const CcSymbolTable_t * symtab = &globals->symtab;

    *retNum = array->Count;
    if (array->Count == 0) return NULL;
    retlist = curret = CcMalloc(array->Count * sizeof(CcXmlSpecData_t));
    for (data = (const CcXmlData_t *)CcArrayList_FirstC(array, &iter);
	 data; data = (const CcXmlData_t *)CcArrayList_NextC(array, &iter)) {
	curret->name = CcStrdup(data->name);
	sym = CcSymbolTable_CheckTerminal(symtab, data->tokenName);
	CcsAssert(sym != NULL);
	curret->kind0 = sym->base.index;
	if (useEnd) {
	    snprintf(EndTag, sizeof(EndTag), "END_%s", data->tokenName);
	    sym = CcSymbolTable_CheckTerminal(symtab, EndTag);
	    CcsAssert(sym != NULL);
	    curret->kind1 = sym->base.index;
	}
	++curret;
    }
    CcsAssert(curret - retlist == array->Count);
    qsort(retlist, array->Count, sizeof(CcXmlSpecData_t), cmpXSData);
    return retlist;
}

CcXmlSpecData_t *
CcXmlSpec_GetSortedTagList(const CcXmlSpec_t * self,
			   const CcGlobals_t * globals, size_t * retNum)
{
    return CcXmlSpec_GetSortedList(&self->Tags, TRUE, globals, retNum);
}

CcXmlSpecData_t *
CcXmlSpec_GetSortedAttrList(const CcXmlSpec_t * self,
			    const CcGlobals_t * globals, size_t * retNum)
{
    return CcXmlSpec_GetSortedList(&self->Attrs, FALSE, globals, retNum);
}

CcXmlSpecData_t *
CcXmlSpec_GetSortedPIList(const CcXmlSpec_t * self,
			  const CcGlobals_t * globals, size_t * retNum)
{
    return CcXmlSpec_GetSortedList(&self->PInstructions, FALSE, globals, retNum);
}

void
CcXmlSpecData_Destruct(CcXmlSpecData_t * self, size_t num)
{
    CcXmlSpecData_t * cur;
    for (cur = self; cur - self < num; ++cur) CcFree(cur->name);
    CcFree(self);
}
