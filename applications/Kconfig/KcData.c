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
KcSymbol(KcSymbolType_t type, const char * symname)
{
    KcSymbol_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbol_t) + strlen(symname) + 1)))
	return NULL;
    self->type = type;
    self->next = NULL;
    self->symname = (char *)(self + 1);
    strcpy(self->symname, symname);
    return self;
}

static KcSymbol_t *
KcBoolSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstBool, symname))) return NULL;
    self->u._bool_ = *value == 'y' || *value == 'Y' ? KcYes : KcNo;
    return self;
}

static KcSymbol_t *
KcTristateSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstTristate, symname))) return NULL;
    self->u._tristate_ = *value == 'y' || *value == 'Y' ? KcYes :
	*value == 'm' || *value == 'M' ? KcModule : KcNo;
    return self;
}

static KcSymbol_t *
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

static KcSymbol_t *
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

static KcSymbol_t *
KcIntSymbol(const char * symname, const char * value)
{
    KcSymbol_t * self;
    if (!(self = KcSymbol(KcstTristate, symname))) return NULL;
    self->u._int_ = atoi(value);
    return self;
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

static KcSymbol_t *
KcSymbolTable_Add(KcSymbolTable_t * self,
		  KcSymbol_t * (* creator)(const char * symname,
					   const char * value),
		  const char * symname, const char * value)
{
    KcSymbol_t ** start, ** cur;
    start = self->first + strhash(symname, self->last - self->first);
    do {
	if (*cur == NULL) {
	    if (!(*cur = creator(symname, value))) return NULL;
	    if (!self->firstsym) self->firstsym = *cur;
	    if (self->lastsym) self->lastsym->next = *cur;
	    self->lastsym = *cur;
	    return *cur;
	}
	if (++cur == self->last) cur = self->first;
    } while (cur != start);
    return NULL; /* Full */
}

KcSymbol_t *
KcSymbolTable_AddBool(KcSymbolTable_t * self, const char * symname,
		      const char * value)
{
    return KcSymbolTable_Add(self, KcBoolSymbol, symname, value);
}

KcSymbol_t *
KcSymbolTable_AddTristate(KcSymbolTable_t * self, const char * symname,
			  const char * value)
{
    return KcSymbolTable_Add(self, KcTristateSymbol, symname, value);
}

KcSymbol_t *
KcSymbolTable_AddString(KcSymbolTable_t * self, const char * symname,
			const char * value)
{
    return KcSymbolTable_Add(self, KcStringSymbol, symname, value);
}

KcSymbol_t *
KcSymbolTable_AddHex(KcSymbolTable_t * self, const char * symname,
		     const char * value)
{
    return KcSymbolTable_Add(self, KcHexSymbol, symname, value);
}

KcSymbol_t *
KcSymbolTable_AddInt(KcSymbolTable_t * self, const char * symname,
		     const char * value)
{
    return KcSymbolTable_Add(self, KcIntSymbol, symname, value);
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
