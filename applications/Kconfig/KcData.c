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

static void
KcProperty_Destruct(KcProperty_t * self)
{
    CcsFree(self);
}

static KcSymbol_t *
KcSymbol(const char * symname)
{
    KcSymbol_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbol_t) + strlen(symname) + 1)))
	return NULL;
    memset(self, 0, sizeof(KcSymbol_t));
    self->type = KcstNone;
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
    if (self->type == KcstString && self->u._string_) CcsFree(self->u._string_);
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
    KcProperty_t * cur, * next;
    for (cur = self->propFirst; cur; cur = next) {
	next = cur->next;
	KcProperty_Destruct(cur);
    }
    if (self->type == KcstString && self->u._string_ != NULL)
	CcsFree(self->u._string_);
    CcsFree(self);
}

const char *
KcSymbol_AppendPrompt(KcSymbol_t * self, const char * prompt)
{
}

const char *
KcSymbol_AppendDefault(KcSymbol_t * self, const char * _default_)
{
}

const char *
KcSymbol_AppendDepends(KcSymbol_t * self, KcExpr_t * depExpr)
{
}

#ifndef  KCSIZE_SYMTAB
#define  KCSIZE_SYMTAB  509
#endif

KcSymbolTable_t *
KcSymbolTable(void)
{
    KcSymbolTable_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbolTable_t)))) goto errquit0;
    if (!(self->first = CcsMalloc(sizeof(KcSymbol_t *) * KCSIZE_SYMTAB)))
	goto errquit1;
    memset(self->first, 0, sizeof(KcSymbol_t *) * KCSIZE_SYMTAB);
    self->last = self->first + KCSIZE_SYMTAB;
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

static KcSymbol_t **
symtabHash(KcSymbolTable_t * self, const char * symname)
{
    int value = 0;
    const char * cur;
    for (cur = symname; *cur; ++cur)
	value += *cur;
    return self->first + value % (self->last - self->first);
}

static const char *
KcSymbolTable_Set(KcSymbolTable_t * self, KcSymbolType_t type,
		  const char * (* setfunc)(KcSymbol_t * sym,
					   const char * value),
		  const char * symname, const char * value)
{
    KcSymbol_t ** cur;
    for (cur = symtabHash(self, symname); *cur; cur = &((*cur)->next)) {
	if (strcmp((*cur)->symname, symname)) continue;
	if ((*cur)->type == KcstNone) (*cur)->type = type;
	else if ((*cur)->type != type) return "Type conflict for symbol '%s'";
	return setfunc(*cur, value);
    }
    if (!(*cur = KcSymbol(symname))) return "Not enough memory";
    return setfunc(*cur, value);
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
    KcSymbol_t ** cur;
    for (cur = symtabHash(self, symname); *cur; cur = &((*cur)->next))
	if (!strcmp((*cur)->symname, symname)) return *cur;
    return *cur = KcSymbol(symname);
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

static KcMenuEntry_t *
KcMenuEntry(KcMenuEntryType_t type, KcSymbol_t * symbol, KcMenu_t * submenu)
{
    KcMenuEntry_t * self;
    if (!(self = CcsMalloc(sizeof(KcMenuEntry_t)))) return NULL;
    self->type = type;
    switch (self->type) {
    case KcmetSymbol: self->u.symbol = symbol; break;
    case KcmetSubmenu: self->u.submenu = submenu; break;
    }
    return self;
}

static void
KcMenuEntry_Destruct(KcMenuEntry_t * self)
{
    if (self->type == KcmetSubmenu) KcMenu_Destruct(self->u.submenu);
    CcsFree(self);
}

KcMenu_t *
KcMenu(void)
{
    KcMenu_t * self;
    if (!(self = CcsMalloc(sizeof(KcMenu_t)))) return NULL;
    self->first = self->last = NULL;
    return self;
}

void
KcMenu_Destruct(KcMenu_t * self)
{
    KcMenuEntry_t * cur, * next;
    for (cur = self->first; cur; cur = next) {
	next = cur->next;
	KcMenuEntry_Destruct(cur);
    }
    CcsFree(self);
}

static const char *
KcMenu_AppendEntry(KcMenu_t * self, KcMenuEntry_t * entry)
{
    if (!entry) return "Not enough memory";
    if (self->last) {
	self->last->next = entry; self->last = entry;
    } else {
	self->first = self->last = entry;
    }
    return NULL;
}

const char *
KcMenu_AppendSymbol(KcMenu_t * self, KcSymbol_t * symbol)
{
    return KcMenu_AppendEntry(self, KcMenuEntry(KcmetSymbol, symbol, NULL));
}

const char *
KcMenu_AppendSubmenu(KcMenu_t * self, KcMenu_t * submenu)
{
    return KcMenu_AppendEntry(self, KcMenuEntry(KcmetSubmenu, NULL, submenu));
}
