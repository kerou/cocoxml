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
static void KcParser_ConfigDecl(KcParser_t * self);
static void KcParser_ConfigProperty(KcParser_t * self);
static void KcParser_Help(KcParser_t * self);
static void KcParser_TypeDefine(KcParser_t * self);
static void KcParser_TypeWithDefault(KcParser_t * self);
static void KcParser_Expr(KcParser_t * self, KcExpr_t ** expr);
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

#define ERRQUIT  errquit1
KcParser_t *
KcParser(KcParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!KcScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 24;
    if (!(self->symtab = KcSymbolTable(4409))) goto ERRQUIT;
    /*---- enable ----*/
    return self;
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
KcParser_Destruct(KcParser_t * self)
{
    /*---- destructor ----*/
    KcSymbolTable_Destruct(self->symtab);
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
    while (self->la->kind == 7) {
	KcParser_ConfigDecl(self);
    }
}

static void
KcParser_ConfigDecl(KcParser_t * self)
{
    KcParser_Expect(self, 7);
    KcParser_Expect(self, 4);
    KcParser_Expect(self, 1);
    while (KcParser_StartOf(self, 1)) {
	KcParser_ConfigProperty(self);
    }
    if (self->la->kind == 16) {
	KcParser_Help(self);
    }
    KcParser_Expect(self, 2);
}

static void
KcParser_ConfigProperty(KcParser_t * self)
{
    if (self->la->kind == 8 || self->la->kind == 9) {
	KcParser_TypeDefine(self);
    } else if (self->la->kind == 10 || self->la->kind == 13) {
	KcParser_TypeWithDefault(self);
    } else KcParser_SynErr(self, 25);
}

static void
KcParser_Help(KcParser_t * self)
{
    KcParser_Expect(self, 16);
    KcParser_Expect(self, 1);
    while (KcParser_StartOf(self, 2)) {
	KcParser_Get(self);
    }
    KcParser_Expect(self, 2);
}

static void
KcParser_TypeDefine(KcParser_t * self)
{
    if (self->la->kind == 8) {
	KcParser_Get(self);
	if (self->la->kind == 5) {
	    KcParser_Get(self);
	}
    } else if (self->la->kind == 9) {
	KcParser_Get(self);
	if (self->la->kind == 5) {
	    KcParser_Get(self);
	}
    } else KcParser_SynErr(self, 26);
}

static void
KcParser_TypeWithDefault(KcParser_t * self)
{
    if (self->la->kind == 10) {
	KcExpr_t * expr; 
	KcParser_Get(self);
	if (self->la->kind == 11) {
	    KcParser_Get(self);
	} else if (self->la->kind == 12) {
	    KcParser_Get(self);
	} else KcParser_SynErr(self, 27);
    } else if (self->la->kind == 13) {
	KcParser_Get(self);
	if (self->la->kind == 11) {
	    KcParser_Get(self);
	} else if (self->la->kind == 14) {
	    KcParser_Get(self);
	} else if (self->la->kind == 12) {
	    KcParser_Get(self);
	} else KcParser_SynErr(self, 28);
	if (self->la->kind == 15) {
	    KcParser_Get(self);
	    KcParser_Expr(self, &expr);
	}
    } else KcParser_SynErr(self, 29);
}

static void
KcParser_Expr(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    KcParser_Expr0(self, expr);
    while (self->la->kind == 17) {
	KcParser_Get(self);
	KcParser_Expr0(self, &expr0);
	*expr = KcExpr(KcetExprOr, NULL, NULL, *expr, expr0); 
    }
}

static void
KcParser_Expr0(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    KcParser_Expr1(self, expr);
    while (self->la->kind == 18) {
	KcParser_Get(self);
	KcParser_Expr1(self, &expr0);
	*expr = KcExpr(KcetExprAnd, NULL, NULL, *expr, expr0); 
    }
}

static void
KcParser_Expr1(KcParser_t * self, KcExpr_t ** expr)
{
    KcExpr_t * expr0; 
    KcParser_Expect(self, 19);
    KcParser_Expr2(self, &expr0);
    *expr = KcExpr(KcetNotExpr, NULL, NULL, expr0, NULL); 
}

static void
KcParser_Expr2(KcParser_t * self, KcExpr_t ** expr)
{
    KcParser_Expect(self, 20);
    KcParser_Expr3(self, expr);
    KcParser_Expect(self, 21);
}

static void
KcParser_Expr3(KcParser_t * self, KcExpr_t ** expr)
{
    if (self->la->kind == 4) {
	char op = 0; KcSymbol_t * sym0 = NULL, * sym1 = NULL; 
	KcParser_Get(self);
	sym0 = KcSymbolTable_Get(self->symtab, self->t->val); 
	if (self->la->kind == 22 || self->la->kind == 23) {
	    if (self->la->kind == 22) {
		KcParser_Get(self);
		op = '='; 
	    } else {
		KcParser_Get(self);
		op = '!'; 
	    }
	    KcParser_Expect(self, 4);
	    sym1 = KcSymbolTable_Get(self->symtab, self->t->val); 
	}
	switch (op) {
	case 0: *expr = KcExpr(KcetSymbol, sym0, NULL, NULL, NULL); break;
	case '=': *expr = KcExpr(KcetSymbolEqual, sym0, sym1, NULL, NULL); break;
	case '!': *expr = KcExpr(KcetSymbolNotEqual, sym0, sym1, NULL, NULL); break;
	} 
    } else if (self->la->kind == 19) {
	KcParser_Expr(self, expr);
    } else KcParser_SynErr(self, 30);
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
    case 8: s = "\"" "bool" "\" expected"; break;
    case 9: s = "\"" "tristate" "\" expected"; break;
    case 10: s = "\"" "def_bool" "\" expected"; break;
    case 11: s = "\"" "y" "\" expected"; break;
    case 12: s = "\"" "n" "\" expected"; break;
    case 13: s = "\"" "def_tristate" "\" expected"; break;
    case 14: s = "\"" "m" "\" expected"; break;
    case 15: s = "\"" "if" "\" expected"; break;
    case 16: s = "\"" "help" "\" expected"; break;
    case 17: s = "\"" "||" "\" expected"; break;
    case 18: s = "\"" "&&" "\" expected"; break;
    case 19: s = "\"" "!" "\" expected"; break;
    case 20: s = "\"" "(" "\" expected"; break;
    case 21: s = "\"" ")" "\" expected"; break;
    case 22: s = "\"" "=" "\" expected"; break;
    case 23: s = "\"" "!=" "\" expected"; break;
    case 24: s = "\"" "???" "\" expected"; break;
    case 25: s = "this symbol not expected in \"" "ConfigProperty" "\""; break;
    case 26: s = "this symbol not expected in \"" "TypeDefine" "\""; break;
    case 27: s = "this symbol not expected in \"" "TypeWithDefault" "\""; break;
    case 28: s = "this symbol not expected in \"" "TypeWithDefault" "\""; break;
    case 29: s = "this symbol not expected in \"" "TypeWithDefault" "\""; break;
    case 30: s = "this symbol not expected in \"" "Expr3" "\""; break;
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
    /*    5    0    5    0     */
    "*.........................", /* 0 */
    "........***..*............", /* 1 */
    ".*.**********************."  /* 2 */
    /*---- enable ----*/
};
