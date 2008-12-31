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
#include  "Scanner.h"

static int Char2State(int chr);
static int Identifier2KWKind(const char * key, size_t keylen, int defaultVal);
static void PgnScanner_Init(PgnScanner_t * self);
static CcsToken_t * PgnScanner_NextToken(PgnScanner_t * self);
static void PgnScanner_GetCh(PgnScanner_t * self);

static const char * dummyval = "dummy";
PgnScanner_t *
PgnScanner(PgnScanner_t * self, CcsErrorPool_t * errpool,
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
#ifdef COCO_INDENTATION
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * COCO_INDENT_START)))
	goto errquit3;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + COCO_INDENT_START;
    *self->indentUsed++ = 0;
#endif
    PgnScanner_Init(self);
    return self;
#ifdef COCO_INDENTATION
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
PgnScanner_Init(PgnScanner_t * self)
{
    /*---- declarations ----*/
    self->eofSym = 0;
    self->maxT = 12;
    self->noSym = 12;
    /*---- enable ----*/

    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
    PgnScanner_GetCh(self);
}

void
PgnScanner_Destruct(PgnScanner_t * self)
{
    CcsToken_t * cur, * next;

#ifdef COCO_INDENTATION
    CcsFree(self->indent);
#endif
    for (cur = self->busyTokenList; cur; cur = next) {
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    CcsToken_Destruct(self->dummyToken);
    CcsBuffer_Destruct(&self->buffer);
}

CcsToken_t *
PgnScanner_GetDummy(PgnScanner_t * self)
{
    return self->dummyToken;
}

CcsToken_t *
PgnScanner_Scan(PgnScanner_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = PgnScanner_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

CcsToken_t *
PgnScanner_Peek(PgnScanner_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = PgnScanner_NextToken(self);
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
PgnScanner_ResetPeek(PgnScanner_t * self)
{
    *self->peekToken = *self->curToken;
}

void
PgnScanner_IncRef(PgnScanner_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
PgnScanner_DecRef(PgnScanner_t * self, CcsToken_t * token)
{
    CcsToken_t ** curToken;
    if (token == self->dummyToken) return;
    if (--token->refcnt > 0) return;
    CcsAssert(self->busyTokenList != NULL);
    for (curToken = &self->busyTokenList;
	 *curToken != token; curToken = &(*curToken)->next)
	CcsAssert(*curToken && curToken != self->curToken);
    /* Found, *curToken == token, detach and destroy it. */
    *curToken = (*curToken)->next;
    CcsToken_Destruct(token);
    /* Adjust CcsBuffer busy pointer */
    if (curToken == &self->busyTokenList) {
	if (self->busyTokenList) {
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
	} else {
	    CcsBuffer_ClearBusy(&self->buffer);
	}
    }
}

CcsPosition_t *
PgnScanner_GetPosition(PgnScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    int len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

CcsPosition_t *
PgnScanner_GetPositionBetween(PgnScanner_t * self, const CcsToken_t * begin,
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

/* All the following things are used by PgnScanner_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 34, 34, 4 },	/* '"' '"' */
    { 46, 46, 33 },	/* '.' '.' */
    { 48, 48, 45 },	/* '0' '0' */
    { 49, 49, 44 },	/* '1' '1' */
    { 50, 57, 3 },	/* '2' '9' */
    { 66, 66, 11 },	/* 'B' 'B' */
    { 75, 75, 11 },	/* 'K' 'K' */
    { 78, 78, 11 },	/* 'N' 'N' */
    { 79, 79, 29 },	/* 'O' 'O' */
    { 81, 82, 11 },	/* 'Q' 'R' */
    { 91, 91, 1 },	/* '[' '[' */
    { 97, 104, 28 },	/* 'a' 'h' */
    { 123, 123, 8 },	/* '{' '{' */
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
    char * keystr;
#ifndef COCO_CASE_SENSITIVE
    char * cur;
#endif
    Identifier2KWKind_t * i2k;

    if (!(keystr = CcsMalloc(keylen + 1))) exit(-1);
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef COCO_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    CcsFree(keystr);
    return i2k ? i2k->val : defaultVal;
}

static int
PgnScanner_GetKWKind(PgnScanner_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}

static void
PgnScanner_GetCh(PgnScanner_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
#ifdef COCO_INDENTATION
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
PgnScanner_LockCh(PgnScanner_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
PgnScanner_UnlockCh(PgnScanner_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
PgnScanner_ResetCh(PgnScanner_t * self, SLock_t * slock)
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
PgnScanner_Comment(PgnScanner_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	PgnScanner_LockCh(self, &slock); PgnScanner_GetCh(self);
	if (self->ch != c->start[1]) {
	    PgnScanner_ResetCh(self, &slock);
	    return FALSE;
	}
	PgnScanner_UnlockCh(self, &slock);
    }
    PgnScanner_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		PgnScanner_LockCh(self, &slock); PgnScanner_GetCh(self);
		if (self->ch == c->end[1]) {
		    PgnScanner_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    PgnScanner_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		PgnScanner_LockCh(self, &slock); PgnScanner_GetCh(self);
		if (self->ch == c->start[1]) {
		    PgnScanner_UnlockCh(self, &slock);
		    ++level;
		} else {
		    PgnScanner_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	PgnScanner_GetCh(self);
    }
    self->oldEols = self->line - line0;
    PgnScanner_GetCh(self);
    return TRUE;
}

#ifdef COCO_INDENTATION
static CcsToken_t *
PgnScanner_IndentGenerator(PgnScanner_t * self)
{
    int newLen; int * newIndent, * curIndent;
    CcsToken_t * head, * cur;

    if (!self->lineStart) return NULL;
    CcsAssert(self->indent < self->indentUsed);
    self->lineStart = FALSE;
    if (self->col > self->indentUsed[-1]) {
	if (self->indentUsed == self->indentLast) {
	    newLen = (self->indentLast - self->indent) + COCO_INDENT_START;
	    newIndent = CcRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = self->col;
	return CcsToken(COCO_INDENT_IN, self->pos,
			self->col, self->line, NULL, 0);
    }
    for (curIndent = self->indentUsed - 1; self->col < *curIndent; --curIndent);
    if (self->col > *curIndent)
	return CcsToken(COCO_INDENT_ERR, self->pos,
			self->col, self->line, NULL, 0);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CcsToken(COCO_INDENT_OUT, self->pos,
		       self->col, self->line, NULL, 0);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
#endif

static CcsToken_t *
PgnScanner_NextToken(PgnScanner_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       /*---- scan1 ----*/
	       || (self->ch >= '\t' && self->ch <= '\n')
	       || self->ch == '\r'
	       /*---- enable ----*/
	       ) PgnScanner_GetCh(self);
#ifdef COCO_INDENTATION
	if ((t = PgnScanner_IndentGenerator(self))) return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		PgnScanner_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    PgnScanner_GetCh(self);
    switch (state) {
    case -1: kind = self->eofSym; break;
    case 0: kind = self->noSym; break;
    /*---- scan3 ----*/
    case 1:
	if ((self->ch >= 'A' && self->ch <= 'Z') ||
	    self->ch == '_' ||
	    (self->ch >= 'a' && self->ch <= 'z')) {
	    PgnScanner_GetCh(self); goto case_2;
	} else { kind = self->noSym; break; }
    case 2: case_2:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'Z') ||
	    self->ch == '_' ||
	    (self->ch >= 'a' && self->ch <= 'z')) {
	    PgnScanner_GetCh(self); goto case_2;
	} else { kind = 1; break; }
    case 3: case_3:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanner_GetCh(self); goto case_3;
	} else { kind = 2; break; }
    case 4: case_4:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '!') ||
	    (self->ch >= '#' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    PgnScanner_GetCh(self); goto case_4;
	} else if (self->ch == '"') {
	    PgnScanner_GetCh(self); goto case_5;
	} else if (self->ch == '\\') {
	    PgnScanner_GetCh(self); goto case_6;
	} else { kind = self->noSym; break; }
    case 5: case_5:
	if (self->ch == ']') {
	    PgnScanner_GetCh(self); goto case_7;
	} else { kind = self->noSym; break; }
    case 6: case_6:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    PgnScanner_GetCh(self); goto case_4;
	} else { kind = self->noSym; break; }
    case 7: case_7:
	{ kind = 3; break; }
    case 8: case_8:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= '|') ||
	    (self->ch >= '~' && self->ch <= 65535)) {
	    PgnScanner_GetCh(self); goto case_8;
	} else if (self->ch == '}') {
	    PgnScanner_GetCh(self); goto case_10;
	} else if (self->ch == '\\') {
	    PgnScanner_GetCh(self); goto case_9;
	} else { kind = self->noSym; break; }
    case 9: case_9:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    PgnScanner_GetCh(self); goto case_8;
	} else { kind = self->noSym; break; }
    case 10: case_10:
	{ kind = 4; break; }
    case 11: case_11:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanner_GetCh(self); goto case_12;
	} else { kind = self->noSym; break; }
    case 12: case_12:
	if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanner_GetCh(self); goto case_13;
	} else { kind = self->noSym; break; }
    case 13: case_13:
	if (self->ch == '#') {
	    PgnScanner_GetCh(self); goto case_17;
	} else if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_14;
	} else if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_15;
	} else if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_16;
	} else { kind = 5; break; }
    case 14: case_14:
	if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_14;
	} else { kind = 5; break; }
    case 15: case_15:
	if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_15;
	} else { kind = 5; break; }
    case 16: case_16:
	if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_16;
	} else { kind = 5; break; }
    case 17: case_17:
	{ kind = 5; break; }
    case 18: case_18:
	if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_18;
	} else { kind = 6; break; }
    case 19: case_19:
	if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_19;
	} else { kind = 6; break; }
    case 20: case_20:
	if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_20;
	} else { kind = 6; break; }
    case 21: case_21:
	{ kind = 6; break; }
    case 22: case_22:
	if (self->ch == 'O') {
	    PgnScanner_GetCh(self); goto case_23;
	} else { kind = self->noSym; break; }
    case 23: case_23:
	if (self->ch == '#') {
	    PgnScanner_GetCh(self); goto case_27;
	} else if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_24;
	} else if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_25;
	} else if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_26;
	} else { kind = 7; break; }
    case 24: case_24:
	if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_24;
	} else { kind = 7; break; }
    case 25: case_25:
	if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_25;
	} else { kind = 7; break; }
    case 26: case_26:
	if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_26;
	} else { kind = 7; break; }
    case 27: case_27:
	{ kind = 7; break; }
    case 28:
	if ((self->ch >= '1' && self->ch <= '8')) {
	    PgnScanner_GetCh(self); goto case_30;
	} else if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanner_GetCh(self); goto case_12;
	} else if (self->ch == 'x') {
	    PgnScanner_GetCh(self); goto case_11;
	} else { kind = self->noSym; break; }
    case 29:
	if (self->ch == '-') {
	    PgnScanner_GetCh(self); goto case_31;
	} else { kind = self->noSym; break; }
    case 30: case_30:
	if ((self->ch >= 'a' && self->ch <= 'h')) {
	    PgnScanner_GetCh(self); goto case_12;
	} else if (self->ch == '#') {
	    PgnScanner_GetCh(self); goto case_17;
	} else if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_14;
	} else if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_15;
	} else if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_16;
	} else if (self->ch == 'x') {
	    PgnScanner_GetCh(self); goto case_11;
	} else { kind = 5; break; }
    case 31: case_31:
	if (self->ch == 'O') {
	    PgnScanner_GetCh(self); goto case_32;
	} else { kind = self->noSym; break; }
    case 32: case_32:
	if (self->ch == '#') {
	    PgnScanner_GetCh(self); goto case_21;
	} else if (self->ch == '+') {
	    PgnScanner_GetCh(self); goto case_18;
	} else if (self->ch == '?') {
	    PgnScanner_GetCh(self); goto case_19;
	} else if (self->ch == '!') {
	    PgnScanner_GetCh(self); goto case_20;
	} else if (self->ch == '-') {
	    PgnScanner_GetCh(self); goto case_22;
	} else { kind = 6; break; }
    case 33:
	{ kind = 8; break; }
    case 34: case_34:
	if (self->ch == '0') {
	    PgnScanner_GetCh(self); goto case_35;
	} else { kind = self->noSym; break; }
    case 35: case_35:
	{ kind = 9; break; }
    case 36: case_36:
	if (self->ch == '1') {
	    PgnScanner_GetCh(self); goto case_37;
	} else { kind = self->noSym; break; }
    case 37: case_37:
	{ kind = 10; break; }
    case 38: case_38:
	if (self->ch == '2') {
	    PgnScanner_GetCh(self); goto case_39;
	} else { kind = self->noSym; break; }
    case 39: case_39:
	if (self->ch == '-') {
	    PgnScanner_GetCh(self); goto case_40;
	} else { kind = self->noSym; break; }
    case 40: case_40:
	if (self->ch == '1') {
	    PgnScanner_GetCh(self); goto case_41;
	} else { kind = self->noSym; break; }
    case 41: case_41:
	if (self->ch == '/') {
	    PgnScanner_GetCh(self); goto case_42;
	} else { kind = self->noSym; break; }
    case 42: case_42:
	if (self->ch == '2') {
	    PgnScanner_GetCh(self); goto case_43;
	} else { kind = self->noSym; break; }
    case 43: case_43:
	{ kind = 11; break; }
    case 44:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanner_GetCh(self); goto case_3;
	} else if (self->ch == '-') {
	    PgnScanner_GetCh(self); goto case_34;
	} else if (self->ch == '/') {
	    PgnScanner_GetCh(self); goto case_38;
	} else { kind = 2; break; }
    case 45:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    PgnScanner_GetCh(self); goto case_3;
	} else if (self->ch == '-') {
	    PgnScanner_GetCh(self); goto case_36;
	} else { kind = 2; break; }
    /*---- enable ----*/
    }
    t = CcsToken(kind, pos, col, line,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
