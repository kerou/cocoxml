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
#include  "KcData.h"

static void KcSymbol_Destruct(KcSymbol_t * self);

static KcSymbol_t *
KcSymbol(const char * symname)
{
    KcSymbol_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbol_t) + strlen(symname) + 1)))
	return NULL;
    self->type = KcstNone;
    self->next = NULL;
    self->symname = (char *)(self + 1);
    strcpy(self->symname, symname);
    return self;
}

static const char *
KcSetBool(KcSymbol_t * self, const char * value)
{
    self->u._bool_ = *value == 'y' || *value == 'Y' ? KcYes : KcNo;
    return NULL;
}

static const char *
KcSetTristate(KcSymbol_t * self, const char * value)
{
    self->u._tristate_ = *value == 'y' || *value == 'Y' ? KcYes :
	*value == 'm' || *value == 'M' ? KcModule : KcNo;
    return NULL;
}

static const char *
KcSetString(KcSymbol_t * self, const char * value)
{
    return self->u._string_ = CcsStrdup(value) ? NULL : "Not enough memory";
}

static const char *
KcSetHex(KcSymbol_t * self, const char * value)
{
    const char * cur;
    self->u._hex_ = 0;
    for (cur = value; *cur; ++cur)
	if (*cur >= '0' && *cur <= '9')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - '0');
	else if (*cur >= 'A' && *cur <= 'Z')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - 'A' + 10);
	else if (*cur >= 'a' && *cur <= 'z')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - 'a' + 10);
    return NULL;
}

static const char *
KcSetInt(KcSymbol_t * self, const char * value)
{
    self->u._int_ = atoi(value);
    return NULL;
}

static void
KcSymbol_Destruct(KcSymbol_t * self)
{
    if (self->type == KcstString && self->u._string_ != NULL)
	CcsFree(self->u._string_);
    CcsFree(self);
}

KcSymbolTable_t *
KcSymbolTable(size_t space)
{
    KcSymbolTable_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbolTable_t)))) goto errquit0;
    if (!(self->first = CcsMalloc(sizeof(KcSymbol_t *) * space))) goto errquit1;
    memset(self->first, 0, sizeof(KcSymbol_t *) * space);
    self->firstsym = self->lastsym = NULL;
    self->last = self->first + space;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
}

void
KcSymbolTable_Destruct(KcSymbolTable_t * self)
{
    KcSymbol_t ** cur;
    for (cur = self->first; cur < self->last; ++cur)
	if (*cur) KcSymbol_Destruct(*cur);
    CcsFree(self);
}

static int
strhash(const char * str, int szhash)
{
    int value = 0;
    const char * cur;
    for (cur = str; *cur; ++cur)
	value += *cur;
    return value % szhash;
}

static const char *
KcSymbolTable_Set(KcSymbolTable_t * self, KcSymbolType_t type,
		  const char * (* setfunc)(KcSymbol_t * sym,
					   const char * value),
		  const char * symname, const char * value)
{
    KcSymbol_t ** start, ** cur;
    start = self->first + strhash(symname, self->last - self->first);
    do {
	if (!*cur || !strcmp((*cur)->symname, symname)) {
	    if (!*cur) {
		if (!(*cur = KcSymbol(symname))) return "Not enough memory";
	    }
	    if ((*cur)->type != KcstNone && (*cur)->type != type)
		return "Type conflict for symbol '%s'";
	    if ((*cur)->type == KcstNone) {
		if (!self->firstsym) self->firstsym = *cur;
		if (self->lastsym) self->lastsym->next = *cur;
		self->lastsym = *cur;
		(*cur)->type = type;
	    }
	    return setfunc(*cur, value);
	}
	if (++cur == self->last) cur = self->first;
    } while (cur != start);
    return "The symbol table is full when tring to add '%s'";
}

const char *
KcSymbolTable_SetBool(KcSymbolTable_t * self, const char * symname,
		      const char * value)
{
    return KcSymbolTable_Set(self, KcstBool, KcSetBool, symname, value);
}

const char *
KcSymbolTable_SetTristate(KcSymbolTable_t * self, const char * symname,
			  const char * value)
{
    return KcSymbolTable_Set(self, KcstTristate, KcSetTristate, symname, value);
}

const char *
KcSymbolTable_SetString(KcSymbolTable_t * self, const char * symname,
			const char * value)
{
    return KcSymbolTable_Set(self, KcstString, KcSetString, symname, value);
}

const char *
KcSymbolTable_SetHex(KcSymbolTable_t * self, const char * symname,
		     const char * value)
{
    return KcSymbolTable_Set(self, KcstHex, KcSetHex, symname, value);
}

const char *
KcSymbolTable_SetInt(KcSymbolTable_t * self, const char * symname,
		     const char * value)
{
    return KcSymbolTable_Set(self, KcstInt, KcSetInt, symname, value);
}

KcSymbol_t *
KcSymbolTable_Get(KcSymbolTable_t * self, const char * symname)
{
    KcSymbol_t ** start, ** cur;
    start = self->first + strhash(symname, self->last - self->first);
    do {
	if (*cur == NULL) return *cur = KcSymbol(symname);
	if (!strcmp((*cur)->symname, symname)) return *cur;
	if (++cur == self->last) cur = self->first;
    } while (cur != start);
    return NULL;
}

KcExpr_t *
KcExpr(KcExprType_t type, KcSymbol_t * sym0, KcSymbol_t * sym1,
       KcExpr_t * exp0, KcExpr_t * exp1)
{
    KcExpr_t * self;
    if (!(self = CcsMalloc(sizeof(KcExpr_t)))) return NULL;
    self->type = type;
    switch (type) {
    case KcetSymbol: case KcetSymbolEqual: case KcetSymbolNotEqual:
	self->u.s.symbol0 = sym0; self->u.s.symbol1 = sym1;
	break;
    case KcetNotExpr: case KcetExprAnd: case KcetExprOr:
	self->u.e.expr0 = exp0; self->u.e.expr1 = exp1;
	break;
    }
    return self;
}

void
KcExpr_Destruct(KcExpr_t * self)
{
    switch (self->type) {
    case KcetSymbol: case KcetSymbolEqual: case KcetSymbolNotEqual:
	/* Symbol is not belong to Expr. */
	break;
    case KcetNotExpr: case KcetExprAnd: case KcetExprOr:
	if (self->u.e.expr0) KcExpr_Destruct(self->u.e.expr0);
	if (self->u.e.expr1) KcExpr_Destruct(self->u.e.expr1);
	break;
    }
    CcsFree(self);
}
