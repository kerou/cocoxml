/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Symbols.h"

static void
CcSymbol_Destruct(CcObject_t * self)
{
    CcSymbol_t * ccself = (CcSymbol_t *)self;
    CcFree(ccself->name);
    CcObject_Destruct(self);
}

static void
CcSymbolPR_Destruct(CcObject_t * self)
{
    CcSymbolPR_t * ccself = (CcSymbolPR_t *)self;
    if (ccself->semPos) CcsPosition_Destruct(ccself->semPos);
    CcSymbol_Destruct(self);
}

static void
CcSymbolNT_Destruct(CcObject_t * self)
{
    CcSymbolNT_t * ccself = (CcSymbolNT_t *)self;
    if (ccself->semPos)  CcsPosition_Destruct(ccself->semPos);
    if (ccself->attrPos)  CcsPosition_Destruct(ccself->attrPos);
    if (ccself->first)  CcBitArray_Destruct(ccself->first);
    if (ccself->follow)  CcBitArray_Destruct(ccself->follow);
    if (ccself->nts)  CcBitArray_Destruct(ccself->nts);
    CcSymbol_Destruct(self);
}

static const CcObjectType_t SymbolT = {
    sizeof(CcSymbolT_t), "symbol_t", CcSymbol_Destruct
};
const CcObjectType_t * symbol_t = &SymbolT;

static const CcObjectType_t SymbolPR = {
    sizeof(CcSymbolPR_t), "symbol_pr", CcSymbolPR_Destruct
};
const CcObjectType_t * symbol_pr = &SymbolPR;

static const CcObjectType_t SymbolNT = {
    sizeof(CcSymbolNT_t), "symbol_nt", CcSymbolNT_Destruct
};
const CcObjectType_t * symbol_nt = &SymbolNT;

static const CcObjectType_t SymbolUnknown = {
    sizeof(CcSymbolUnknown_t), "symbol_unknown", CcSymbol_Destruct
};
const CcObjectType_t * symbol_unknown = &SymbolUnknown;

static const CcObjectType_t SymbolRSLV = {
    sizeof(CcSymbolRSLV_t), "symbol_rslv", CcSymbol_Destruct
};
const CcObjectType_t * symbol_rslv = &SymbolRSLV;

CcSymbol_t *
CcSymbol(const CcObjectType_t * type, const char * name, int line)
{
    CcSymbol_t * self = (CcSymbol_t *)CcObject(type);
    self->name = CcStrdup(name);
    self->line = line;
    return self;
}

CcSymbol_TokenKind_t
CcSymbol_GetTokenKind(CcSymbol_t * self)
{
    if (self->base.type == symbol_t)
	return ((CcSymbolT_t *)self)->tokenKind;
    CcsAssert(self->base.type == symbol_pr);
    return ((CcSymbolPR_t *)self)->tokenKind;
}

void
CcSymbol_SetTokenKind(CcSymbol_t * self, CcSymbol_TokenKind_t tokenKind)
{
    if (self->base.type == symbol_t) {
	((CcSymbolT_t *)self)->tokenKind = tokenKind;
    } else {
	CcsAssert(self->base.type == symbol_pr);
	((CcSymbolPR_t *)self)->tokenKind = tokenKind;
    }
}
