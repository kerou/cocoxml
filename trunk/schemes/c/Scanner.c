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

static int Char2State(int chr);
static int Identifier2KWKind(const char * key, size_t keylen, int defaultVal);
static void CcsScanner_Init(CcsScanner_t * self);
static CcsToken_t * CcsScanner_NextToken(CcsScanner_t * self);
static void CcsScanner_GetCh(CcsScanner_t * self);

static const char * dummyval = "dummy";
CcsScanner_t *
CcsScanner(CcsScanner_t * self, CcsErrorPool_t * errpool,
	   const char * filename)
{
    FILE * fp;
    self->errpool = errpool;
    if (!(fp = fopen(filename, "r"))) {
	fprintf(stderr, "Can not open '%s'.\n", filename);
	goto errquit0;
    }
    if (!(self->dummyToken = CcsToken(0, 0, 0, 0, dummyval, strlen(dummyval))))
	goto errquit1;
    if (CcsBuffer(&self->buffer, fp) == NULL) goto errquit2;
#ifdef CcsScanner_INDENTATION
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * CcsScanner_INDENT_START)))
	goto errquit3;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + CcsScanner_INDENT_START;
    *self->indentUsed++ = 0;
#endif
    CcsScanner_Init(self);
    return self;
#ifdef CcsScanner_INDENTATION
 errquit3:
    CcsBuffer_Destruct(&self->buffer);
#endif
 errquit2:
    fclose(fp);
 errquit1:
    CcsToken_Destruct(self->dummyToken);
 errquit0:
    return NULL;
}

static void
CcsScanner_Init(CcsScanner_t * self)
{
    /*---- declarations ----*/
    self->eofSym = 0;
    self->maxT = 47;
    self->noSym = 47;
    /*---- enable ----*/

    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
    CcsScanner_GetCh(self);
}

void
CcsScanner_Destruct(CcsScanner_t * self)
{
    CcsToken_t * cur, * next;

#ifdef CcsScanner_INDENTATION
    CcsFree(self->indent);
#endif
    for (cur = self->busyTokenList; cur; cur = next) {
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
    CcsBuffer_Destruct(&self->buffer);
}

CcsToken_t *
CcsScanner_GetDummy(CcsScanner_t * self)
{
    CcsScanner_IncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
CcsScanner_Scan(CcsScanner_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = CcsScanner_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

CcsToken_t *
CcsScanner_Peek(CcsScanner_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = CcsScanner_NextToken(self);
	    if (self->peekToken == &self->busyTokenList)
		CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
	}
	cur = *self->peekToken;
	self->peekToken = &cur->next;
    } while (cur->kind > self->maxT); /* Skip pragmas */
    ++cur->refcnt;
    return cur;
}

void
CcsScanner_ResetPeek(CcsScanner_t * self)
{
    self->peekToken = self->curToken;
}

void
CcsScanner_IncRef(CcsScanner_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
CcsScanner_DecRef(CcsScanner_t * self, CcsToken_t * token)
{
    if (--token->refcnt > 1) return;
    CcsAssert(token->refcnt == 1);
    if (token != self->busyTokenList) return;
    /* Detach all tokens which is refered by self->busyTokenList only. */
    while (token && token->refcnt <= 1) {
	CcsAssert(token->refcnt == 1);
	/* Detach token. */
	if (self->curToken == &token->next)
	    self->curToken = &self->busyTokenList;
	if (self->peekToken == &token->next)
	    self->peekToken = &self->busyTokenList;
	self->busyTokenList = token->next;
	CcsToken_Destruct(token);
	token = self->busyTokenList;
    }
    /* Adjust CcsBuffer busy pointer */
    if (self->busyTokenList) {
	CcsAssert(self->busyTokenList->refcnt > 1);
	CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    } else {
	CcsBuffer_ClearBusy(&self->buffer);
    }
}

CcsPosition_t *
CcsScanner_GetPosition(CcsScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    int len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

CcsPosition_t *
CcsScanner_GetPositionBetween(CcsScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    int begpos = begin->pos + strlen(begin->val);
    int len = end->pos - begpos;
    const char * start = CcsBuffer_GetString(&self->buffer, begpos, len);
    const char * cur, * last = start + len;

    /* Skip the leading spaces. */
    for (cur = start; cur < last; ++cur)
	if (*cur != ' ' && *cur != '\t' && *cur != '\r' && *cur != '\n') break;
    return CcsPosition(begpos + (cur - start), last - cur, 0, cur);
}

/* All the following things are used by CcsScanner_NextToken. */
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
    { "IGNORECASE", 10 },
    { "INDENTATIONS", 11 },
    { "MEMBERS", 7 },
    { "NESTED", 18 },
    { "PRAGMAS", 14 },
    { "PRODUCTIONS", 20 },
    { "SCHEME", 24 },
    { "SECTION", 25 },
    { "SYNC", 42 },
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
CcsScanner_GetKWKind(CcsScanner_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}

static void
CcsScanner_GetCh(CcsScanner_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
#ifdef CcsScanner_INDENTATION
	    self->lineStart = TRUE;
#endif
	} else if (self->ch == '\t') {
	    self->col += 8 - self->col % 8;
	} else {
	    /* FIX ME: May be the width of some specical character
	     * is NOT self->chBytes. */
	    self->col += self->chBytes;
	}
	self->ch = CcsBuffer_Read(&self->buffer, &self->chBytes);
	self->pos = CcsBuffer_GetPos(&self->buffer);
    }
}

typedef struct {
    int ch, chBytes;
    int pos, line, col;
}  SLock_t;
static void
CcsScanner_LockCh(CcsScanner_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
CcsScanner_UnlockCh(CcsScanner_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
CcsScanner_ResetCh(CcsScanner_t * self, SLock_t * slock)
{
    self->ch = slock->ch;
    self->chBytes = slock->chBytes;
    self->pos = slock->pos;
    self->line = slock->line;
    CcsBuffer_LockReset(&self->buffer);
}

typedef struct {
    int start[2];
    int end[2];
    CcsBool_t nested;
}  CcsComment_t;

static const CcsComment_t comments[] = {
/*---- comments ----*/
    { { '/', '/' }, { '\n', 0 }, FALSE },
    { { '/', '*' }, { '*', '/' }, TRUE },
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsBool_t
CcsScanner_Comment(CcsScanner_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	CcsScanner_LockCh(self, &slock); CcsScanner_GetCh(self);
	if (self->ch != c->start[1]) {
	    CcsScanner_ResetCh(self, &slock);
	    return FALSE;
	}
	CcsScanner_UnlockCh(self, &slock);
    }
    CcsScanner_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		CcsScanner_LockCh(self, &slock); CcsScanner_GetCh(self);
		if (self->ch == c->end[1]) {
		    CcsScanner_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    CcsScanner_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		CcsScanner_LockCh(self, &slock); CcsScanner_GetCh(self);
		if (self->ch == c->start[1]) {
		    CcsScanner_UnlockCh(self, &slock);
		    ++level;
		} else {
		    CcsScanner_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	CcsScanner_GetCh(self);
    }
    self->oldEols = self->line - line0;
    CcsScanner_GetCh(self);
    return TRUE;
}

#ifdef CcsScanner_INDENTATION
static CcsToken_t *
CcsScanner_IndentGenerator(CcsScanner_t * self)
{
    int newLen; int * newIndent, * curIndent;
    CcsToken_t * head, * cur;

    if (!self->lineStart) return NULL;
    CcsAssert(self->indent < self->indentUsed);
    self->lineStart = FALSE;
    if (self->col > self->indentUsed[-1]) {
	if (self->indentUsed == self->indentLast) {
	    newLen = (self->indentLast - self->indent) + CcsScanner_INDENT_START;
	    newIndent = CcRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = self->col;
	return CcsToken(CcsScanner_INDENT_IN, self->pos,
			self->col, self->line, NULL, 0);
    }
    for (curIndent = self->indentUsed - 1; self->col < *curIndent; --curIndent);
    if (self->col > *curIndent)
	return CcsToken(CcsScanner_INDENT_ERR, self->pos,
			self->col, self->line, NULL, 0);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CcsToken(CcsScanner_INDENT_OUT, self->pos,
		       self->col, self->line, NULL, 0);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
#endif

static CcsToken_t *
CcsScanner_NextToken(CcsScanner_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       /*---- scan1 ----*/
	       || (self->ch >= '\t' && self->ch <= '\n')
	       || self->ch == '\r'
	       /*---- enable ----*/
	       ) CcsScanner_GetCh(self);
#ifdef CcsScanner_INDENTATION
	if ((t = CcsScanner_IndentGenerator(self))) return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		CcsScanner_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    CcsScanner_GetCh(self);
    switch (state) {
    case -1: kind = self->eofSym; break;
    case 0: kind = self->noSym; break;
    /*---- scan3 ----*/
    case 1: case_1:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'Z') ||
	    self->ch == '_' ||
	    (self->ch >= 'a' && self->ch <= 'z')) {
	    CcsScanner_GetCh(self); goto case_1;
	} else { kind = CcsScanner_GetKWKind(self, pos, self->pos, 1); break; }
    case 2: case_2:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    CcsScanner_GetCh(self); goto case_2;
	} else { kind = 2; break; }
    case 3: case_3:
	{ kind = 3; break; }
    case 4: case_4:
	{ kind = 4; break; }
    case 5:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '&') ||
	    (self->ch >= '(' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    CcsScanner_GetCh(self); goto case_6;
	} else if (self->ch == '\\') {
	    CcsScanner_GetCh(self); goto case_7;
	} else { kind = self->noSym; break; }
    case 6: case_6:
	if (self->ch == '\'') {
	    CcsScanner_GetCh(self); goto case_9;
	} else { kind = self->noSym; break; }
    case 7: case_7:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    CcsScanner_GetCh(self); goto case_8;
	} else { kind = self->noSym; break; }
    case 8: case_8:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    CcsScanner_GetCh(self); goto case_8;
	} else if (self->ch == '\'') {
	    CcsScanner_GetCh(self); goto case_9;
	} else { kind = self->noSym; break; }
    case 9: case_9:
	{ kind = 5; break; }
    case 10: case_10:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'Z') ||
	    self->ch == '_' ||
	    (self->ch >= 'a' && self->ch <= 'z')) {
	    CcsScanner_GetCh(self); goto case_10;
	} else { kind = 48; break; }
    case 11: case_11:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '!') ||
	    (self->ch >= '#' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    CcsScanner_GetCh(self); goto case_11;
	} else if (self->ch == '"') {
	    CcsScanner_GetCh(self); goto case_3;
	} else if (self->ch == '\\') {
	    CcsScanner_GetCh(self); goto case_12;
	} else if (self->ch == '\n' ||
	    self->ch == '\r') {
	    CcsScanner_GetCh(self); goto case_4;
	} else { kind = self->noSym; break; }
    case 12: case_12:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    CcsScanner_GetCh(self); goto case_11;
	} else { kind = self->noSym; break; }
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
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_16;
	} else if (self->ch == '>') {
	    CcsScanner_GetCh(self); goto case_19;
	} else if (self->ch == ')') {
	    CcsScanner_GetCh(self); goto case_27;
	} else { kind = 22; break; }
    case 29:
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_18;
	} else { kind = 30; break; }
    case 30:
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_26;
	} else { kind = 36; break; }
    /*---- enable ----*/
    }
    t = CcsToken(kind, pos, col, line,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
