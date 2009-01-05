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
    self->maxT = 7;
    
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
    case 7: s = "\"" "???" "\" expected"; break;
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
    /*    5   */
    "*........"  /* 0 */
    /*---- enable ----*/
};
