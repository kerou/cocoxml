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
#include  <ctype.h>
#include  <limits.h>
#include  "Scanner.h"
#include  "c/IncPathList.h"

/*------------------------------- ScanInput --------------------------------*/
struct PgnScanInput_s {
    PgnScanInput_t * next;

    PgnScanner_t   * scanner;
    char           * fname;
    CcsBuffer_t      buffer;

    CcsToken_t     * busyTokenList;
    CcsToken_t    ** curToken;
    CcsToken_t    ** peekToken;

    int              ch;
    int              chBytes;
    int              pos;
    int              line;
    int              col;
    int              oldEols;
    int              oldEolsEOL;

#ifdef PgnScanner_INDENTATION
    CcsBool_t        lineStart;
    int            * indent;
    int            * indentUsed;
    int            * indentLast;
    int              indentLimit;
#endif
};

static CcsToken_t * PgnScanInput_NextToken(PgnScanInput_t * self);

static CcsBool_t
PgnScanInput_Init(PgnScanInput_t * self, PgnScanner_t * scanner, FILE * fp)
{
    self->scanner = scanner;
    if (!CcsBuffer(&self->buffer, fp)) goto errquit0;
    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
#ifdef PgnScanner_INDENTATION
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * PgnScanner_INDENT_START)))
	goto errquit1;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + PgnScanner_INDENT_START;
    *self->indentUsed++ = 0;
    self->indentLimit = -1;
#endif
    return TRUE;
#ifdef PgnScanner_INDENTATION
 errquit1:
    CcsBuffer_Destruct(&self->buffer);
#endif
 errquit0:
    return FALSE;
}

static PgnScanInput_t *
PgnScanInput(PgnScanner_t * scanner, FILE * fp)
{
    PgnScanInput_t * self;
    if (!(self = CcsMalloc(sizeof(PgnScanInput_t)))) goto errquit0;
    self->next = NULL;
    self->fname = NULL;
    if (!PgnScanInput_Init(self, scanner, fp)) goto errquit1;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
}

static PgnScanInput_t *
PgnScanInput_ByName(PgnScanner_t * scanner, const CcsIncPathList_t * list,
		    const char * includer, const char * infn)
{
    FILE * fp;
    PgnScanInput_t * self;
    char infnpath[PATH_MAX];
    if (!(fp = CcsIncPathList_Open(list, infnpath, sizeof(infnpath),
				   includer, infn)))
	goto errquit0;
    if (!(self = CcsMalloc(sizeof(PgnScanInput_t) + strlen(infnpath) + 1)))
	goto errquit1;
    self->next = NULL;
    strcpy(self->fname = (char *)(self + 1), infnpath);
    if (!PgnScanInput_Init(self, scanner, fp)) goto errquit2;
    return self;
 errquit2:
    CcsFree(self);
 errquit1:
    fclose(fp);
 errquit0:
    return NULL;
}

static void
PgnScanInput_Destruct(PgnScanInput_t * self)
{
    CcsToken_t * cur, * next;

#ifdef PgnScanner_INDENTATION
    CcsFree(self->indent);
#endif
    for (cur = self->busyTokenList; cur; cur = next) {
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    CcsBuffer_Destruct(&self->buffer);
    CcsFree(self);
}

static void
PgnScanInput_GetCh(PgnScanInput_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
#ifdef PgnScanner_INDENTATION
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

static CcsToken_t *
PgnScanInput_Scan(PgnScanInput_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = PgnScanInput_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

static CcsToken_t *
PgnScanInput_Peek(PgnScanInput_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = PgnScanInput_NextToken(self);
	    if (self->peekToken == &self->busyTokenList)
		CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
	}
	cur = *self->peekToken;
	self->peekToken = &cur->next;
    } while (cur->kind > self->scanner->maxT); /* Skip pragmas */
    ++cur->refcnt;
    return cur;
}

static void
PgnScanInput_ResetPeek(PgnScanInput_t * self)
{
    self->peekToken = self->curToken;
}

static void
PgnScanInput_IncRef(PgnScanInput_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

static void
PgnScanInput_DecRef(PgnScanInput_t * self, CcsToken_t * token)
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

#ifdef PgnScanner_INDENTATION
static void
PgnScanInput_IndentLimit(PgnScanInput_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->kind == PgnScanner_INDENT_IN);
    self->indentLimit = indentIn->col;
}
#endif

static CcsPosition_t *
PgnScanInput_GetPosition(PgnScanInput_t * self, const CcsToken_t * begin,
			 const CcsToken_t * end)
{
    int len;
    CcsAssert(self == begin->input);
    CcsAssert(self == end->input);
    len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

static CcsPosition_t *
PgnScanInput_GetPositionBetween(PgnScanInput_t * self,
				const CcsToken_t * begin,
				const CcsToken_t * end)
{
    int begpos, len;
    CcsAssert(self == begin->input);
    CcsAssert(self == end->input);
    begpos = begin->pos + strlen(begin->val);
    len = end->pos - begpos;
    const char * start = CcsBuffer_GetString(&self->buffer, begpos, len);
    const char * cur, * last = start + len;

    /* Skip the leading spaces. */
    for (cur = start; cur < last; ++cur)
	if (*cur != ' ' && *cur != '\t' && *cur != '\r' && *cur != '\n') break;
    return CcsPosition(begpos + (cur - start), last - cur, 0, cur);
}

/*------------------------------- Scanner --------------------------------*/
static const char * dummyval = "dummy";

static CcsBool_t
PgnScanner_Init(PgnScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    /*---- declarations ----*/
    self->eofSym = 0;
    self->maxT = 23;
    self->noSym = 23;
    /*---- enable ----*/
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

PgnScanner_t *
PgnScanner(PgnScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = PgnScanInput(self, fp))) goto errquit0;
    if (!PgnScanner_Init(self, errpool)) goto errquit1;
    PgnScanInput_GetCh(self->cur);
    return self;
 errquit1:
    PgnScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

PgnScanner_t *
PgnScanner_ByName(PgnScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur = PgnScanInput_ByName(self, NULL, NULL, fn)))
	goto errquit0;
    if (!PgnScanner_Init(self, errpool)) goto errquit1;
    PgnScanInput_GetCh(self->cur);
    return self;
 errquit1:
    PgnScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

void
PgnScanner_Destruct(PgnScanner_t * self)
{
    PgnScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	PgnScanInput_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
PgnScanner_GetDummy(PgnScanner_t * self)
{
    PgnScanner_IncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
PgnScanner_Scan(PgnScanner_t * self)
{
    CcsToken_t * token; PgnScanInput_t * next;
    for (;;) {
	token = PgnScanInput_Scan(self->cur);
	if (token->kind != self->eofSym) break;
	if (self->cur->next == NULL) break;
	PgnScanInput_DecRef(token->input, token);
	next = self->cur->next;
	PgnScanInput_Destruct(self->cur);
	self->cur = next;
    }
    return token;
}

CcsToken_t *
PgnScanner_Peek(PgnScanner_t * self)
{
    CcsToken_t * token; PgnScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = PgnScanInput_Peek(self->cur);
	if (token->kind != self->eofSym) break;
	if (cur->next == NULL) break;
	PgnScanInput_DecRef(token->input, token);
	cur = cur->next;
    }
    return token;
}

void
PgnScanner_ResetPeek(PgnScanner_t * self)
{
    PgnScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	PgnScanInput_ResetPeek(cur);
}

void
PgnScanner_IncRef(PgnScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else PgnScanInput_IncRef(token->input, token);
}

void
PgnScanner_DecRef(PgnScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else PgnScanInput_DecRef(token->input, token);
}

#ifdef PgnScanner_INDENTATION
void
PgnScanner_IndentLimit(PgnScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    PgnScanInput_IndentLimit(self->cur, indentIn);
}
#endif

CcsPosition_t *
PgnScanner_GetPosition(PgnScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return PgnScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
PgnScanner_GetPositionBetween(PgnScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return PgnScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsBool_t
PgnScanner_Include(PgnScanner_t * self, FILE * fp)
{
    PgnScanInput_t * input;
    if (!(input = PgnScanInput(self, fp))) return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

CcsBool_t
PgnScanner_IncludeByName(PgnScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn)
{
    PgnScanInput_t * input;
    if (!(input = PgnScanInput_ByName(self, list, self->cur->fname, infn)))
	return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

/*------------------------------- ScanInput --------------------------------*/
/* All the following things are used by PgnScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 34, 34, 2 },	/* '"' '"' */
    { 46, 46, 75 },	/* '.' '.' */
    { 48, 48, 90 },	/* '0' '0' */
    { 49, 49, 89 },	/* '1' '1' */
    { 50, 56, 30 },	/* '2' '8' */
    { 57, 57, 1 },	/* '9' '9' */
    { 66, 66, 88 },	/* 'B' 'B' */
    { 68, 68, 45 },	/* 'D' 'D' */
    { 69, 69, 36 },	/* 'E' 'E' */
    { 75, 75, 8 },	/* 'K' 'K' */
    { 78, 78, 8 },	/* 'N' 'N' */
    { 79, 79, 31 },	/* 'O' 'O' */
    { 81, 81, 8 },	/* 'Q' 'Q' */
    { 82, 82, 86 },	/* 'R' 'R' */
    { 83, 83, 41 },	/* 'S' 'S' */
    { 84, 84, 59 },	/* 'T' 'T' */
    { 87, 87, 87 },	/* 'W' 'W' */
    { 91, 91, 35 },	/* '[' '[' */
    { 93, 93, 74 },	/* ']' ']' */
    { 97, 104, 29 },	/* 'a' 'h' */
    { 120, 120, 10 },	/* 'x' 'x' */
    { 123, 123, 5 },	/* '{' '{' */
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

#ifdef PgnScanner_KEYWORD_USED
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
#ifndef PgnScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[PgnScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > PgnScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef PgnScanner_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    return i2k ? i2k->val : defaultVal;
}

static int
GetKWKind(PgnScanInput_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}
#endif /* PgnScanner_KEYWORD_USED */

typedef struct {
    int ch, chBytes;
    int pos, line, col;
}  SLock_t;
static void
PgnScanInput_LockCh(PgnScanInput_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
PgnScanInput_UnlockCh(PgnScanInput_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
PgnScanInput_ResetCh(PgnScanInput_t * self, SLock_t * slock)
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
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsBool_t
PgnScanInput_Comment(PgnScanInput_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	PgnScanInput_LockCh(self, &slock); PgnScanInput_GetCh(self);
	if (self->ch != c->start[1]) {
	    PgnScanInput_ResetCh(self, &slock);
	    return FALSE;
	}
	PgnScanInput_UnlockCh(self, &slock);
    }
    PgnScanInput_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		PgnScanInput_LockCh(self, &slock); PgnScanInput_GetCh(self);
		if (self->ch == c->end[1]) {
		    PgnScanInput_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    PgnScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		PgnScanInput_LockCh(self, &slock); PgnScanInput_GetCh(self);
		if (self->ch == c->start[1]) {
		    PgnScanInput_UnlockCh(self, &slock);
		    ++level;
		} else {
		    PgnScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	PgnScanInput_GetCh(self);
    }
    self->oldEols = self->line - line0;
    PgnScanInput_GetCh(self);
    return TRUE;
}

#ifdef PgnScanner_INDENTATION
static CcsToken_t *
PgnScanInput_IndentGenerator(PgnScanInput_t * self)
{
    int newLen; int * newIndent, * curIndent;
    CcsToken_t * head, * cur;

    if (!self->lineStart) return NULL;
    CcsAssert(self->indent < self->indentUsed);
    /* Skip blank lines. */
    if (self->ch == '\r' || self->ch == '\n') return NULL;
    /* Dump all required IndentOut when EoF encountered. */
    if (self->ch == EoF) {
	head = NULL;
	while (self->indent < self->indentUsed - 1) {
	    cur = CcsToken(self, PgnScanner_INDENT_OUT, self->pos,
			   self->col, self->line, NULL, 0);
	    cur->next = head; head = cur;
	    --self->indentUsed;
	}
	return head;
    }
    if (self->indentLimit != -1 && self->col >= self->indentLimit) return NULL;
    self->indentLimit = -1;
    self->lineStart = FALSE;
    if (self->col > self->indentUsed[-1]) {
	if (self->indentUsed == self->indentLast) {
	    newLen = (self->indentLast - self->indent) + PgnScanner_INDENT_START;
	    newIndent = CcsRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = self->col;
	return CcsToken(self, PgnScanner_INDENT_IN, self->pos,
			self->col, self->line, NULL, 0);
    }
    for (curIndent = self->indentUsed - 1; self->col < *curIndent; --curIndent);
    if (self->col > *curIndent)
	return CcsToken(self, PgnScanner_INDENT_ERR, self->pos,
			self->col, self->line, NULL, 0);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CcsToken(self, PgnScanner_INDENT_OUT, self->pos,
		       self->col, self->line, NULL, 0);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
#endif

static CcsToken_t *
PgnScanInput_NextToken(PgnScanInput_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       /*---- scan1 ----*/
	       || (self->ch >= '\t' && self->ch <= '\n')
	       || self->ch == '\r'
	       /*---- enable ----*/
	       ) PgnScanInput_GetCh(self);
#ifdef PgnScanner_INDENTATION
	if ((t = PgnScanInput_IndentGenerator(self))) return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		PgnScanInput_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    PgnScanInput_GetCh(self);
    kind = -2; /* Avoid gcc warning */
    switch (state) {
    case -1: kind = self->scanner->eofSym; break;
    case 0: kind = self->scanner->noSym; break;
    /*---- scan3 ----*/
    case 1: case_1:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanInput_GetCh(self); goto case_1;
	} else { kind = 1; break; }
    case 2: case_2:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '!') ||
	    (self->ch >= '#' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    PgnScanInput_GetCh(self); goto case_2;
	} else if (self->ch == '"') {
	    PgnScanInput_GetCh(self); goto case_4;
	} else if (self->ch == '\\') {
	    PgnScanInput_GetCh(self); goto case_3;
	} else { kind = self->scanner->noSym; break; }
    case 3: case_3:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    PgnScanInput_GetCh(self); goto case_2;
	} else { kind = self->scanner->noSym; break; }
    case 4: case_4:
	{ kind = 2; break; }
    case 5: case_5:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= '|') ||
	    (self->ch >= '~' && self->ch <= 65535)) {
	    PgnScanInput_GetCh(self); goto case_5;
	} else if (self->ch == '}') {
	    PgnScanInput_GetCh(self); goto case_7;
	} else if (self->ch == '\\') {
	    PgnScanInput_GetCh(self); goto case_6;
	} else { kind = self->scanner->noSym; break; }
    case 6: case_6:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    PgnScanInput_GetCh(self); goto case_5;
	} else { kind = self->scanner->noSym; break; }
    case 7: case_7:
	{ kind = 3; break; }
    case 8:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_29;
	} else if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_9;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else { kind = self->scanner->noSym; break; }
    case 9: case_9:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else { kind = self->scanner->noSym; break; }
    case 10: case_10:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else { kind = self->scanner->noSym; break; }
    case 11: case_11:
	if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_12;
	} else { kind = self->scanner->noSym; break; }
    case 12: case_12:
	if (self->ch == '#') {
	    PgnScanInput_GetCh(self); goto case_18;
	} else if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_14;
	} else if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_15;
	} else if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_16;
	} else if (self->ch == '=') {
	    PgnScanInput_GetCh(self); goto case_17;
	} else { kind = 4; break; }
    case 13: case_13:
	if (self->ch == '#') {
	    PgnScanInput_GetCh(self); goto case_18;
	} else if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_14;
	} else if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_15;
	} else if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_16;
	} else { kind = 4; break; }
    case 14: case_14:
	if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_14;
	} else { kind = 4; break; }
    case 15: case_15:
	if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_15;
	} else { kind = 4; break; }
    case 16: case_16:
	if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_16;
	} else { kind = 4; break; }
    case 17: case_17:
	if (self->ch == 'B' ||
	    self->ch == 'K' ||
	    self->ch == 'N' ||
	    (self->ch >= 'Q' && self->ch <= 'R')) {
	    PgnScanInput_GetCh(self); goto case_13;
	} else { kind = self->scanner->noSym; break; }
    case 18: case_18:
	{ kind = 4; break; }
    case 19: case_19:
	if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_19;
	} else { kind = 5; break; }
    case 20: case_20:
	if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_20;
	} else { kind = 5; break; }
    case 21: case_21:
	if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_21;
	} else { kind = 5; break; }
    case 22: case_22:
	{ kind = 5; break; }
    case 23: case_23:
	if (self->ch == 'O') {
	    PgnScanInput_GetCh(self); goto case_24;
	} else { kind = self->scanner->noSym; break; }
    case 24: case_24:
	if (self->ch == '#') {
	    PgnScanInput_GetCh(self); goto case_28;
	} else if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_25;
	} else if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_26;
	} else if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_27;
	} else { kind = 6; break; }
    case 25: case_25:
	if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_25;
	} else { kind = 6; break; }
    case 26: case_26:
	if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_26;
	} else { kind = 6; break; }
    case 27: case_27:
	if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_27;
	} else { kind = 6; break; }
    case 28: case_28:
	{ kind = 6; break; }
    case 29: case_29:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_32;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else { kind = self->scanner->noSym; break; }
    case 30:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanInput_GetCh(self); goto case_1;
	} else if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else { kind = 1; break; }
    case 31:
	if (self->ch == '-') {
	    PgnScanInput_GetCh(self); goto case_33;
	} else { kind = self->scanner->noSym; break; }
    case 32: case_32:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else if (self->ch == '#') {
	    PgnScanInput_GetCh(self); goto case_18;
	} else if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_14;
	} else if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_15;
	} else if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_16;
	} else if (self->ch == '=') {
	    PgnScanInput_GetCh(self); goto case_17;
	} else { kind = 4; break; }
    case 33: case_33:
	if (self->ch == 'O') {
	    PgnScanInput_GetCh(self); goto case_34;
	} else { kind = self->scanner->noSym; break; }
    case 34: case_34:
	if (self->ch == '#') {
	    PgnScanInput_GetCh(self); goto case_22;
	} else if (self->ch == '+') {
	    PgnScanInput_GetCh(self); goto case_19;
	} else if (self->ch == '?') {
	    PgnScanInput_GetCh(self); goto case_20;
	} else if (self->ch == '!') {
	    PgnScanInput_GetCh(self); goto case_21;
	} else if (self->ch == '-') {
	    PgnScanInput_GetCh(self); goto case_23;
	} else { kind = 5; break; }
    case 35:
	{ kind = 7; break; }
    case 36:
	if (self->ch == 'v') {
	    PgnScanInput_GetCh(self); goto case_37;
	} else { kind = self->scanner->noSym; break; }
    case 37: case_37:
	if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_38;
	} else { kind = self->scanner->noSym; break; }
    case 38: case_38:
	if (self->ch == 'n') {
	    PgnScanInput_GetCh(self); goto case_39;
	} else { kind = self->scanner->noSym; break; }
    case 39: case_39:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_40;
	} else { kind = self->scanner->noSym; break; }
    case 40: case_40:
	{ kind = 8; break; }
    case 41:
	if (self->ch == 'i') {
	    PgnScanInput_GetCh(self); goto case_42;
	} else { kind = self->scanner->noSym; break; }
    case 42: case_42:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_43;
	} else { kind = self->scanner->noSym; break; }
    case 43: case_43:
	if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_44;
	} else { kind = self->scanner->noSym; break; }
    case 44: case_44:
	{ kind = 9; break; }
    case 45:
	if (self->ch == 'a') {
	    PgnScanInput_GetCh(self); goto case_46;
	} else { kind = self->scanner->noSym; break; }
    case 46: case_46:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_47;
	} else { kind = self->scanner->noSym; break; }
    case 47: case_47:
	if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_48;
	} else { kind = self->scanner->noSym; break; }
    case 48: case_48:
	{ kind = 10; break; }
    case 49: case_49:
	if (self->ch == 'u') {
	    PgnScanInput_GetCh(self); goto case_50;
	} else { kind = self->scanner->noSym; break; }
    case 50: case_50:
	if (self->ch == 'n') {
	    PgnScanInput_GetCh(self); goto case_51;
	} else { kind = self->scanner->noSym; break; }
    case 51: case_51:
	if (self->ch == 'd') {
	    PgnScanInput_GetCh(self); goto case_52;
	} else { kind = self->scanner->noSym; break; }
    case 52: case_52:
	{ kind = 11; break; }
    case 53: case_53:
	if (self->ch == 'l') {
	    PgnScanInput_GetCh(self); goto case_54;
	} else { kind = self->scanner->noSym; break; }
    case 54: case_54:
	if (self->ch == 'o') {
	    PgnScanInput_GetCh(self); goto case_55;
	} else { kind = self->scanner->noSym; break; }
    case 55: case_55:
	{ kind = 14; break; }
    case 56: case_56:
	if (self->ch == 'l') {
	    PgnScanInput_GetCh(self); goto case_57;
	} else { kind = self->scanner->noSym; break; }
    case 57: case_57:
	if (self->ch == 'o') {
	    PgnScanInput_GetCh(self); goto case_58;
	} else { kind = self->scanner->noSym; break; }
    case 58: case_58:
	{ kind = 15; break; }
    case 59:
	if (self->ch == 'i') {
	    PgnScanInput_GetCh(self); goto case_60;
	} else { kind = self->scanner->noSym; break; }
    case 60: case_60:
	if (self->ch == 'm') {
	    PgnScanInput_GetCh(self); goto case_61;
	} else { kind = self->scanner->noSym; break; }
    case 61: case_61:
	if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_62;
	} else { kind = self->scanner->noSym; break; }
    case 62: case_62:
	if (self->ch == 'C') {
	    PgnScanInput_GetCh(self); goto case_63;
	} else { kind = self->scanner->noSym; break; }
    case 63: case_63:
	if (self->ch == 'o') {
	    PgnScanInput_GetCh(self); goto case_64;
	} else { kind = self->scanner->noSym; break; }
    case 64: case_64:
	if (self->ch == 'n') {
	    PgnScanInput_GetCh(self); goto case_65;
	} else { kind = self->scanner->noSym; break; }
    case 65: case_65:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_66;
	} else { kind = self->scanner->noSym; break; }
    case 66: case_66:
	if (self->ch == 'r') {
	    PgnScanInput_GetCh(self); goto case_67;
	} else { kind = self->scanner->noSym; break; }
    case 67: case_67:
	if (self->ch == 'o') {
	    PgnScanInput_GetCh(self); goto case_68;
	} else { kind = self->scanner->noSym; break; }
    case 68: case_68:
	if (self->ch == 'l') {
	    PgnScanInput_GetCh(self); goto case_69;
	} else { kind = self->scanner->noSym; break; }
    case 69: case_69:
	{ kind = 16; break; }
    case 70: case_70:
	if (self->ch == 'u') {
	    PgnScanInput_GetCh(self); goto case_71;
	} else { kind = self->scanner->noSym; break; }
    case 71: case_71:
	if (self->ch == 'l') {
	    PgnScanInput_GetCh(self); goto case_72;
	} else { kind = self->scanner->noSym; break; }
    case 72: case_72:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_73;
	} else { kind = self->scanner->noSym; break; }
    case 73: case_73:
	{ kind = 17; break; }
    case 74:
	{ kind = 18; break; }
    case 75:
	{ kind = 19; break; }
    case 76: case_76:
	if (self->ch == '0') {
	    PgnScanInput_GetCh(self); goto case_77;
	} else { kind = self->scanner->noSym; break; }
    case 77: case_77:
	{ kind = 20; break; }
    case 78: case_78:
	if (self->ch == '2') {
	    PgnScanInput_GetCh(self); goto case_79;
	} else { kind = self->scanner->noSym; break; }
    case 79: case_79:
	if (self->ch == '-') {
	    PgnScanInput_GetCh(self); goto case_80;
	} else { kind = self->scanner->noSym; break; }
    case 80: case_80:
	if (self->ch == '1') {
	    PgnScanInput_GetCh(self); goto case_81;
	} else { kind = self->scanner->noSym; break; }
    case 81: case_81:
	if (self->ch == '/') {
	    PgnScanInput_GetCh(self); goto case_82;
	} else { kind = self->scanner->noSym; break; }
    case 82: case_82:
	if (self->ch == '2') {
	    PgnScanInput_GetCh(self); goto case_83;
	} else { kind = self->scanner->noSym; break; }
    case 83: case_83:
	{ kind = 21; break; }
    case 84: case_84:
	if (self->ch == '1') {
	    PgnScanInput_GetCh(self); goto case_85;
	} else { kind = self->scanner->noSym; break; }
    case 85: case_85:
	{ kind = 22; break; }
    case 86:
	if ((self->ch >= 'a' && self->ch <= 'd') ||
	    (self->ch >= 'f' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_29;
	} else if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_9;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else if (self->ch == 'o') {
	    PgnScanInput_GetCh(self); goto case_49;
	} else if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_91;
	} else { kind = self->scanner->noSym; break; }
    case 87:
	if (self->ch == 'h') {
	    PgnScanInput_GetCh(self); goto case_92;
	} else { kind = self->scanner->noSym; break; }
    case 88:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_29;
	} else if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_9;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else if (self->ch == 'l') {
	    PgnScanInput_GetCh(self); goto case_93;
	} else { kind = self->scanner->noSym; break; }
    case 89:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanInput_GetCh(self); goto case_1;
	} else if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else if (self->ch == '-') {
	    PgnScanInput_GetCh(self); goto case_76;
	} else if (self->ch == '/') {
	    PgnScanInput_GetCh(self); goto case_78;
	} else { kind = 1; break; }
    case 90:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanInput_GetCh(self); goto case_1;
	} else if (self->ch == '-') {
	    PgnScanInput_GetCh(self); goto case_84;
	} else { kind = 1; break; }
    case 91: case_91:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanInput_GetCh(self); goto case_11;
	} else if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanInput_GetCh(self); goto case_32;
	} else if (self->ch == 'x') {
	    PgnScanInput_GetCh(self); goto case_10;
	} else if (self->ch == 's') {
	    PgnScanInput_GetCh(self); goto case_70;
	} else { kind = self->scanner->noSym; break; }
    case 92: case_92:
	if (self->ch == 'i') {
	    PgnScanInput_GetCh(self); goto case_94;
	} else { kind = self->scanner->noSym; break; }
    case 93: case_93:
	if (self->ch == 'a') {
	    PgnScanInput_GetCh(self); goto case_95;
	} else { kind = self->scanner->noSym; break; }
    case 94: case_94:
	if (self->ch == 't') {
	    PgnScanInput_GetCh(self); goto case_96;
	} else { kind = self->scanner->noSym; break; }
    case 95: case_95:
	if (self->ch == 'c') {
	    PgnScanInput_GetCh(self); goto case_97;
	} else { kind = self->scanner->noSym; break; }
    case 96: case_96:
	if (self->ch == 'e') {
	    PgnScanInput_GetCh(self); goto case_98;
	} else { kind = self->scanner->noSym; break; }
    case 97: case_97:
	if (self->ch == 'k') {
	    PgnScanInput_GetCh(self); goto case_99;
	} else { kind = self->scanner->noSym; break; }
    case 98: case_98:
	if (self->ch == 'E') {
	    PgnScanInput_GetCh(self); goto case_53;
	} else { kind = 12; break; }
    case 99: case_99:
	if (self->ch == 'E') {
	    PgnScanInput_GetCh(self); goto case_56;
	} else { kind = 13; break; }
    /*---- enable ----*/
    }
    CcsAssert(kind != -2);
    t = CcsToken(self, kind, pos, col, line,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
