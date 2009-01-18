/*---- license ----*/
/*-------------------------------------------------------------------------
 patch.atg
 Copyright (C) 2008, Charles Wang
 Author: Charles Wang  <charlesw123456@gmail.com>
 License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void PatchParser_SynErr(PatchParser_t * self, int n);
static const char * set[];

static void
PatchParser_Get(PatchParser_t * self)
{
    if (self->t) PatchScanner_TokenDecRef(&self->scanner, self->t);
    self->t = self->la;
    for (;;) {
	self->la = PatchScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/* May be implement pragmas here is wrong... But I still not found any
	 * needs to use pragmas, so just leave it along. */
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
PatchParser_StartOf(PatchParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
PatchParser_Expect(PatchParser_t * self, int n)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else PatchParser_SynErr(self, n);
}

#ifdef PatchParser_WEAK_USED
static void
PatchParser_ExpectWeak(PatchParser_t * self, int n, int follow)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else {
	PatchParser_SynErr(self, n);
	while (!PatchParser_StartOf(self, follow)) PatchParser_Get(self);
    }
}

static CcsBool_t
PatchParser_WeakSeparator(PatchParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { PatchParser_Get(self); return TRUE; }
    else if (PatchParser_StartOf(self, repFol)) { return FALSE; }
    PatchParser_SynErr(self, n);
    while (!(PatchParser_StartOf(self, syFol) ||
	     PatchParser_StartOf(self, repFol) ||
	     PatchParser_StartOf(self, 0)))
	PatchParser_Get(self);
    return PatchParser_StartOf(self, syFol);
}
#endif /* PatchParser_WEAK_USED */

/*---- ProductionsHeader ----*/
static void PatchParser_Patch(PatchParser_t * self);
static void PatchParser_FilePatch(PatchParser_t * self);
static void PatchParser_HeadLine(PatchParser_t * self);
static void PatchParser_FileSubLine(PatchParser_t * self);
static void PatchParser_FileAddLine(PatchParser_t * self);
static void PatchParser_Piece(PatchParser_t * self);
static void PatchParser_PieceLine(PatchParser_t * self);
static void PatchParser_AddLine(PatchParser_t * self);
static void PatchParser_SubLine(PatchParser_t * self);
static void PatchParser_SameLine(PatchParser_t * self);
static void PatchParser_NoNewLine(PatchParser_t * self);
/*---- enable ----*/

void
PatchParser_Parse(PatchParser_t * self)
{
    self->t = NULL;
    self->la = PatchScanner_GetDummy(&self->scanner);
    PatchParser_Get(self);
    /*---- ParseRoot ----*/
    PatchParser_Patch(self);
    /*---- enable ----*/
    PatchParser_Expect(self, 0);
}

void
PatchParser_SemErr(PatchParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &token->loc, format, ap);
    va_end(ap);
}

void
PatchParser_SemErrT(PatchParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, &self->t->loc, format, ap);
    va_end(ap);
}

static CcsBool_t
PatchParser_Init(PatchParser_t * self)
{
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 10;
    
    /*---- enable ----*/
    return TRUE;
}

PatchParser_t *
PatchParser(PatchParser_t * self, FILE  * infp, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PatchScanner(&self->scanner, &self->errpool, infp)) goto errquit1;
    if (!PatchParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PatchScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

PatchParser_t *
PatchParser_ByName(PatchParser_t * self, const char * infn, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PatchScanner_ByName(&self->scanner, &self->errpool, infn))
	goto errquit1;
    if (!PatchParser_Init(self)) goto errquit2;
    return self;
 errquit2:
    PatchScanner_Destruct(&self->scanner);
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
PatchParser_Destruct(PatchParser_t * self)
{
    /*---- destructor ----*/
    
    /*---- enable ----*/
    if (self->la) PatchScanner_TokenDecRef(&self->scanner, self->la);
    if (self->t) PatchScanner_TokenDecRef(&self->scanner, self->t);
    PatchScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
PatchParser_Patch(PatchParser_t * self)
{
    PatchParser_FilePatch(self);
    while (self->la->kind == 1 || self->la->kind == 4) {
	PatchParser_FilePatch(self);
    }
}

static void
PatchParser_FilePatch(PatchParser_t * self)
{
    while (self->la->kind == 1) {
	PatchParser_HeadLine(self);
    }
    PatchParser_FileSubLine(self);
    PatchParser_FileAddLine(self);
    PatchParser_Piece(self);
    while (self->la->kind == 7) {
	PatchParser_Piece(self);
    }
}

static void
PatchParser_HeadLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 1);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_FileSubLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 4);
    PatchParser_Expect(self, 4);
    PatchParser_Expect(self, 4);
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_FileAddLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 6);
    PatchParser_Expect(self, 6);
    PatchParser_Expect(self, 6);
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_Piece(PatchParser_t * self)
{
    PatchParser_PieceLine(self);
    while (PatchParser_StartOf(self, 2)) {
	if (self->la->kind == 6) {
	    PatchParser_AddLine(self);
	} else if (self->la->kind == 4) {
	    PatchParser_SubLine(self);
	} else if (self->la->kind == 5) {
	    PatchParser_SameLine(self);
	} else {
	    PatchParser_NoNewLine(self);
	}
    }
}

static void
PatchParser_PieceLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 7);
    while (self->la->kind == 5) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 4);
    PatchParser_Expect(self, 2);
    if (self->la->kind == 8) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
    }
    while (self->la->kind == 5) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 6);
    PatchParser_Expect(self, 2);
    if (self->la->kind == 8) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
    }
    while (self->la->kind == 5) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 7);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_AddLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 6);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_SubLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 4);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_SameLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

static void
PatchParser_NoNewLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 9);
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 3);
}

/*---- enable ----*/

static void
PatchParser_SynErr(PatchParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "infoch" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "eol" "\" expected"; break;
    case 4: s = "\"" "-" "\" expected"; break;
    case 5: s = "\"" " " "\" expected"; break;
    case 6: s = "\"" "+" "\" expected"; break;
    case 7: s = "\"" "@@" "\" expected"; break;
    case 8: s = "\"" "," "\" expected"; break;
    case 9: s = "\"" "\\" "\" expected"; break;
    case 10: s = "\"" "???" "\" expected"; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    PatchParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0 */
    "*...........", /* 0 */
    ".**.*******.", /* 1 */
    "....***..*.."  /* 2 */
    /*---- enable ----*/
};
