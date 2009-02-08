/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_SYMBOLTABLE_H
#define  COCO_SYMBOLTABLE_H

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

#ifndef  COCO_SYMBOLS_H
#include  "Symbols.h"
#endif

EXTC_BEGIN

struct CcSymbolTable_s {
    CcArrayList_t terminals;
    CcArrayList_t pragmas;
    CcArrayList_t nonterminals;
};

CcSymbolTable_t * CcSymbolTable(CcSymbolTable_t * self);
void CcSymbolTable_Destruct(CcSymbolTable_t * self);

/* name is a unescaped string. */
const CcSymbol_t *
CcSymbolTable_CheckTerminal(const CcSymbolTable_t * self, const char * name);
CcsBool_t
CcSymbolTable_NewTerminalWithCheck(CcSymbolTable_t * self, const char * name,
				   int line);

/* name is not unescaped yet. */
CcSymbol_t *
CcSymbolTable_NewTerminal(CcSymbolTable_t * self, const char * name, int line);

CcSymbol_t *
CcSymbolTable_NewPragma(CcSymbolTable_t * self, const char * name, int line);

CcSymbol_t *
CcSymbolTable_NewNonTerminal(CcSymbolTable_t * self,
			     const char * name, int line);

CcSymbol_t *
CcSymbolTable_FindSym(CcSymbolTable_t * self, const char * name);

CcsBool_t CcSymbolTable_Finish(CcSymbolTable_t * self);

EXTC_END

#endif  /* COCO_SYMBOLTABLE_H */
