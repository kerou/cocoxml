/*---- license ----*/
/*-------------------------------------------------------------------------
 Coco.ATG -- Attributed Grammar
 Compiler Generator Coco/R,
 Copyright (c) 1990, 2004 Hanspeter Moessenboeck, University of Linz
 extended by M. Loeberbauer & A. Woess, Univ. of Linz
 with improvements by Pat Terry, Rhodes University.
 ported to C by Charles Wang <charlesw123456@gmail.com>

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

 As an exception, it is allowed to write an extension of Coco/R that is
 used as a plugin in non-free software.

 If not otherwise stated, any source code generated by Coco/R (other than 
 Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Scanner.h"
#include  "c/ScanInput.h"
#include  "c/Indent.h"

static CcsBool_t CcsScanner_AddInit(void * additional, void * scanner);
static void CcsScanner_AddDestruct(void * additional);
static CcsToken_t * CcsScanner_Skip(void * scanner, CcsScanInput_t * input);
static int CcsScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    0, /* additionalSpace */
    0, /* eofSym */
    47, /* maxT */
    47, /* noSym */
    /*---- enable ----*/
    CcsScanner_AddInit,
    CcsScanner_AddDestruct,
    CcsScanner_Skip,
    CcsScanner_Kind
};

#ifdef CcsScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    CcsScanner_INDENT_IN, CcsScanner_INDENT_OUT, CcsScanner_INDENT_ERR
};
static void CcsGetCh(CcsScanInput_t * si)
{
    CcsBool_t lineStart;
    CcsIndent_t * indent = (CcsIndent_t *)(si + 1);
    /*---- checkLineStart ----*/
    lineStart = (si->ch == '\n')
    /*---- enable ----*/
    if (lineStart) indent->lineStart = TRUE;
    CcsScanInput_GetCh(si);
}
#else
#define CcsGetCh(si)  CcsScanInput_GetCh(si)
#endif

static const char * dummyval = "dummy";

static CcsBool_t
CcsScanner_Init(CcsScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

CcsScanner_t *
CcsScanner(CcsScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!CcsScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

CcsScanner_t *
CcsScanner_ByName(CcsScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!CcsScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Detach(self->cur);
 errquit0:
    return NULL;
}

void
CcsScanner_Destruct(CcsScanner_t * self)
{
    CcsScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	/* May be trigged by .atg/.xatg. */
	CcsAssert(cur->busyFirst == NULL);
	CcsAssert(cur->busyLast == NULL);
	CcsScanInput_Detach(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
CcsScanner_GetDummy(CcsScanner_t * self)
{
    CcsScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
CcsScanner_Scan(CcsScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * next;
    for (;;) {
	token = CcsScanInput_Scan(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (self->cur->next == NULL) break;
	CcsScanInput_TokenDecRef(token->input, token);
	next = self->cur->next;
	CcsScanInput_Detach(self->cur);
	self->cur = next;
    }
    return token;
}

void
CcsScanner_TokenIncRef(CcsScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
CcsScanner_TokenDecRef(CcsScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

const char *
CcsScanner_GetString(CcsScanner_t * self, const CcsToken_t * begin, size_t len)
{
    return CcsScanInput_GetString(begin->input, begin, len);
}

CcsPosition_t *
CcsScanner_GetPosition(CcsScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
CcsScanner_GetPositionBetween(CcsScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsToken_t *
CcsScanner_Peek(CcsScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = CcsScanInput_Peek(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (cur->next == NULL) break;
	cur = cur->next;
    }
    return token;
}

void
CcsScanner_ResetPeek(CcsScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

#ifdef CcsScanner_INDENTATION
void
CcsScanner_IndentLimit(CcsScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    /*CcsAssert(indentIn->kind == CcsScanner_INDENT_IN);*/
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsBool_t
CcsScanner_Include(CcsScanner_t * self, FILE * fp, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput(self, &Scanner_Info, fp))) return FALSE;
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
}

CcsBool_t
CcsScanner_IncludeByName(CcsScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput_ByName(self, &Scanner_Info,
				      list, self->cur->fname, infn)))
	return FALSE;
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
}

CcsBool_t
CcsScanner_InsertExpect(CcsScanner_t * self, int kind, const char * val,
			size_t vallen, CcsToken_t ** token)
{
    CcsBool_t ret;
    CcsScanInput_WithDraw(self->cur, *token);
    ret = CcsScanInput_Prepend(self->cur, kind, val, vallen);
    *token = CcsScanInput_Scan(self->cur);
    return ret;
}

/* All the following things are used by CcsScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 34, 34, 11 },	/* '"' '"' */
    { 36, 36, 10 },	/* '$' '$' */
    { 39, 39, 5 },	/* '\'' '\'' */
    { 40, 40, 30 },	/* '(' '(' */
    { 41, 41, 21 },	/* ')' ')' */
    { 43, 43, 14 },	/* '+' '+' */
    { 45, 45, 15 },	/* '-' '-' */
    { 46, 46, 28 },	/* '.' '.' */
    { 48, 57, 2 },	/* '0' '9' */
    { 60, 60, 29 },	/* '<' '<' */
    { 61, 61, 13 },	/* '=' '=' */
    { 62, 62, 17 },	/* '>' '>' */
    { 65, 90, 1 },	/* 'A' 'Z' */
    { 91, 91, 22 },	/* '[' '[' */
    { 93, 93, 23 },	/* ']' ']' */
    { 95, 95, 1 },	/* '_' '_' */
    { 97, 122, 1 },	/* 'a' 'z' */
    { 123, 123, 24 },	/* '{' '{' */
    { 124, 124, 20 },	/* '|' '|' */
    { 125, 125, 25 },	/* '}' '}' */
    /*---- enable ----*/
};
static const int c2sNum = sizeof(c2sArr) / sizeof(c2sArr[0]);

static int
c2sCmp(const void * key, const void * c2s)
{
    int keyval = *(const int *)key;
    const Char2State_t * ccc2s = (const Char2State_t *)c2s;
    if (keyval < ccc2s->keyFrom) return -1;
    if (keyval > ccc2s->keyTo) return 1;
    return 0;
}
static int
Char2State(int chr)
{
    Char2State_t * c2s;

    c2s = bsearch(&chr, c2sArr, c2sNum, sizeof(Char2State_t), c2sCmp);
    return c2s ? c2s->val : 0;
}

#ifdef CcsScanner_KEYWORD_USED
typedef struct {
    const char * key;
    int val;
}  Identifier2KWKind_t;

static const Identifier2KWKind_t i2kArr[] = {
    /*---- identifiers2keywordkinds ----*/
    { "ANY", 29 },
    { "CHARACTERS", 12 },
    { "COMMENTS", 15 },
    { "COMPILER", 6 },
    { "CONSTRUCTOR", 8 },
    { "CONTEXT", 44 },
    { "DESTRUCTOR", 9 },
    { "END", 23 },
    { "FROM", 16 },
    { "IF", 43 },
    { "IGNORE", 19 },
    { "MEMBERS", 7 },
    { "NESTED", 18 },
    { "OPTIONS", 10 },
    { "PRAGMAS", 14 },
    { "PRODUCTIONS", 20 },
    { "SCHEME", 24 },
    { "SECTION", 25 },
    { "SYNC", 42 },
    { "TERMINALS", 11 },
    { "TO", 17 },
    { "TOKENS", 13 },
    { "WEAK", 35 },
    /*---- enable ----*/
};
static const int i2kNum = sizeof(i2kArr) / sizeof(i2kArr[0]);

static int
i2kCmp(const void * key, const void * i2k)
{
    return strcmp((const char *)key, ((const Identifier2KWKind_t *)i2k)->key);
}

static int
Identifier2KWKind(const char * key, size_t keylen, int defaultVal)
{
#ifndef CcsScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[CcsScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > CcsScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef CcsScanner_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    return i2k ? i2k->val : defaultVal;
}

static int
GetKWKind(CcsScanInput_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}
#endif /* CcsScanner_KEYWORD_USED */

static CcsBool_t
CcsScanner_AddInit(void * additional, void * scanner)
{
#ifdef CcsScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
CcsScanner_AddDestruct(void * additional)
{
#ifdef CcsScanner_INDENTATION
    CcsIndent_Destruct(additional);
#endif
}

static const CcsComment_t comments[] = {
/*---- comments ----*/
    { { '/', '/' }, { '\n', 0 }, FALSE },
    { { '/', '*' }, { '*', '/' }, TRUE },
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsToken_t *
CcsScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef CcsScanner_INDENTATION
    CcsToken_t * t;
#endif
    const CcsComment_t * curComment;
    for (;;) {
	while (
	       /*---- scan1 ----*/
	       (input->ch >= '\t' && input->ch <= '\n')
	       || input->ch == '\r'
	       || input->ch == ' '
	       /*---- enable ----*/
	       )  CcsGetCh(input);
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (input->ch == curComment->start[0] &&
		CcsScanInput_Comment(input, curComment)) break;
	if (curComment >= commentsLast) break;
    }
#ifdef CcsScanner_INDENTATION
    if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	return t;
#endif
    return NULL;
}

static int CcsScanner_Kind(void * scanner, CcsScanInput_t * input)
{
    int kind, pos, state;

    pos = input->pos;
    state = Char2State(input->ch);
    CcsGetCh(input);
    kind = -2; /* Avoid gcc warning */
    switch (state) {
    case -1: kind = Scanner_Info.eofSym; break;
    case 0: kind = Scanner_Info.noSym; break;
    /*---- scan3 ----*/
    case 1: case_1:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'Z') ||
	    input->ch == '_' ||
	    (input->ch >= 'a' && input->ch <= 'z')) {
	    CcsGetCh(input); goto case_1;
	} else { kind = GetKWKind(input, pos, input->pos, 1); break; }
    case 2: case_2:
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_2;
	} else { kind = 2; break; }
    case 3: case_3:
	{ kind = 3; break; }
    case 4: case_4:
	{ kind = 4; break; }
    case 5:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '&') ||
	    (input->ch >= '(' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_6;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_7;
	} else { kind = Scanner_Info.noSym; break; }
    case 6: case_6:
	if (input->ch == '\'') {
	    CcsGetCh(input); goto case_9;
	} else { kind = Scanner_Info.noSym; break; }
    case 7: case_7:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_8;
	} else { kind = Scanner_Info.noSym; break; }
    case 8: case_8:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'a' && input->ch <= 'f')) {
	    CcsGetCh(input); goto case_8;
	} else if (input->ch == '\'') {
	    CcsGetCh(input); goto case_9;
	} else { kind = Scanner_Info.noSym; break; }
    case 9: case_9:
	{ kind = 5; break; }
    case 10: case_10:
	if ((input->ch >= '0' && input->ch <= '9') ||
	    (input->ch >= 'A' && input->ch <= 'Z') ||
	    input->ch == '_' ||
	    (input->ch >= 'a' && input->ch <= 'z')) {
	    CcsGetCh(input); goto case_10;
	} else { kind = 48; break; }
    case 11: case_11:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_3;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_12;
	} else if (input->ch == '\n' ||
	    input->ch == '\r') {
	    CcsGetCh(input); goto case_4;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_11;
	} else { kind = Scanner_Info.noSym; break; }
    case 13:
	{ kind = 21; break; }
    case 14:
	{ kind = 26; break; }
    case 15:
	{ kind = 27; break; }
    case 16: case_16:
	{ kind = 28; break; }
    case 17:
	{ kind = 31; break; }
    case 18: case_18:
	{ kind = 32; break; }
    case 19: case_19:
	{ kind = 33; break; }
    case 20:
	{ kind = 34; break; }
    case 21:
	{ kind = 37; break; }
    case 22:
	{ kind = 38; break; }
    case 23:
	{ kind = 39; break; }
    case 24:
	{ kind = 40; break; }
    case 25:
	{ kind = 41; break; }
    case 26: case_26:
	{ kind = 45; break; }
    case 27: case_27:
	{ kind = 46; break; }
    case 28:
	if (input->ch == '.') {
	    CcsGetCh(input); goto case_16;
	} else if (input->ch == '>') {
	    CcsGetCh(input); goto case_19;
	} else if (input->ch == ')') {
	    CcsGetCh(input); goto case_27;
	} else { kind = 22; break; }
    case 29:
	if (input->ch == '.') {
	    CcsGetCh(input); goto case_18;
	} else { kind = 30; break; }
    case 30:
	if (input->ch == '.') {
	    CcsGetCh(input); goto case_26;
	} else { kind = 36; break; }
    /*---- enable ----*/
    }
    return kind;
}
