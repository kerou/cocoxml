/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

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
#ifndef  COCO_KCDATA_H
#define  COCO_KCDATA_H

#ifndef  COCO_CDEFS_H
#include  "c/CDefs.h"
#endif

EXTC_BEGIN

typedef struct KcSymbol_s KcSymbol_t;
typedef struct KcExpr_s KcExpr_t;

#define  KcNo      0
#define  KcModule  1
#define  KcYes     2

typedef enum {
    KcstBool, KcstTristate, KcstString, KcstHex, KcstInt
}   KcSymbolType_t;
struct KcSymbol_s {
    KcSymbolType_t type;
    KcSymbol_t * next;
    char * symname;
    union {
	int _bool_;
	int _tristate_;
	char * _string_;
	unsigned _hex_;
	int _int_;
    } u;
};

typedef struct {
    KcSymbol_t *  firstsym;
    KcSymbol_t *  lastsym;
    KcSymbol_t ** first;
    KcSymbol_t ** last;
}   KcSymbolTable_t;

typedef enum {
    KcetSymbol, KcetSymbolEqual, KcetSymbolNotEqual,
    KcetNotExpr, KcetExprAnd, KcetExprOr
}   KcExprType_t;
struct KcExpr_s {
    KcExprType_t type;
    union {
	struct { /* KcetSymbol/KcetSymbolEqual/KcetSymbolNotEqual */
	    KcSymbol_t * symbol0;
	    KcSymbol_t * symbol1;
	} s;
	struct { /* KcetNotExpr/KcetExprAnd/KcetExprOr */
	    KcExpr_t * expr0;
	    KcExpr_t * expr1;
	} e;
    } u;
};

KcSymbolTable_t * KcSymbolTable(size_t space);
void KcSymbolTable_Destruct(KcSymbolTable_t * self);

KcSymbol_t *
KcSymbolTable_AddBool(KcSymbolTable_t * self, const char * symname,
		      const char * value);
KcSymbol_t *
KcSymbolTable_AddTristate(KcSymbolTable_t * self, const char * symname,
			  const char * value);
KcSymbol_t *
KcSymbolTable_AddString(KcSymbolTable_t * self, const char * symname,
			const char * value);
KcSymbol_t *
KcSymbolTable_AddHex(KcSymbolTable_t * self, const char * symname,
		     const char * value);
KcSymbol_t *
KcSymbolTable_AddInt(KcSymbolTable_t * self, const char * symname,
		     const char * value);
KcSymbol_t * KcSymbolTable_Get(KcSymbolTable_t * self, const char * symname);

KcExpr_t * KcExpr(KcExprType_t type,
		  KcSymbol_t * sym0, KcSymbol_t * sym1,
		  KcExpr_t * exp0, KcExpr_t * exp1);
void KcExpr_Destruct(KcExpr_t * self);

EXTC_END

#endif  /* COCO_KCDATA_H */
