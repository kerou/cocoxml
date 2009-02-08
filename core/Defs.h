/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_DEFS_H
#define  COCO_DEFS_H

#ifndef  COCO_CDEFS_H
#include  "c/CDefs.h"
#endif

EXTC_BEGIN

/* Basic DataStructures */
typedef struct CcObjectType_s CcObjectType_t;
typedef struct CcObject_s CcObject_t;
typedef struct CcString_s CcString_t;
typedef struct CcArrayList_s CcArrayList_t;
typedef struct CcBitArray_s CcBitArray_t;
typedef struct CcHashTable_s CcHashTable_t;

typedef struct CcSymbol_s CcSymbol_t;
typedef struct CcNode_s CcNode_t;

/* OutputScheme types. */
typedef struct CcOutputSchemeType_s CcOutputSchemeType_t;
typedef struct CcOutputScheme_s CcOutputScheme_t;
typedef struct CcSourceOutputSchemeType_s CcSourceOutputSchemeType_t;
typedef struct CcSourceOutputScheme_s CcSourceOutputScheme_t;

/* Algorithm types */
typedef struct CcLexical_s CcLexical_t;
typedef struct CcXmlSpecMap_s CcXmlSpecMap_t;
typedef struct CcSyntax_s CcSyntax_t;
typedef struct CcSymbolTable_s CcSymbolTable_t;

/* Lexical types */
typedef struct CcTransition_s CcTransition_t;
typedef struct CcAction_s CcAction_t;
typedef struct CcCharSet_s CcCharSet_t;
typedef struct CcCharClass_s CcCharClass_t;
typedef struct CcComment_s CcComment_t;
typedef struct CcMelted_s CcMelted_t;
typedef struct CcState_s CcState_t;
typedef struct CcTarget_s CcTarget_t;

/* XmlSpec types */
typedef enum CcXmlSpecOption_e CcXmlSpecOption_t;
typedef struct CcXmlSpec_s CcXmlSpec_t;

#define CcMalloc(size) _CcMalloc_(size, __FILE__, __LINE__)
void * _CcMalloc_(size_t size, const char * fname, int line);

#define CcRealloc(ptr, size) _CcRealloc_(ptr, size, __FILE__, __LINE__)
void * _CcRealloc_(void * ptr, size_t size, const char * fname, int line);

#define CcFree(ptr) _CcFree_(ptr, __FILE__, __LINE__)
void _CcFree_(void * ptr, const char * fname, int line);

#define CcStrdup(str) _CcStrdup_(str, __FILE__, __LINE__)
char * _CcStrdup_(const char * str, const char * fname, int line);

typedef struct CcGlobals_s CcGlobals_t;
typedef struct CcArguments_s CcArguments_t;

char * CcUnescape(const char * str);
char * CcEscape(const char * str);

EXTC_END

#endif /* COCO_DEFS_H */
