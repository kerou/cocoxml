/*---- license ----*/
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
    self->t = self->la;
    for (;;) {
	self->la = KcScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
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

/*---- ProductionsHeader ----*/
/*---- enable ----*/

void
KcParser_Parse(KcParser_t * self)
{
    self->t = NULL;
    self->la = KcScanner_GetDummy(&self->scanner);
    KcParser_Get(self);
    /*---- ParseRoot ----*/
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

KcParser_t *
KcParser(KcParser_t * self, const char * fname, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!KcScanner(&self->scanner, &self->errpool, fname)) goto errquit1;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    /*---- enable ----*/
    return self;
 ERRQUIT:
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
    KcScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
/*---- enable ----*/

static void
KcParser_SynErr(KcParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
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
    /*---- enable ----*/
};
