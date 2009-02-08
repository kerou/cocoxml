/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_SYMBOLS_H
#define  COCO_SYMBOLS_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

#ifndef  COCO_BITARRAY_H
#include "BitArray.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

extern const CcObjectType_t * symbol_t;
extern const CcObjectType_t * symbol_nt;
extern const CcObjectType_t * symbol_pr;
extern const CcObjectType_t * symbol_unknown;
extern const CcObjectType_t * symbol_rslv;

typedef struct CcSymbolT_s CcSymbolT_t;
typedef struct CcSymbolPR_s CcSymbolPR_t;
typedef struct CcSymbolNT_s CcSymbolNT_t;
typedef struct CcSymbolUnknown_s CcSymbolUnknown_t;
typedef struct CcSymbolRSLV_s CcSymbolRSLV_t;

typedef enum {
    symbol_fixedToken = 0,
    symbol_classToken = 1,
    symbol_litToken = 2,
    symbol_classLitToken = 3
} CcSymbol_TokenKind_t;

struct CcSymbol_s {
    CcObject_t   base;
    int          kind;
    char       * name;
    int          line;
};
CcSymbol_t *
CcSymbol(const CcObjectType_t * type, const char * name, int line);

struct CcSymbolT_s {
    CcSymbol_t             base;
    CcSymbol_TokenKind_t   tokenKind;
};
#define CcSymbolT(name, line)  CcSymbol(symbol_t, (name), (line))

struct CcSymbolPR_s {
    CcSymbol_t             base;
    CcSymbol_TokenKind_t   tokenKind;
    CcsPosition_t        * semPos;
};
#define CcSymbolPR(name, line) CcSymbol(symbol_pr, (name), (line))

struct CcSymbolNT_s {
    CcSymbol_t      base;
    CcNode_t      * graph;
    CcsBool_t       deletable;
    CcsBool_t       firstReady;
    CcBitArray_t  * first;
    CcBitArray_t    firstSpace;
    CcBitArray_t  * follow;
    CcBitArray_t    followSpace;
    CcBitArray_t  * nts;
    CcBitArray_t    ntsSpace;
    CcsPosition_t * attrPos;
    CcsPosition_t * semPos;
};
#define CcSymbolNT(name, line) CcSymbol(symbol_nt, (name), (line))

struct CcSymbolUnknown_s {
    CcSymbol_t base;
};
#define CcSymbolUnknown(name, line) CcSymbol(symbol_unknown, (name), (line))

struct CcSymbolRSLV_s {
    CcSymbol_t base;
};
#define CcSymbolRSLV(name, line) CcSymbol(symbol_rslv, (name), (line))

CcSymbol_TokenKind_t CcSymbol_GetTokenKind(CcSymbol_t * self);
void CcSymbol_SetTokenKind(CcSymbol_t * self, CcSymbol_TokenKind_t tokenKind);

EXTC_END

#endif  /* COCO_SYMBOLS_H */
