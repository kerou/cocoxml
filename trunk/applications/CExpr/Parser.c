/*---- license ----*/
/*-------------------------------------------------------------------------
c-expr.atg -- atg for c expression input
Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
Author: Charles Wang <charlesw123456@gmail.com>

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
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void CExprParser_SynErr(CExprParser_t * self, int n);
static const char * set[];

static void
CExprParser_Get(CExprParser_t * self)
{
    if (self->t) CExprScanner_DecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = CExprScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
CExprParser_StartOf(CExprParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
CExprParser_Expect(CExprParser_t * self, int n)
{
    if (self->la->kind == n) CExprParser_Get(self);
    else CExprParser_SynErr(self, n);
}

#ifdef CExprParser_WEAK_USED
static void
CExprParser_ExpectWeak(CExprParser_t * self, int n, int follow)
{
    if (self->la->kind == n) CExprParser_Get(self);
    else {
	CExprParser_SynErr(self, n);
	while (!CExprParser_StartOf(self, follow)) CExprParser_Get(self);
    }
}

static CcsBool_t
CExprParser_WeakSeparator(CExprParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { CExprParser_Get(self); return TRUE; }
    else if (CExprParser_StartOf(self, repFol)) { return FALSE; }
    CExprParser_SynErr(self, n);
    while (!(CExprParser_StartOf(self, syFol) ||
	     CExprParser_StartOf(self, repFol) ||
	     CExprParser_StartOf(self, 0)))
	CExprParser_Get(self);
    return CExprParser_StartOf(self, syFol);
}
#endif /* CExprParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void CExprParser_CExpr(CExprParser_t * self);
/*---- enable ----*/

void
CExprParser_Parse(CExprParser_t * self)
{
    self->t = NULL;
    self->la = CExprScanner_GetDummy(&self->scanner);
    CExprParser_Get(self);
    /*---- ParseRoot ----*/
    CExprParser_CExpr(self);
    /*---- enable ----*/
    CExprParser_Expect(self, 0);
}

void
CExprParser_SemErr(CExprParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, token->line, token->col,
			format, ap);
    va_end(ap);
}

void
CExprParser_SemErrT(CExprParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, self->t->line, self->t->col,
			format, ap);
    va_end(ap);
}

#define ERRQUIT  errquit1
CExprParser_t *
CExprParser(CExprParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!CExprScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 1;
    
    /*---- enable ----*/
    return self;
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
CExprParser_Destruct(CExprParser_t * self)
{
    /*---- destructor ----*/
    
    /*---- enable ----*/
    if (self->la) CExprScanner_DecRef(&self->scanner, self->la);
    if (self->t) CExprScanner_DecRef(&self->scanner, self->t);
    CExprScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
CExprParser_CExpr(CExprParser_t * self)
{
}

/*---- enable ----*/

static void
CExprParser_SynErr(CExprParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "???" "\" expected"; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    CExprParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    "*.."  /* 0 */
    /*---- enable ----*/
};
