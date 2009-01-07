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

typedef struct KcProperty_s KcProperty_t;
typedef struct KcSymbol_s KcSymbol_t;
typedef struct KcExpr_s KcExpr_t;
typedef struct KcMenuEntry_s KcMenuEntry_t;
typedef struct KcMenu_s KcMenu_t;

#define  KcNo      0
#define  KcModule  1
#define  KcYes     2

typedef enum {
    KcptPropmt, KcptDefault, KcptDepends
}   KcPropertyType_t;
struct KcProperty_s {
    KcPropertyType_t type;
    KcProperty_t * next;
};

typedef enum {
    KcstNone, KcstBool, KcstTristate, KcstString, KcstHex, KcstInt
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
    KcProperty_t * propFirst;
    KcProperty_t * propLast;
};

const char *
KcSymbol_AppendPrompt(KcSymbol_t * self, const char * prompt);
const char *
KcSymbol_AppendDefault(KcSymbol_t * self, const char * _default_);
const char *
KcSymbol_AppendDepends(KcSymbol_t * self, KcExpr_t * depExpr);

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

KcExpr_t * KcExpr(KcExprType_t type,
		  KcSymbol_t * sym0, KcSymbol_t * sym1,
		  KcExpr_t * exp0, KcExpr_t * exp1);
void KcExpr_Destruct(KcExpr_t * self);

typedef struct {
    KcSymbol_t ** first;
    KcSymbol_t ** last;
}   KcSymbolTable_t;

KcSymbolTable_t * KcSymbolTable(void);
void KcSymbolTable_Destruct(KcSymbolTable_t * self);

/* Return NULL in success, return error message format when failed.
 * The only formatter is '%s' which will be replaced by symname. */
const char *
KcSymbolTable_SetBool(KcSymbolTable_t * self, const char * symname,
		      const char * value);
const char *
KcSymbolTable_SetTristate(KcSymbolTable_t * self, const char * symname,
			  const char * value);
const char *
KcSymbolTable_SetString(KcSymbolTable_t * self, const char * symname,
			const char * value);
const char *
KcSymbolTable_SetHex(KcSymbolTable_t * self, const char * symname,
		     const char * value);
const char *
KcSymbolTable_SetInt(KcSymbolTable_t * self, const char * symname,
		     const char * value);

/* Get the specified symbol. If the symbol is not defined yet, return a 'none'
 * symbol with the specified name. */
KcSymbol_t * KcSymbolTable_Get(KcSymbolTable_t * self, const char * symname);

typedef enum {
    KcmetSymbol, KcmetSubmenu
}   KcMenuEntryType_t;
struct KcMenuEntry_s {
    KcMenuEntryType_t type;
    KcMenuEntry_t * next;
    union {
	KcSymbol_t * symbol;
	KcMenu_t * submenu;
    } u;
};

struct KcMenu_s {
    KcMenuEntry_t * first;
    KcMenuEntry_t * last;
};

KcMenu_t * KcMenu(void);
void KcMenu_Destruct(KcMenu_t * self);

const char * KcMenu_AppendSymbol(KcMenu_t * self, KcSymbol_t * symbol);
const char * KcMenu_AppendSubmenu(KcMenu_t * self, KcMenu_t * submenu);

EXTC_END

#endif  /* COCO_KCDATA_H */
