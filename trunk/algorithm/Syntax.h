/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_SYNTAX_H
#define  COCO_SYNTAX_H

#ifndef  COCO_BITARRAY_H
#include  "BitArray.h"
#endif

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

#ifndef  COCO_EBNF_H
#include  "EBNF.h"
#endif

EXTC_BEGIN

struct CcSyntax_s {
    CcEBNF_t        base;
    CcGlobals_t   * globals;

    CcsPosition_t * members;
    CcsPosition_t * constructor;
    CcsPosition_t * destructor;

    char          * schemeName;
    char          * grammarPrefix;
    CcSymbol_t    * gramSy;
    CcSymbol_t    * eofSy;
    CcSymbol_t    * noSy;
    CcSymbol_t    * curSy;
    /* Sometimes the length of visited is nodes.Count.
     * And sometimes the length of visited is nonterminals.Count...... */
    CcBitArray_t  * visited;
    CcBitArray_t    visitedSpace;
    CcBitArray_t  * allSyncSets;
    CcBitArray_t    allSyncSetsSpace;

    CcArrayList_t   errors;
};

CcSyntax_t * CcSyntax(CcSyntax_t * self, CcGlobals_t * globals);
void CcSyntax_Destruct(CcSyntax_t * self);

void CcSyntax_First(CcSyntax_t * self, CcBitArray_t * ret, CcNode_t * p);

void CcSyntax_Expected(CcSyntax_t * self, CcBitArray_t * ret,
		       CcNode_t * p, const CcSymbol_t * curSy);
void CcSyntax_Expected0(CcSyntax_t * self, CcBitArray_t * ret,
			CcNode_t * p, const CcSymbol_t * curSy);

CcNode_t *
CcSyntax_NodeFromSymbol(CcSyntax_t * self, const CcSymbol_t * sym, int line,
			CcsBool_t weak);
void CcSyntax_SetupAnys(CcSyntax_t * self);

CcsBool_t CcSyntax_Finish(CcSyntax_t * self);

typedef enum {
    cet_t, cet_alt, cet_sync
}  CcSyntaxErrorType_t;
typedef struct {
    CcObject_t base;
    CcSyntaxErrorType_t type;
    const CcSymbol_t * sym;
}  CcSyntaxError_t;

int CcSyntax_AltError(CcSyntax_t * self, const CcSymbol_t * sym);
int CcSyntax_SyncError(CcSyntax_t * self, const CcSymbol_t * sym);

typedef struct {
    CcBitArray_t * start;
    CcBitArray_t * used;
    CcBitArray_t * last;
}  CcSyntaxSymSet_t;

void CcSyntaxSymSet(CcSyntaxSymSet_t * self);
int CcSyntaxSymSet_New(CcSyntaxSymSet_t * self, const CcBitArray_t * s);
void CcSyntaxSymSet_Destruct(CcSyntaxSymSet_t * self);

EXTC_END

#endif  /* COCO_SYNTAX_H */
