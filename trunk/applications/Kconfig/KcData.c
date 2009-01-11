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

void
KcPropertyList_Destruct(KcProperty_t * self)
{
    KcProperty_t * next;
    while (self) {
	next = self->next;
	KcProperty_Destruct(self);
	self = next;
    }
}

static const char *
AppendProperty(KcProperty_t ** props, KcProperty_t * prop)
{
    while (*props) props = &((*props)->next);
    *props = prop;
    return NULL;
}

const char *
KcProperty_AppendPrompt(KcProperty_t ** props, char * prompt, KcExpr_t * expr)
{
    KcProperty_t * self;
    if (!(self = CcsMalloc(sizeof(KcProperty_t) + strlen(prompt) + 1)))
	return "Not enough memory";
    memset(self, 0, sizeof(KcProperty_t));
    self->type = KcptPrompt;
    self->prompt = (char *)(self + 1);
    strcpy(self->prompt, prompt);
    return AppendProperty(props, self);
}

const char *
KcProperty_AppendDefault(KcProperty_t ** props,
			 KcExpr_t * expr, KcExpr_t * ifexpr)
{
    KcProperty_t * self;
    if (!(self = CcsMalloc(sizeof(KcProperty_t)))) return "Not enough memory";
    memset(self, 0, sizeof(KcProperty_t));
    self->type = KcptDefault;
    self->expr = expr;
    self->ifexpr = ifexpr;
    return AppendProperty(props, self);
}

const char *
KcProperty_AppendDepends(KcProperty_t ** props, KcExpr_t * expr)
{
    KcProperty_t * self;
    if (!(self = CcsMalloc(sizeof(KcProperty_t)))) return "Not enough memory";
    memset(self, 0, sizeof(KcProperty_t));
    self->type = KcptDepends;
    self->expr = expr;
    return AppendProperty(props, self);
}

const char *
KcProperty_AppendSelect(KcProperty_t ** props, KcSymbol_t * sym,
			KcExpr_t * ifexpr)
{
    KcProperty_t * self;
    if (!(self = CcsMalloc(sizeof(KcProperty_t)))) return "Not enough memory";
    memset(self, 0, sizeof(KcProperty_t));
    self->type = KcptSelect;
    self->sym0 = sym;
    self->ifexpr = ifexpr;
    return AppendProperty(props, self);
}

const char *
KcProperty_AppendRanges(KcProperty_t ** props, KcSymbol_t * sym0,
			KcSymbol_t * sym1, KcExpr_t * ifexpr)
{
    KcProperty_t * self;
    if (!(self = CcsMalloc(sizeof(KcProperty_t)))) return "Not enough memory";
    memset(self, 0, sizeof(KcProperty_t));
    self->type = KcptRanges;
    self->sym0 = sym0;
    self->sym1 = sym1;
    self->ifexpr = ifexpr;
    return AppendProperty(props, self);
}

static KcSymbol_t *
KcSymbol(const char * symname)
{
    KcSymbol_t * self;
    size_t symnamelen = symname ? strlen(symname) + 1 : 0;
    if (!(self = CcsMalloc(sizeof(KcSymbol_t) + symnamelen)))
	return NULL;
    memset(self, 0, sizeof(KcSymbol_t));
    self->type = KcstNone;
    if (symname) {
	self->symname = (char *)(self + 1);
	strcpy(self->symname, symname);
    } else {
	self->symname = NULL;
    }
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
    for (cur = self->props; cur; cur = next) {
	next = cur->next;
	KcProperty_Destruct(cur);
    }
    switch (self->type) {
    case KcstString: if (self->u._string_) CcsFree(self->u._string_);
	break;
    case KcstMenu: if (self->u._menu_) KcSymbolList_Destruct(self->u._menu_);
	break;
    case KcstChoice: if (self->u._choice_) KcSymbolList_Destruct(self->u._choice_);
	break;
    case KcstComment: if (self->u._comment_) CcsFree(self->u._comment_);
	break;
    case KcstIf: if (self->u._ifexpr_) KcExpr_Destruct(self->u._ifexpr_);
	break;
    case KcstConst: if (self->u._const_) CcsFree(self->u._const_);
	break;
    default: break;
    }
    CcsFree(self);
}

#ifndef  KCSIZE_SYMTAB
#define  KCSIZE_SYMTAB  509
#endif

KcSymbolTable_t *
KcSymbolTable(void)
{
    KcSymbolTable_t * self;
    if (!(self = CcsMalloc(sizeof(KcSymbolTable_t)))) goto errquit0;
    self->nonlist = NULL;
    self->constlist = NULL;
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
    KcSymbol_t * cur0, * next0, ** cur;
    for (cur0 = self->nonlist; cur0; cur0 = next0) {
	next0 = cur0->next;
	KcSymbol_Destruct(cur0);
    }
    for (cur0 = self->constlist; cur0; cur0 = next0) {
	next0 = cur0->next;
	KcSymbol_Destruct(cur0);
    }
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

const char *
KcSymbolTable_AppendSymbol(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
			   const char * symname, KcSymbolType_t symtype,
			   CcsBool_t menuOrNot, KcProperty_t * props,
			   CcsPosition_t * helpmsg)
{
    KcSymbol_t ** cur;
    for (cur = symtabHash(self, symname); *cur; cur = &((*cur)->next)) {
	if (strcmp((*cur)->symname, symname)) continue;
	if ((*cur)->type != KcstNone) return "Symbol '%s' is defined already.";
	break;
    }
    if (!*cur && !(*cur = KcSymbol(symname))) return "Not enough memory";
    (*cur)->type = symtype;
    (*cur)->menuOrNot = menuOrNot;
    (*cur)->props = props;
    (*cur)->helpmsg = helpmsg;
    *retSymbol = *cur;
    return NULL;
}

const char *
KcSymbolTable_AddNoNSymbol(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
			   KcSymbolType_t symtype, KcProperty_t * properties)
{
    KcSymbol_t * nonSymbol;
    if (!(nonSymbol = KcSymbol(NULL))) return "Not enough memory";
    nonSymbol->type = symtype;
    nonSymbol->props = properties;
    nonSymbol->next = self->nonlist;
    self->nonlist = nonSymbol;
    *retSymbol = nonSymbol;
    return NULL;
}

const char *
KcSymbolTable_AddConst(KcSymbolTable_t * self, KcSymbol_t ** retSymbol,
		       char * value)
{
    KcSymbol_t * constSymbol;
    if (!(constSymbol = KcSymbol(NULL))) return "Not enough memory";
    constSymbol->type = KcstConst;
    constSymbol->u._const_ = value;
    constSymbol->next = self->constlist;
    self->constlist = constSymbol;
    *retSymbol = constSymbol;
    return NULL;
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

KcSymbolList_t *
KcSymbolList(void)
{
    KcSymbolList_t * self;

    if (!(self = CcsMalloc(sizeof(KcSymbolList_t)))) return NULL;
    self->next = NULL;
    self->symarrUsed = self->symarr;
    return self;
}

void
KcSymbolList_Destruct(KcSymbolList_t * self)
{
    KcSymbolList_t * next;

    while (self) {
	next = self->next;
	CcsFree(self);
	self = next;
    }
}

const char *
KcSymbolList_Append(KcSymbolList_t * self, KcSymbol_t * symbol)
{
    KcSymbolList_t * newlist;
    while (self->next) self = self->next;
    if (self->symarrUsed >= self->symarr + SZ_SYMLISTARR) {
	if (!(newlist = KcSymbolList())) return "Not enough memory";
	self->next = newlist; self = newlist;
    }
    *self->symarrUsed = symbol;
    ++self->symarrUsed;
    return NULL;
}

void
KcSymbolList_Link(KcSymbolList_t * self, KcSymbolList_t * successor)
{
    while (self->next) self = self->next;
    self->next = successor;
}
