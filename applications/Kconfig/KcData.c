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

static KcSymbol_t *
KcSymbol(KcSymbolType_t type, const char * symname)
{
    KcSymbol_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbol_t) + strlen(symname) + 1)))
	return NULL;
    self->type = type;
    self->symname = (char *)(self + 1);
    strcpy(self->symname, symname);
    return self;
}

KcSymbol_t *
KcBoolSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstBool, symname))) return NULL;
    self->u._bool_ = *value == 'y' || *value == 'Y' ? KcYes : KcNo;
    return self;
}

KcSymbol_t *
KcTristateSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstTristate, symname))) return NULL;
    self->u._tristate_ = *value == 'y' || *value == 'Y' ? KcYes :
	*value == 'm' || *value == 'M' ? KcModule : KcNo;
    return self;
}

KcSymbol_t *
KcStringSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstString, symname))) goto errquit0;
    if (!(self->u._string_ = CcsStrdup(value))) goto errquit1;
    return self;
 errquit1:
    KcSymbol_Destruct(self);
 errquit0:
    return NULL;
}

KcSymbol_t *
KcHexSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    const char * cur;
    if (!(self = KcSymbol(KcstTristate, symname))) return NULL;
    self->u._hex_ = 0;
    for (cur = value; *cur; ++cur)
	if (*cur >= '0' && *cur <= '9')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - '0');
	else if (*cur >= 'A' && *cur <= 'Z')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - 'A' + 10);
	else if (*cur >= 'a' && *cur <= 'z')
	    self->u._hex_ = (self->u._hex_ << 4) + (*cur - 'a' + 10);
    return self;
}

KcSymbol_t *
KcIntSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstTristate, symname))) return NULL;
    self->u._int_ = atoi(value);
    return self;
}

void
KcSymbol_Destruct(KcSymbol_t * self)
{
    if (self->type == KcstString && self->u._string_ != NULL)
	CcsFree(self->u._string_);
    CcsFree(self);
}

KcSymbolTable_t *
KcSymbolTable(int space)
{
    KcSymbolTable_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbolTable_t)))) goto errquit0;
    if (!(self->first = CcsMalloc(sizeof(KcSymbol_t *) * space))) goto errquit1;
    memset(self->first, 0, sizeof(KcSymbol_t *) * space);
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

void
KcSymbolTable_Add(KcSymbolTable_t * self, KcSymbol_t * sym)
{
    KcSymbol_t ** start, ** cur;
    start = self->first + strhash(sym->symname, self->last - self->first);
    do {
	if (*cur == NULL) { *cur = sym; return; }
	if (++cur == self->last) cur = self->first;
    } while (cur != start);
    CcsAssert(0); /* Full */
}

KcSymbol_t *
KcSymbolTable_Get(KcSymbolTable_t * self, const char * symname)
{
    KcSymbol_t ** start, ** cur;
    start = self->first + strhash(symname, self->last - self->first);
    do {
	if (*cur == NULL) return NULL;
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
