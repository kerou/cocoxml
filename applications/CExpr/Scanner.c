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
#include  <ctype.h>
#include  "Scanner.h"
#include  "c/ScanInput.h"
#include  "c/Indent.h"

static CcsToken_t * CExprScanner_Skip(void * scanner, CcsScanInput_t * input);
static int CExprScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    0, /* additionalSpace */
    0, /* eofSym */
    24, /* maxT */
    24, /* noSym */
    /*---- enable ----*/
    CExprScanner_Skip,
    CExprScanner_Kind
};

#ifdef CExprScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    CExprScanner_INDENT_IN, CExprScanner_INDENT_OUT, CExprScanner_INDENT_ERR
};
static void CcsGetCh(CcsScanInput_t * si)
{
    CcsIndent_t * indent = (CcsIndent_t *)(si + 1);
    if (si->oldEols == 0 && si->ch == '\n') indent->lineStart = TRUE;
    CcsScanInput_GetCh(si);
}
#else
#define CcsGetCh(si)  CcsScanInput_GetCh(si)
#endif

static const char * dummyval = "dummy";

static CcsBool_t
CExprScanner_Init(CExprScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

CExprScanner_t *
CExprScanner(CExprScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
#ifdef CExprScanner_INDENTATION
    if (!CcsIndent_Init((CcsIndent_t *)(self->cur + 1), &Scanner_IndentInfo))
	goto errquit1;
#endif
    if (!CExprScanner_Init(self, errpool)) goto errquit2;
    CcsGetCh(self->cur);
    return self;
 errquit2:
#ifdef CExprScanner_INDENTATION
    CcsIndent_Destruct((CcsIndent_t *)(self->cur + 1));
 errquit1:
#endif
    CcsScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

CExprScanner_t *
CExprScanner_ByName(CExprScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
#ifdef CExprScanner_INDENTATION
    if (!CcsIndent_Init((CcsIndent_t *)(self->cur + 1), &Scanner_IndentInfo))
	goto errquit1;
#endif
    if (!CExprScanner_Init(self, errpool)) goto errquit2;
    CcsGetCh(self->cur);
    return self;
 errquit2:
#ifdef CExprScanner_INDENTATION
    CcsIndent_Destruct((CcsIndent_t *)(self->cur + 1));
 errquit1:
#endif
    CcsScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

void
CExprScanner_Destruct(CExprScanner_t * self)
{
    CcsScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
#ifdef CExprScanner_INDENTATION
	CcsIndent_Destruct((CcsIndent_t *)(cur + 1));
#endif
	CcsScanInput_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
CExprScanner_GetDummy(CExprScanner_t * self)
{
    CExprScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
CExprScanner_Scan(CExprScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * next;
    for (;;) {
	token = CcsScanInput_Scan(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (self->cur->next == NULL) break;
	CcsScanInput_TokenDecRef(token->input, token);
	next = self->cur->next;
	CcsScanInput_DecRef(self->cur);
	self->cur = next;
    }
    return token;
}

CcsToken_t *
CExprScanner_Peek(CExprScanner_t * self)
{
    CcsToken_t * token; CcsScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = CcsScanInput_Peek(self->cur);
	if (token->kind != Scanner_Info.eofSym) break;
	if (cur->next == NULL) break;
	CcsScanInput_TokenDecRef(token->input, token);
	cur = cur->next;
    }
    return token;
}

void
CExprScanner_ResetPeek(CExprScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

void
CExprScanner_TokenIncRef(CExprScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
CExprScanner_TokenDecRef(CExprScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

#ifdef CExprScanner_INDENTATION
void
CExprScanner_IndentLimit(CExprScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    CcsAssert(indentIn->kind == CExprScanner_INDENT_IN);
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsPosition_t *
CExprScanner_GetPosition(CExprScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
CExprScanner_GetPositionBetween(CExprScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsBool_t
CExprScanner_Include(CExprScanner_t * self, FILE * fp, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput(self, &Scanner_Info, fp))) return FALSE;
#ifdef CExprScanner_INDENTATION
    if (!CcsIndent_Init((CcsIndent_t *)(input + 1), &Scanner_IndentInfo)) {
	CcsScanInput_Destruct(input);
	return FALSE;
    }
#endif
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
}

CcsBool_t
CExprScanner_IncludeByName(CExprScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn, CcsToken_t ** token)
{
    CcsScanInput_t * input;
    if (!(input = CcsScanInput_ByName(self, &Scanner_Info,
				      list, self->cur->fname, infn)))
	return FALSE;
#ifdef CExprScanner_INDENTATION
    if (!CcsIndent_Init((CcsIndent_t *)(input + 1), &Scanner_IndentInfo)) {
	CcsScanInput_Destruct(input);
	return FALSE;
    }
#endif
    CcsScanInput_WithDraw(self->cur, *token);
    input->next = self->cur;
    self->cur = input;
    CcsGetCh(input);
    *token = CcsScanInput_Scan(self->cur);
    return TRUE;
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
    { 33, 33, 9 },	/* '!' '!' */
    { 37, 37, 19 },	/* '%' '%' */
    { 38, 38, 23 },	/* '&' '&' */
    { 40, 40, 20 },	/* '(' '(' */
    { 41, 41, 21 },	/* ')' ')' */
    { 42, 42, 17 },	/* '*' '*' */
    { 43, 43, 15 },	/* '+' '+' */
    { 45, 45, 16 },	/* '-' '-' */
    { 47, 47, 18 },	/* '/' '/' */
    { 48, 57, 1 },	/* '0' '9' */
    { 58, 58, 3 },	/* ':' ':' */
    { 60, 60, 25 },	/* '<' '<' */
    { 61, 61, 7 },	/* '=' '=' */
    { 62, 62, 24 },	/* '>' '>' */
    { 63, 63, 2 },	/* '?' '?' */
    { 94, 94, 6 },	/* '^' '^' */
    { 124, 124, 22 },	/* '|' '|' */
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

#ifdef CExprScanner_KEYWORD_USED
typedef struct {
    const char * key;
    int val;
}  Identifier2KWKind_t;

static const Identifier2KWKind_t i2kArr[] = {
    /*---- identifiers2keywordkinds ----*/
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
#ifndef CExprScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[CExprScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > CExprScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef CExprScanner_CASE_SENSITIVE
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
#endif /* CExprScanner_KEYWORD_USED */

static const CcsComment_t comments[] = {
/*---- comments ----*/
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsToken_t *
CExprScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef CExprScanner_INDENTATION
    CcsToken_t * t;
#endif
    const CcsComment_t * curComment;
    for (;;) {
	while (input->ch == ' '
	       /*---- scan1 ----*/
	       || (input->ch >= '\t' && input->ch <= '\n')
	       || input->ch == '\r'
	       /*---- enable ----*/
	       )  CcsGetCh(input);
#ifdef CExprScanner_INDENTATION
	if ((t = CcsIndent_Generator((CcsIndent_t *)(input + 1), input)))
	    return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (input->ch == curComment->start[0] &&
		CcsScanInput_Comment(input, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    return NULL;
}

static int CExprScanner_Kind(void * scanner, CcsScanInput_t * input)
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
	if ((input->ch >= '0' && input->ch <= '9')) {
	    CcsGetCh(input); goto case_1;
	} else { kind = 1; break; }
    case 2:
	{ kind = 2; break; }
    case 3:
	{ kind = 3; break; }
    case 4: case_4:
	{ kind = 4; break; }
    case 5: case_5:
	{ kind = 5; break; }
    case 6:
	{ kind = 7; break; }
    case 7:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_8;
	} else { kind = Scanner_Info.noSym; break; }
    case 8: case_8:
	{ kind = 9; break; }
    case 9:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 10: case_10:
	{ kind = 10; break; }
    case 11: case_11:
	{ kind = 12; break; }
    case 12: case_12:
	{ kind = 14; break; }
    case 13: case_13:
	{ kind = 15; break; }
    case 14: case_14:
	{ kind = 16; break; }
    case 15:
	{ kind = 17; break; }
    case 16:
	{ kind = 18; break; }
    case 17:
	{ kind = 19; break; }
    case 18:
	{ kind = 20; break; }
    case 19:
	{ kind = 21; break; }
    case 20:
	{ kind = 22; break; }
    case 21:
	{ kind = 23; break; }
    case 22:
	if (input->ch == '|') {
	    CcsGetCh(input); goto case_4;
	} else { kind = 6; break; }
    case 23:
	if (input->ch == '&') {
	    CcsGetCh(input); goto case_5;
	} else { kind = 8; break; }
    case 24:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_11;
	} else if (input->ch == '>') {
	    CcsGetCh(input); goto case_14;
	} else { kind = 11; break; }
    case 25:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_12;
	} else if (input->ch == '<') {
	    CcsGetCh(input); goto case_13;
	} else { kind = 13; break; }
    /*---- enable ----*/
    }
    return kind;
}
