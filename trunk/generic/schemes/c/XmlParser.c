/*---- license ----*/
/*-------------------------------------------------------------------------
 Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>

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
#include  "c/XmlParser.h"
#include  "c/XmlScanner.h"
#include  "c/Token.h"
#include  "c/CGlobals.h"

/*---- cIncludes ----*/
/*---- enable ----*/

static void CcsXmlParser_SynErr(CcsXmlParser_t * self, int n);
static const char * set[];

static void
CcsXmlParser_Get(CcsXmlParser_t * self)
{
    for (;;) {
	self->t = self->la;
	self->la = CcsXmlScanner_Scan(self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/*---- Pragmas ----*/
	/*---- enable ----*/
	self->la = self->t;
    }
}

static CcsBool_t
CcsXmlParser_StartOf(CcsXmlParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
CcsXmlParser_Expect(CcsXmlParser_t * self, int n)
{
    if (self->la->kind == n) CcsXmlParser_Get(self);
    else CcsXmlParser_SynErr(self, n);
}

static void
CcsXmlParser_ExpectWeak(CcsXmlParser_t * self, int n, int follow)
{
    if (self->la->kind == n) CcsXmlParser_Get(self);
    else {
	CcsXmlParser_SynErr(self, n);
	while (!CcsXmlParser_StartOf(self, follow)) CcsXmlParser_Get(self);
    }
}

static CcsBool_t
CcsXmlParser_WeakSeparator(CcsXmlParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { CcsXmlParser_Get(self); return TRUE; }
    else if (CcsXmlParser_StartOf(self, repFol)) { return FALSE; }
    CcsXmlParser_SynErr(self, n);
    while (!(CcsXmlParser_StartOf(self, syFol) ||
	     CcsXmlParser_StartOf(self, repFol) ||
	     CcsXmlParser_StartOf(self, 0)))
	CcsXmlParser_Get(self);
    return CcsXmlParser_StartOf(self, syFol);
}

/*---- ProductionsHeader ----*/
/*---- enable ----*/

void
CcsXmlParser_Parse(CcsXmlParser_t * self)
{
    self->t = NULL;
    self->la = CcsXmlScanner_GetDummy(self->scanner);
    CcsXmlParser_Get(self);
    /*---- ParseRoot ----*/
    /*---- enable ----*/
    CcsXmlParser_Expect(self, 0);
}

CcsXmlParser_t *
CcsXmlParser(CcsXmlParser_t * self, CcsGlobals_t * globals)
{
    self->globals = globals;
    self->scanner = globals->xmlscanner;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    /*---- enable ----*/
    return self;
}

void
CcsXmlParser_Destruct(CcsXmlParser_t * self)
{
    /*---- destructor ----*/
    /*---- enable ----*/
}

/*---- ProductionsBody ----*/
/*---- enable ----*/

static void
CcsXmlParser_SynErr(CcsXmlParser_t * self, int n)
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
    CcsGlobals_SemErr(self->globals, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*---- enable ----*/
};
