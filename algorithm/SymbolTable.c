/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "SymbolTable.h"
#include  "Symbols.h"

CcSymbolTable_t *
CcSymbolTable(CcSymbolTable_t * self)
{
    CcArrayList(&self->terminals);
    CcArrayList(&self->nonterminals);
    CcArrayList(&self->pragmas);
    return self;
}

void
CcSymbolTable_Destruct(CcSymbolTable_t * self)
{
    CcArrayList_Destruct(&self->nonterminals);
    CcArrayList_Destruct(&self->terminals);
    CcArrayList_Destruct(&self->pragmas);
}

const CcSymbol_t *
CcSymbolTable_CheckTerminal(const CcSymbolTable_t * self, const char * name)
{
    CcArrayListIter_t iter; const CcSymbol_t * sym;
    for (sym = (const CcSymbol_t *)CcArrayList_FirstC(&self->terminals, &iter);
	 sym; sym = (const CcSymbol_t *)CcArrayList_NextC(&self->terminals, &iter))
	if (!strcmp(sym->name, name)) return sym;
    return NULL;
}

CcsBool_t
CcSymbolTable_NewTerminalWithCheck(CcSymbolTable_t * self, const char * name,
				   int line)
{
    const CcSymbol_t * sym = CcSymbolTable_CheckTerminal(self, name);
    if (sym) return FALSE;
    CcArrayList_New(&self->terminals, (CcObject_t *)CcSymbolT(name, line));
    return TRUE;
}

CcSymbol_t *
CcSymbolTable_NewTerminal(CcSymbolTable_t * self, const char * name, int line)
{
    CcObject_t * sym = CcArrayList_New(&self->terminals,
				       (CcObject_t *)CcSymbolT(name, line));
    return (CcSymbol_t *)sym;
}

CcSymbol_t *
CcSymbolTable_NewPragma(CcSymbolTable_t * self, const char * name, int line)
{
    CcObject_t * sym = CcArrayList_New(&self->pragmas,
				       (CcObject_t *)CcSymbolPR(name, line));
    return (CcSymbol_t *)sym;
}

CcSymbol_t *
CcSymbolTable_NewNonTerminal(CcSymbolTable_t * self,
			     const char * name, int line)
{
    CcObject_t * sym = CcArrayList_New(&self->nonterminals,
				       (CcObject_t *)CcSymbolNT(name, line));
    return (CcSymbol_t *)sym;
}

CcSymbol_t *
CcSymbolTable_FindSym(CcSymbolTable_t * self, const char * name)
{
    CcArrayListIter_t iter; CcSymbol_t * sym;
    for (sym = (CcSymbol_t *)CcArrayList_First(&self->terminals, &iter);
	 sym; sym = (CcSymbol_t *)CcArrayList_Next(&self->terminals, &iter))
	if (!strcmp(sym->name, name)) return sym;
    for (sym = (CcSymbol_t *)CcArrayList_First(&self->nonterminals, &iter);
	 sym; sym = (CcSymbol_t *)CcArrayList_Next(&self->nonterminals, &iter))
	if (!strcmp(sym->name, name)) return sym;
    return NULL;
}

CcsBool_t
CcSymbolTable_Finish(CcSymbolTable_t * self)
{
    CcArrayListIter_t iter; CcSymbol_t * sym;
    for (sym = (CcSymbol_t *)CcArrayList_First(&self->terminals, &iter);
	 sym; sym = (CcSymbol_t *)CcArrayList_Next(&self->terminals, &iter))
	sym->kind = sym->base.index;
    for (sym = (CcSymbol_t *)CcArrayList_First(&self->pragmas, &iter);
	 sym; sym = (CcSymbol_t *)CcArrayList_Next(&self->pragmas, &iter))
	sym->kind = sym->base.index + self->terminals.Count;
    for (sym = (CcSymbol_t *)CcArrayList_First(&self->nonterminals, &iter);
	 sym; sym = (CcSymbol_t *)CcArrayList_Next(&self->nonterminals, &iter))
	sym->kind = sym->base.index;
    return TRUE;
}
