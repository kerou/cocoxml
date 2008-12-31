/*---- license ----*/
/*-------------------------------------------------------------------------
pgn.atg -- atg for chess pgn file
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

static void PgnParser_SynErr(PgnParser_t * self, int n);
static const char * set[];

static void
PgnParser_Get(PgnParser_t * self)
{
    self->t = self->la;
    for (;;) {
	self->la = PgnScanner_Scan(&self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/*---- Pragmas ----*/
	/*---- enable ----*/
    }
}

static CcsBool_t
PgnParser_StartOf(PgnParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
PgnParser_Expect(PgnParser_t * self, int n)
{
    if (self->la->kind == n) PgnParser_Get(self);
    else PgnParser_SynErr(self, n);
}

static void
PgnParser_ExpectWeak(PgnParser_t * self, int n, int follow)
{
    if (self->la->kind == n) PgnParser_Get(self);
    else {
	PgnParser_SynErr(self, n);
	while (!PgnParser_StartOf(self, follow)) PgnParser_Get(self);
    }
}

static CcsBool_t
PgnParser_WeakSeparator(PgnParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { PgnParser_Get(self); return TRUE; }
    else if (PgnParser_StartOf(self, repFol)) { return FALSE; }
    PgnParser_SynErr(self, n);
    while (!(PgnParser_StartOf(self, syFol) ||
	     PgnParser_StartOf(self, repFol) ||
	     PgnParser_StartOf(self, 0)))
	PgnParser_Get(self);
    return PgnParser_StartOf(self, syFol);
}

/*---- ProductionsHeader ----*/
static void PgnParser_pgn(PgnParser_t * self);
static void PgnParser_game(PgnParser_t * self);
static void PgnParser_info(PgnParser_t * self);
static void PgnParser_round(PgnParser_t * self);
static void PgnParser_resultnum(PgnParser_t * self);
static void PgnParser_move(PgnParser_t * self);
/*---- enable ----*/

void
PgnParser_Parse(PgnParser_t * self)
{
    self->t = NULL;
    self->la = PgnScanner_GetDummy(&self->scanner);
    PgnParser_Get(self);
    /*---- ParseRoot ----*/
    PgnParser_pgn(self);
    /*---- enable ----*/
    PgnParser_Expect(self, 0);
}

void
PgnParser_SemErr(PgnParser_t * self, const CcsToken_t * token,
		 const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, token->line, token->col,
			format, ap);
    va_end(ap);
}

void
PgnParser_SemErrT(PgnParser_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(&self->errpool, self->t->line, self->t->col,
			format, ap);
    va_end(ap);
}

PgnParser_t *
PgnParser(PgnParser_t * self, const char * fname, FILE * errfp)
{
    if (!CcsErrorPool(&self->errpool, errfp)) goto errquit0;
    if (!PgnScanner(&self->scanner, &self->errpool, fname)) goto errquit1;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 12;
    /*---- enable ----*/
    return self;
 ERRQUIT:
 errquit1:
    CcsErrorPool_Destruct(&self->errpool);
 errquit0:
    return NULL;
}

void
PgnParser_Destruct(PgnParser_t * self)
{
    /*---- destructor ----*/
    /*---- enable ----*/
    PgnScanner_Destruct(&self->scanner);
    CcsErrorPool_Destruct(&self->errpool);
}

/*---- ProductionsBody ----*/
static void
PgnParser_pgn(PgnParser_t * self)
{
    while (self->la->kind == 1 || self->la->kind == 2 || self->la->kind == 4) {
	PgnParser_game(self);
    }
}

static void
PgnParser_game(PgnParser_t * self)
{
    while (self->la->kind == 1) {
	PgnParser_info(self);
    }
    while (self->la->kind == 2) {
	PgnParser_round(self);
    }
    PgnParser_Expect(self, 4);
    PgnParser_resultnum(self);
}

static void
PgnParser_info(PgnParser_t * self)
{
    PgnParser_Expect(self, 1);
    PgnParser_Expect(self, 3);
}

static void
PgnParser_round(PgnParser_t * self)
{
    PgnParser_Expect(self, 2);
    PgnParser_Expect(self, 8);
    PgnParser_move(self);
    if (self->la->kind == 5 || self->la->kind == 6 || self->la->kind == 7) {
	PgnParser_move(self);
    }
}

static void
PgnParser_resultnum(PgnParser_t * self)
{
    if (self->la->kind == 9) {
	PgnParser_Get(self);
    } else if (self->la->kind == 10) {
	PgnParser_Get(self);
    } else if (self->la->kind == 11) {
	PgnParser_Get(self);
    } else PgnParser_SynErr(self, 13);
}

static void
PgnParser_move(PgnParser_t * self)
{
    if (self->la->kind == 5) {
	PgnParser_Get(self);
    } else if (self->la->kind == 6) {
	PgnParser_Get(self);
    } else if (self->la->kind == 7) {
	PgnParser_Get(self);
    } else PgnParser_SynErr(self, 14);
}

/*---- enable ----*/

static void
PgnParser_SynErr(PgnParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "ident" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "string" "\" expected"; break;
    case 4: s = "\"" "result" "\" expected"; break;
    case 5: s = "\"" "basemove" "\" expected"; break;
    case 6: s = "\"" "castling" "\" expected"; break;
    case 7: s = "\"" "castlingL" "\" expected"; break;
    case 8: s = "\"" "." "\" expected"; break;
    case 9: s = "\"" "1-0" "\" expected"; break;
    case 10: s = "\"" "0-1" "\" expected"; break;
    case 11: s = "\"" "1/2-1/2" "\" expected"; break;
    case 12: s = "\"" "???" "\" expected"; break;
    case 13: s = "this symbol not expected in \"" "resultnum" "\""; break;
    case 14: s = "this symbol not expected in \"" "move" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    PgnParser_SemErr(self, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0   */
    "*............."  /* 0 */
    /*---- enable ----*/
};
