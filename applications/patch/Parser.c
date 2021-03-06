/*---- license ----*/
/*-------------------------------------------------------------------------
  patch.atg -- atg for patch.
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang  <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  "Parser.h"
#include  "c/Token.h"
#if defined(PatchParser_USE_GetSS)
#include  "c/ScanInput.h"
#endif

/*---- cIncludes ----*/
/*---- enable ----*/

static void PatchParser_SynErr(PatchParser_t * self, int n);
#ifdef PatchParser_USE_StartOf
static const char * set[];
#endif /* PatchParser_USE_StartOf */

#if defined(PatchParser_USE_GetSS) || defined(PatchParser_USE_ExpectSS)
typedef CcsToken_t *
(* SubScanner_t)(PatchParser_t * self, const char * fname,
		 int pos, int line, int col);

static void
PatchParser_TokenIncRef(PatchParser_t * self, CcsToken_t * token)
{
    if (token->destructor) ++token->refcnt;
    else PatchScanner_TokenIncRef(&self->scanner, token);
}

static void
PatchParser_TokenDecRef(PatchParser_t * self, CcsToken_t * token)
{
    if (token->destructor) token->destructor(token);
    else PatchScanner_TokenDecRef(&self->scanner, token);
}
#else

#define PatchParser_TokenIncRef(self, token) \
    PatchScanner_TokenIncRef(&((self)->scanner), token)
#define PatchParser_TokenDecRef(self, token) \
    PatchScanner_TokenDecRef(&((self)->scanner), token)

#endif /* PatchParser_USE_GetSS || PatchParser_USE_ExpectSS */

static void
PatchParser_Get(PatchParser_t * self)
{
    if (self->t) PatchParser_TokenDecRef(self, self->t);
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

#ifdef PatchParser_USE_StartOf
static CcsBool_t
PatchParser_StartOf(PatchParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}
#endif

static void
PatchParser_Expect(PatchParser_t * self, int n)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else PatchParser_SynErr(self, n);
}

#ifdef PatchParser_USE_GetSS
static void
PatchParser_GetSS(PatchParser_t * self, SubScanner_t subscanner)
{
    if (self->t) PatchParser_TokenDecRef(self, self->t);
    self->t = self->la;
    self->la = subscanner(self, self->scanner.cur->fname,
			  self->scanner.cur->pos,
			  self->scanner.cur->line,
			  self->scanner.cur->col);
}
#endif

#ifdef PatchParser_USE_ExpectSS
static void
PatchParser_ExpectSS(PatchParser_t * self, int n, SubScanner_t subscanner)
{
    if (self->la->kind == n) PatchParser_GetSS(self, subscanner);
    else PatchParser_SynErr(self, n);
}
#endif

#ifdef PatchParser_USE_ExpectWeak
static void
PatchParser_ExpectWeak(PatchParser_t * self, int n, int follow)
{
    if (self->la->kind == n) PatchParser_Get(self);
    else {
	PatchParser_SynErr(self, n);
	while (!PatchParser_StartOf(self, follow)) PatchParser_Get(self);
    }
}
#endif

#ifdef PatchParser_USE_WeakSeparator
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
#endif /* PatchParser_USE_WeakSeparator */

/*---- ProductionsHeader ----*/
static void PatchParser_Patch(PatchParser_t * self);
static void PatchParser_FilePatch(PatchParser_t * self, PatchFile_t ** filepatch);
static void PatchParser_HeadLine(PatchParser_t * self);
static void PatchParser_FileSubLine(PatchParser_t * self, CcsPosition_t ** subfname);
static void PatchParser_FileAddLine(PatchParser_t * self, CcsPosition_t ** addfname);
static void PatchParser_Piece(PatchParser_t * self, PatchPiece_t ** piece);
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
    self->maxT = 13;
    self->first = self->last = NULL;
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
    {
	PatchFile_t * cur, * next;
	for (cur = self->first; cur; cur = next) {
	    next = cur->next;
	    PatchFile_Destruct(cur);
	}
    }
    /*---- enable ----*/
    if (self->la) PatchParser_TokenDecRef(self, self->la);
    if (self->t) PatchParser_TokenDecRef(self, self->t);
    PatchScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- SubScanners ----*/
typedef struct {
    CcsToken_t base;
    PatchLine_t * lines;
}  PatchToken_t;
static void PatchToken_Destruct(CcsToken_t * self)
{
    PatchToken_t * pself = (PatchToken_t *)self;
    if (pself->lines) PatchLineList_Destruct(pself->lines);
    CcsFree(self);
}

static CcsToken_t *
PatchSubScanner_LineList(PatchParser_t * self, const char * fname,
			 int pos, int line, int col)
{
    PatchLine_t * lines;
    PatchToken_t * token;

    self->subLastEol = self->addLastEol = TRUE;
    lines = PatchLineList(&self->scanner, self->subStart, self->subNum,
    	    		  self->addStart, self->addNum, &self->subLastEol, &self->addLastEol);
    token = CcsMalloc(sizeof(PatchToken_t));
    token->base.next = NULL;
    token->base.destructor = PatchToken_Destruct;
    token->base.input = self->scanner.cur;
    token->base.refcnt = 1;
    token->base.kind = PatchScanner_piecelines;
    token->base.loc.fname = fname;
    token->base.loc.line = line;
    token->base.loc.col = col;
    token->base.pos = pos;
    token->base.val = NULL;
    token->lines = lines;
    return &token->base;
}
/*---- enable ----*/

/*---- ProductionsBody ----*/
static void
PatchParser_Patch(PatchParser_t * self)
{
    PatchFile_t * newFilePatch; 
    PatchParser_FilePatch(self, &newFilePatch);
    self->first = self->last = newFilePatch; 
    while (self->la->kind == 5 || self->la->kind == 6) {
	PatchParser_FilePatch(self, &newFilePatch);
	if (newFilePatch) {
	    if (self->last) {
		self->last->next = newFilePatch; self->last = newFilePatch;
	    } else {
		self->first = self->last = newFilePatch;
	    }
	} 
    }
}

static void
PatchParser_FilePatch(PatchParser_t * self, PatchFile_t ** filepatch)
{
    CcsPosition_t * subfname, * addfname;
    PatchPiece_t * newPiece; 
    while (self->la->kind == 5) {
	PatchParser_HeadLine(self);
    }
    PatchParser_FileSubLine(self, &subfname);
    PatchParser_FileAddLine(self, &addfname);
    printf("Scanning: %s %s\n", subfname->text, addfname->text);
    if (!(*filepatch = PatchFile(subfname->text, addfname->text)))
       PatchParser_SemErrT(self, "Not enough memory");
    CcsPosition_Destruct(subfname);
    CcsPosition_Destruct(addfname); 
    PatchParser_Piece(self, &newPiece);
    PatchFile_Append(*filepatch, newPiece); 
    while (self->la->kind == 8) {
	PatchParser_Piece(self, &newPiece);
	PatchFile_Append(*filepatch, newPiece); 
    }
}

static void
PatchParser_HeadLine(PatchParser_t * self)
{
    PatchParser_Expect(self, 5);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_FileSubLine(PatchParser_t * self, CcsPosition_t ** subfname)
{
    CcsToken_t * beg; 
    PatchParser_Expect(self, 6);
    PatchScanner_TokenIncRef(&self->scanner, beg = self->la); 
    while (PatchParser_StartOf(self, 2)) {
	PatchParser_Get(self);
    }
    *subfname = PatchScanner_GetPosition(&self->scanner, beg, self->la);
    PatchScanner_TokenDecRef(&self->scanner, beg); 
    if (self->la->kind == 3) {
	PatchParser_Get(self);
	while (PatchParser_StartOf(self, 1)) {
	    PatchParser_Get(self);
	}
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_FileAddLine(PatchParser_t * self, CcsPosition_t ** addfname)
{
    CcsToken_t * beg; 
    PatchParser_Expect(self, 7);
    PatchScanner_TokenIncRef(&self->scanner, beg = self->la); 
    while (PatchParser_StartOf(self, 2)) {
	PatchParser_Get(self);
    }
    *addfname = PatchScanner_GetPosition(&self->scanner, beg, self->la);
    PatchScanner_TokenDecRef(&self->scanner, beg); 
    if (self->la->kind == 3) {
	PatchParser_Get(self);
	while (PatchParser_StartOf(self, 1)) {
	    PatchParser_Get(self);
	}
    }
    PatchParser_Expect(self, 4);
}

static void
PatchParser_Piece(PatchParser_t * self, PatchPiece_t ** piece)
{
    PatchParser_Expect(self, 8);
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 10);
    PatchParser_Expect(self, 2);
    self->subStart = atoi(self->t->val); self->subNum = 0; 
    if (self->la->kind == 11) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
	self->subNum = atoi(self->t->val); 
    }
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 12);
    PatchParser_Expect(self, 2);
    self->addStart = atoi(self->t->val); self->addNum = 0; 
    if (self->la->kind == 11) {
	PatchParser_Get(self);
	PatchParser_Expect(self, 2);
	self->addNum = atoi(self->t->val); 
    }
    PatchParser_Expect(self, 9);
    while (self->la->kind == 9) {
	PatchParser_Get(self);
    }
    PatchParser_Expect(self, 8);
    while (PatchParser_StartOf(self, 1)) {
	PatchParser_Get(self);
    }
    PatchParser_ExpectSS(self, 4, PatchSubScanner_LineList);
    PatchParser_Expect(self, 1);
    printf("  Piece: %d, %d %s  %d, %d %s\n",
	   self->subStart, self->subNum, self->subLastEol ? "TRUE" : "FALSE",
	   self->addStart, self->addNum, self->addLastEol ? "TRUE" : "FALSE");
    *piece = PatchPiece(self->subStart, self->subNum, self->addStart, self->addNum,
			((PatchToken_t *)(self->t))->lines, self->subLastEol, self->addLastEol);
    ((PatchToken_t *)(self->t))->lines = NULL; 
}

/*---- enable ----*/

static void
PatchParser_SynErr(PatchParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "piecelines" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "tab" "\" expected"; break;
    case 4: s = "\"" "eol" "\" expected"; break;
    case 5: s = "\"" "infoch" "\" expected"; break;
    case 6: s = "\"" "--- " "\" expected"; break;
    case 7: s = "\"" "+++ " "\" expected"; break;
    case 8: s = "\"" "@@" "\" expected"; break;
    case 9: s = "\"" " " "\" expected"; break;
    case 10: s = "\"" "-" "\" expected"; break;
    case 11: s = "\"" "," "\" expected"; break;
    case 12: s = "\"" "+" "\" expected"; break;
    case 13: s = "\"" "???" "\" expected"; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    PatchParser_SemErr(self, self->la, "%s", s);
}

#ifdef PatchParser_USE_StartOf
static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    */
    "*..............", /* 0 */
    ".***.*********.", /* 1 */
    ".**..*********."  /* 2 */
    /*---- enable ----*/
};
#endif /* PatchParser_USE_StartOf */
