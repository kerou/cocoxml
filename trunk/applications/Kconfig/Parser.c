/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void KcParser_SynErr(KcParser_t * self, int n);
static const char * set[];

static void
KcParser_Get(KcParser_t * self)
{
    if (self->t) KcScanner_DecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = KcScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
KcParser_StartOf(KcParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
KcParser_Expect(KcParser_t * self, int n)
{
    if (self->la->kind == n) KcParser_Get(self);
    else KcParser_SynErr(self, n);
}

#ifdef KcParser_WEAK_USED
static void
KcParser_ExpectWeak(KcParser_t * self, int n, int follow)
{
    if (self->la->kind == n) KcParser_Get(self);
    else {
	KcParser_SynErr(self, n);
	while (!KcParser_StartOf(self, follow)) KcParser_Get(self);
    }
}

static CcsBool_t
KcParser_WeakSeparator(KcParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { KcParser_Get(self); return TRUE; }
    else if (KcParser_StartOf(self, repFol)) { return FALSE; }
    KcParser_SynErr(self, n);
    while (!(KcParser_StartOf(self, syFol) ||
	     KcParser_StartOf(self, repFol) ||
	     KcParser_StartOf(self, 0)))
	KcParser_Get(self);
    return KcParser_StartOf(self, syFol);
}
#endif /* KcParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void KcParser_Kconfig(KcParser_t * self);
static void KcParser_SymbolListDecl(KcParser_t * self, KcSymbolList_t ** symlist);
static void KcParser_SymbolDecl(KcParser_t * self, KcSymbol_t ** sym);
static void KcParser_ConfigDecl(KcParser_t * self, KcSymbol_t ** sym);
static void KcParser_MenuDecl(KcParser_t * self, KcSymbol_t ** menu);
static void KcParser_ChoiceDecl(KcParser_t * self, KcSymbol_t ** choice);
static void KcParser_CommentDecl(KcParser_t * self, KcSymbol_t ** comment);
static void KcParser_IfDecl(KcParser_t * self, KcSymbol_t ** _if_);
static void KcParser_MainMenuDecl(KcParser_t * self, KcSymbol_t ** sym);
static void KcParser_Property(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop);
static void KcParser_Help(KcParser_t * self, CcsPosition_t ** pos);
static void KcParser_Expr(KcParser_t * self, KcExpr_t ** expr);
static void KcParser_TypeDefine(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop);
static void KcParser_InputPrompt(KcParser_t * self, KcProperty_t ** prop);
static void KcParser_Default(KcParser_t * self, KcProperty_t ** prop);
static void KcParser_TypeWithDefault(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop);
static void KcParser_DependsOn(KcParser_t * self, KcProperty_t ** prop);
static void KcParser_Select(KcParser_t * self, KcProperty_t ** prop);
static void KcParser_Ranges(KcParser_t * self, KcProperty_t ** prop);
static void KcParser_Symbol(KcParser_t * self, KcSymbol_t ** sym);
static void KcParser_Expr0(KcParser_t * self, KcExpr_t ** expr);
static void KcParser_Expr1(KcParser_t * self, KcExpr_t ** expr);
static void KcParser_Expr2(KcParser_t * self, KcExpr_t ** expr);
static void KcParser_Expr3(KcParser_t * self, KcExpr_t ** expr);
/*---- enable ----*/

void
KcParser_Parse(KcParser_t * self)
{
    self->t = NULL;
    self->la = KcScanner_GetDummy(&self->scanner);
    KcParser_Get(self);
    /*---- ParseRoot ----*/
    KcParser_Kconfig(self);
    /*---- enable ----*/
    KcParser_Expect(self, 0);
}

void
KcParser_SemErr(KcParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, token->line, token->col,
			format, ap);
    va_end(ap);
}

void
KcParser_SemErrT(KcParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, self->t->line, self->t->col,
			format, ap);
    va_end(ap);
}

static CcsBool_t
KcParser_Init(KcParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 38;
    self->mainmenu = NULL;
    if (!(self->symtab = KcSymbolTable())) return FALSE;
    self->toplist = NULL;
    /*---- enable ----*/
    return TRUE;
}

KcParser_t *
KcParser(KcParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!KcScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!KcParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    KcScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

KcParser_t *
KcParser_ByName(KcParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!KcScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!KcParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    KcScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
KcParser_Destruct(KcParser_t * self)
{
    /*---- destructor ----*/
    if (self->toplist)  KcSymbolList_Destruct(self->toplist);
    KcSymbolTable_Destruct(self->symtab);
    if (self->mainmenu)  CcsFree(self->mainmenu);
    /*---- enable ----*/
    if (self->la) KcScanner_DecRef(&self->scanner, self->la);
    if (self->t) KcScanner_DecRef(&self->scanner, self->t);
    KcScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
KcParser_Kconfig(KcParser_t * self)
{
    KcSymbolList_t * toplist = NULL; 
    KcParser_SymbolListDecl(self, &toplist);
    self->toplist = toplist; 
}

static void
KcParser_SymbolListDecl(KcParser_t * self, KcSymbolList_t ** symlist)
{
    KcSymbol_t * sym; 
    while (KcParser_StartOf(self, 1)) {
	KcParser_SymbolDecl(self, &sym);
	if (*symlist == NULL) *symlist = KcSymbolList();
	if (sym) KcSymbolList_Append(*symlist, sym); 
    }
}

static void
KcParser_SymbolDecl(KcParser_t * self, KcSymbol_t ** sym)
{
    switch (self->la->kind) {
    case 7: case 8: {
	KcParser_ConfigDecl(self, sym);
	break;
    }
    case 9: {
	KcParser_MenuDecl(self, sym);
	break;
    }
    case 11: {
	KcParser_ChoiceDecl(self, sym);
	break;
    }
    case 13: {
	KcParser_CommentDecl(self, sym);
	break;
    }
    case 14: {
	KcParser_IfDecl(self, sym);
	break;
    }
    case 16: {
	KcParser_MainMenuDecl(self, sym);
	break;
    }
    default: KcParser_SynErr(self, 39); break;
    }
    while (self->la->kind == 6) {
	KcParser_Get(self);
    }
}

static void
KcParser_ConfigDecl(KcParser_t * self, KcSymbol_t ** sym)
{
    const char * errmsg;
    CcsBool_t menuOrNot;
    char * symname = NULL;
    KcSymbolType_t symtype = KcstNone;
    KcProperty_t * props = NULL;
    CcsPosition_t * helpmsg = NULL; 
    if (self->la->kind == 7) {
	KcParser_Get(self);
	menuOrNot = FALSE; 
    } else if (self->la->kind == 8) {
	KcParser_Get(self);
	menuOrNot = TRUE; 
    } else KcParser_SynErr(self, 40);
    KcParser_Expect(self, 4);
    symname = CcsStrdup(self->t->val); 
    KcParser_Expect(self, 6);
    KcParser_Expect(self, 1);
    while (KcParser_StartOf(self, 2)) {
	KcParser_Property(self, &symtype, &props);
    }
    if (self->la->kind == 29 || self->la->kind == 30) {
	KcParser_Help(self, &helpmsg);
    }
    while (self->la->kind == 6) {
	KcParser_Get(self);
    }
    KcParser_Expect(self, 2);
    if ((errmsg = KcSymbolTable_AppendSymbol(self->symtab, sym, symname,
					     symtype, menuOrNot, props, helpmsg))) {
	if (props) KcPropertyList_Destruct(props);
	if (helpmsg) CcsPosition_Destruct(helpmsg);
    }
    CcsFree(symname); 
}

static void
KcParser_MenuDecl(KcParser_t * self, KcSymbol_t ** menu)
{
    const char * errmsg;
    char * prompt;
    KcSymbolType_t symtype;
    KcProperty_t * props = NULL;
    KcSymbolList_t * symlist = NULL; 
    KcParser_Expect(self, 9);
    KcParser_Expect(self, 5);
    prompt = CcsEscape(self->t->val);
    KcProperty_AppendPrompt(&props, prompt, NULL); 
    KcParser_Expect(self, 6);
    while (self->la->kind == 6) {
	KcParser_Get(self);
    }
    if (self->la->kind == 1) {
	KcParser_Get(self);
	while (KcParser_StartOf(self, 2)) {
	    KcParser_Property(self, &symtype, &props);
	}
	while (self->la->kind == 6) {
	    KcParser_Get(self);
	}
	KcParser_Expect(self, 2);
    }
    KcParser_SymbolListDecl(self, &symlist);
    KcParser_Expect(self, 10);
    KcParser_Expect(self, 6);
    if ((errmsg = KcSymbolTable_AddNoNSymbol(self->symtab, menu,
					     KcstMenu, props))) {
	KcParser_SemErrT(self, "Append 'menu' failed: %s.", errmsg);
	if (props) KcPropertyList_Destruct(props);
	if (symlist) KcSymbolList_Destruct(symlist);
    } else (*menu)->u._menu_ = symlist; 
}

static void
KcParser_ChoiceDecl(KcParser_t * self, KcSymbol_t ** choice)
{
    const char * errmsg;
    KcSymbolType_t symtype;
    KcProperty_t * props = NULL;
    KcSymbolList_t * symlist = NULL; 
    KcParser_Expect(self, 11);
    KcParser_Expect(self, 6);
    while (self->la->kind == 6) {
	KcParser_Get(self);
    }
    if (self->la->kind == 1) {
	KcParser_Get(self);
	while (KcParser_StartOf(self, 2)) {
	    KcParser_Property(self, &symtype, &props);
	}
	while (self->la->kind == 6) {
	    KcParser_Get(self);
	}
	KcParser_Expect(self, 2);
    }
    KcParser_SymbolListDecl(self, &symlist);
    KcParser_Expect(self, 12);
    KcParser_Expect(self, 6);
    if ((errmsg = KcSymbolTable_AddNoNSymbol(self->symtab, choice,
					     KcstChoice, props))) {
	KcParser_SemErrT(self, "Append 'choice' failed: %s.", errmsg);
	if (props) KcPropertyList_Destruct(props);
	if (symlist) KcSymbolList_Destruct(symlist);
    } else (*choice)->u._choice_ = symlist; 
}

static void
KcParser_CommentDecl(KcParser_t * self, KcSymbol_t ** comment)
{
    const char * errmsg;
    char * _comment_;
    KcSymbolType_t symtype;
    KcProperty_t * props = NULL; 
    KcParser_Expect(self, 13);
    KcParser_Expect(self, 5);
    _comment_ = CcsEscape(self->t->val); 
    while (KcParser_StartOf(self, 2)) {
	KcParser_Property(self, &symtype, &props);
    }
    KcParser_Expect(self, 6);
    if ((errmsg = KcSymbolTable_AddNoNSymbol(self->symtab, comment,
					     KcstComment, props))) {
	KcParser_SemErrT(self, "Append 'comment' failed: %s.", errmsg);
	if (_comment_) CcsFree(_comment_);
	if (props) KcPropertyList_Destruct(props);
    } else (*comment)->u._comment_ = _comment_; 
}

static void
KcParser_IfDecl(KcParser_t * self, KcSymbol_t ** _if_)
{
    const char * errmsg;
    KcExpr_t * ifexpr = NULL;
    KcSymbolList_t * symlist = NULL; 
    KcParser_Expect(self, 14);
    KcParser_Expr(self, &ifexpr);
    KcParser_Expect(self, 6);
    while (self->la->kind == 6) {
	KcParser_Get(self);
    }
    KcParser_SymbolListDecl(self, &symlist);
    KcParser_Expect(self, 15);
    KcParser_Expect(self, 6);
    if ((errmsg = KcSymbolTable_AddNoNSymbol(self->symtab, _if_,
					     KcstIf, NULL))) {
	KcParser_SemErrT(self, "Append 'if' failed: %s.", errmsg);
	if (ifexpr) KcExpr_Destruct(ifexpr);
    } else (*_if_)->u._ifexpr_ = ifexpr; 
}

static void
KcParser_MainMenuDecl(KcParser_t * self, KcSymbol_t ** sym)
{
    KcParser_Expect(self, 16);
    KcParser_Expect(self, 5);
    if (!self->mainmenu) self->mainmenu = CcsEscape(self->t->val); 
    KcParser_Expect(self, 6);
}

static void
KcParser_Property(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop)
{
    switch (self->la->kind) {
    case 5: case 17: case 18: case 19: case 20: {
	KcParser_TypeDefine(self, symtype, prop);
	break;
    }
    case 21: {
	KcParser_InputPrompt(self, prop);
	break;
    }
    case 22: {
	KcParser_Default(self, prop);
	break;
    }
    case 23: case 24: {
	KcParser_TypeWithDefault(self, symtype, prop);
	break;
    }
    case 25: {
	KcParser_DependsOn(self, prop);
	break;
    }
    case 27: {
	KcParser_Select(self, prop);
	break;
    }
    case 28: {
	KcParser_Ranges(self, prop);
	break;
    }
    default: KcParser_SynErr(self, 41); break;
    }
}

static void
KcParser_Help(KcParser_t * self, CcsPosition_t ** pos)
{
    CcsToken_t * beg; 
    if (self->la->kind == 29) {
	KcParser_Get(self);
    } else if (self->la->kind == 30) {
	KcParser_Get(self);
    } else KcParser_SynErr(self, 42);
    KcParser_Expect(self, 6);
    KcParser_Expect(self, 1);
    KcScanner_IndentLimit(&self->scanner, self->t);
    KcScanner_IncRef(&self->scanner, beg = self->la); 
    while (KcParser_StartOf(self, 3)) {
	KcParser_Get(self);
    }
    KcParser_Expect(self, 2);
    *pos = KcScanner_GetPosition(&self->scanner, beg, self->t);
    KcScanner_DecRef(&self->scanner, beg); 
}

static void
KcParser_Expr(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    KcParser_Expr0(self, expr);
    while (self->la->kind == 31) {
	KcParser_Get(self);
	KcParser_Expr0(self, &expr0);
	*expr = KcExpr(KcetExprOr, NULL, NULL, *expr, expr0); 
    }
}

static void
KcParser_TypeDefine(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop)
{
    char * prompt = NULL; KcExpr_t * expr = NULL; 
    if (self->la->kind == 17) {
	KcParser_Get(self);
	*symtype = KcstBool; 
    } else if (self->la->kind == 18) {
	KcParser_Get(self);
	*symtype = KcstTristate; 
    } else if (self->la->kind == 5) {
	KcParser_Get(self);
	*symtype = KcstString; 
    } else if (self->la->kind == 19) {
	KcParser_Get(self);
	*symtype = KcstHex; 
    } else if (self->la->kind == 20) {
	KcParser_Get(self);
	*symtype = KcstInt; 
    } else KcParser_SynErr(self, 43);
    if (self->la->kind == 5) {
	KcParser_Get(self);
	prompt = CcsEscape(self->t->val); 
	if (self->la->kind == 14) {
	    KcParser_Get(self);
	    KcParser_Expr(self, &expr);
	}
	KcProperty_AppendPrompt(prop, prompt, expr); 
    }
    KcParser_Expect(self, 6);
}

static void
KcParser_InputPrompt(KcParser_t * self, KcProperty_t ** prop)
{
    char * prompt; KcExpr_t * expr = NULL; 
    KcParser_Expect(self, 21);
    KcParser_Expect(self, 5);
    prompt = CcsStrdup(self->t->val); 
    if (self->la->kind == 14) {
	KcParser_Get(self);
	KcParser_Expr(self, &expr);
    }
    KcParser_Expect(self, 6);
    KcProperty_AppendPrompt(prop, prompt, expr); 
}

static void
KcParser_Default(KcParser_t * self, KcProperty_t ** prop)
{
    KcExpr_t * expr0, * expr1 = NULL; 
    KcParser_Expect(self, 22);
    KcParser_Expr(self, &expr0);
    if (self->la->kind == 14) {
	KcParser_Get(self);
	KcParser_Expr(self, &expr1);
    }
    KcParser_Expect(self, 6);
    KcProperty_AppendDefault(prop, expr0, expr1); 
}

static void
KcParser_TypeWithDefault(KcParser_t * self, KcSymbolType_t * symtype, KcProperty_t ** prop)
{
    KcExpr_t * expr0, * expr1  = NULL; 
    if (self->la->kind == 23) {
	KcParser_Get(self);
	*symtype = KcstBool; 
    } else if (self->la->kind == 24) {
	KcParser_Get(self);
	*symtype = KcstTristate; 
    } else KcParser_SynErr(self, 44);
    KcParser_Expr(self, &expr0);
    if (self->la->kind == 14) {
	KcParser_Get(self);
	KcParser_Expr(self, &expr1);
    }
    KcParser_Expect(self, 6);
    KcProperty_AppendDefault(prop, expr0, expr1); 
}

static void
KcParser_DependsOn(KcParser_t * self, KcProperty_t ** prop)
{
    KcExpr_t * expr; 
    KcParser_Expect(self, 25);
    KcParser_Expect(self, 26);
    KcParser_Expr(self, &expr);
    KcParser_Expect(self, 6);
    KcProperty_AppendDepends(prop, expr); 
}

static void
KcParser_Select(KcParser_t * self, KcProperty_t ** prop)
{
    KcSymbol_t * sym; KcExpr_t * expr = NULL; 
    KcParser_Expect(self, 27);
    KcParser_Symbol(self, &sym);
    if (self->la->kind == 14) {
	KcParser_Get(self);
	KcParser_Expr(self, &expr);
    }
    KcParser_Expect(self, 6);
    KcProperty_AppendSelect(prop, sym, expr); 
}

static void
KcParser_Ranges(KcParser_t * self, KcProperty_t ** prop)
{
    KcSymbol_t * sym0, * sym1; KcExpr_t * expr = NULL; 
    KcParser_Expect(self, 28);
    KcParser_Symbol(self, &sym0);
    KcParser_Symbol(self, &sym1);
    if (self->la->kind == 14) {
	KcParser_Get(self);
	KcParser_Expr(self, &expr);
    }
    KcParser_Expect(self, 6);
    KcProperty_AppendRanges(prop, sym0, sym1, expr); 
}

static void
KcParser_Symbol(KcParser_t * self, KcSymbol_t ** sym)
{
    KcParser_Expect(self, 4);
    *sym = KcSymbolTable_Get(self->symtab, self->t->val); 
}

static void
KcParser_Expr0(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    KcParser_Expr1(self, expr);
    while (self->la->kind == 32) {
	KcParser_Get(self);
	KcParser_Expr1(self, &expr0);
	*expr = KcExpr(KcetExprAnd, NULL, NULL, *expr, expr0); 
    }
}

static void
KcParser_Expr1(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    if (self->la->kind == 4 || self->la->kind == 34) {
	KcParser_Expr2(self, expr);
    } else if (self->la->kind == 33) {
	KcParser_Get(self);
	KcParser_Expr2(self, &expr0);
	*expr = KcExpr(KcetNotExpr, NULL, NULL, expr0, NULL); 
    } else KcParser_SynErr(self, 45);
}

static void
KcParser_Expr2(KcParser_t * self, KcExpr_t ** expr)
{
    if (self->la->kind == 4) {
	KcParser_Expr3(self, expr);
    } else if (self->la->kind == 34) {
	KcParser_Get(self);
	KcParser_Expr(self, expr);
	KcParser_Expect(self, 35);
    } else KcParser_SynErr(self, 46);
}

static void
KcParser_Expr3(KcParser_t * self, KcExpr_t ** expr)
{
    char op = 0; KcSymbol_t * sym0 = NULL, * sym1 = NULL; 
    KcParser_Symbol(self, &sym0);
    if (self->la->kind == 36 || self->la->kind == 37) {
	if (self->la->kind == 36) {
	    KcParser_Get(self);
	    op = '='; 
	} else {
	    KcParser_Get(self);
	    op = '!'; 
	}
	KcParser_Symbol(self, &sym1);
    }
    switch (op) {
    case 0: *expr = KcExpr(KcetSymbol, sym0, NULL, NULL, NULL); break;
    case '=': *expr = KcExpr(KcetSymbolEqual, sym0, sym1, NULL, NULL); break;
    case '!': *expr = KcExpr(KcetSymbolNotEqual, sym0, sym1, NULL, NULL); break;
    } 
}

/*---- enable ----*/

static void
KcParser_SynErr(KcParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "IndentIn" "\" expected"; break;
    case 2: s = "\"" "IndentOut" "\" expected"; break;
    case 3: s = "\"" "IndentErr" "\" expected"; break;
    case 4: s = "\"" "ident" "\" expected"; break;
    case 5: s = "\"" "string" "\" expected"; break;
    case 6: s = "\"" "eol" "\" expected"; break;
    case 7: s = "\"" "config" "\" expected"; break;
    case 8: s = "\"" "menuconfig" "\" expected"; break;
    case 9: s = "\"" "menu" "\" expected"; break;
    case 10: s = "\"" "endmenu" "\" expected"; break;
    case 11: s = "\"" "choice" "\" expected"; break;
    case 12: s = "\"" "endchoice" "\" expected"; break;
    case 13: s = "\"" "comment" "\" expected"; break;
    case 14: s = "\"" "if" "\" expected"; break;
    case 15: s = "\"" "endif" "\" expected"; break;
    case 16: s = "\"" "mainmenu" "\" expected"; break;
    case 17: s = "\"" "bool" "\" expected"; break;
    case 18: s = "\"" "tristate" "\" expected"; break;
    case 19: s = "\"" "hex" "\" expected"; break;
    case 20: s = "\"" "int" "\" expected"; break;
    case 21: s = "\"" "prompt" "\" expected"; break;
    case 22: s = "\"" "default" "\" expected"; break;
    case 23: s = "\"" "def_bool" "\" expected"; break;
    case 24: s = "\"" "def_tristate" "\" expected"; break;
    case 25: s = "\"" "depends" "\" expected"; break;
    case 26: s = "\"" "on" "\" expected"; break;
    case 27: s = "\"" "select" "\" expected"; break;
    case 28: s = "\"" "ranges" "\" expected"; break;
    case 29: s = "\"" "help" "\" expected"; break;
    case 30: s = "\"" "---help---" "\" expected"; break;
    case 31: s = "\"" "||" "\" expected"; break;
    case 32: s = "\"" "&&" "\" expected"; break;
    case 33: s = "\"" "!" "\" expected"; break;
    case 34: s = "\"" "(" "\" expected"; break;
    case 35: s = "\"" ")" "\" expected"; break;
    case 36: s = "\"" "=" "\" expected"; break;
    case 37: s = "\"" "!=" "\" expected"; break;
    case 38: s = "\"" "???" "\" expected"; break;
    case 39: s = "this symbol not expected in \"" "SymbolDecl" "\""; break;
    case 40: s = "this symbol not expected in \"" "ConfigDecl" "\""; break;
    case 41: s = "this symbol not expected in \"" "Property" "\""; break;
    case 42: s = "this symbol not expected in \"" "Help" "\""; break;
    case 43: s = "this symbol not expected in \"" "TypeDefine" "\""; break;
    case 44: s = "this symbol not expected in \"" "TypeWithDefault" "\""; break;
    case 45: s = "this symbol not expected in \"" "Expr1" "\""; break;
    case 46: s = "this symbol not expected in \"" "Expr2" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    KcParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    5    0    5    0    5    */
    "*.......................................", /* 0 */
    ".......***.*.**.*.......................", /* 1 */
    ".....*...........*********.**...........", /* 2 */
    ".*.************************************."  /* 3 */
    /*---- enable ----*/
};
