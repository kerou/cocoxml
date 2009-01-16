/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  <ctype.h>
#include  <limits.h>
#include  "Scanner.h"
#include  "c/IncPathList.h"

/*------------------------------- ScanInput --------------------------------*/
struct CfScanInput_s {
    CfScanInput_t * next;

    int              refcnt;
    CfScanner_t   * scanner;
    char           * fname;
    FILE           * fp;
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

#ifdef CfScanner_INDENTATION
    CcsBool_t        lineStart;
    int            * indent;
    int            * indentUsed;
    int            * indentLast;
    int              indentLimit;
#endif
};

static CcsToken_t * CfScanInput_NextToken(CfScanInput_t * self);

static CcsBool_t
CfScanInput_Init(CfScanInput_t * self, CfScanner_t * scanner, FILE * fp)
{
    self->next = NULL;
    self->refcnt = 1;
    self->scanner = scanner;
    self->fp = fp;
    if (!CcsBuffer(&self->buffer, fp)) goto errquit0;
    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
#ifdef CfScanner_INDENTATION
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * CfScanner_INDENT_START)))
	goto errquit1;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + CfScanner_INDENT_START;
    *self->indentUsed++ = 0;
    self->indentLimit = -1;
#endif
    return TRUE;
#ifdef CfScanner_INDENTATION
 errquit1:
    CcsBuffer_Destruct(&self->buffer);
#endif
 errquit0:
    return FALSE;
}

static CfScanInput_t *
CfScanInput(CfScanner_t * scanner, FILE * fp)
{
    CfScanInput_t * self;
    if (!(self = CcsMalloc(sizeof(CfScanInput_t)))) goto errquit0;
    self->fname = NULL;
    if (!CfScanInput_Init(self, scanner, fp)) goto errquit1;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
}

static CfScanInput_t *
CfScanInput_ByName(CfScanner_t * scanner, const CcsIncPathList_t * list,
		    const char * includer, const char * infn)
{
    FILE * fp;
    CfScanInput_t * self;
    char infnpath[PATH_MAX];
    if (!(fp = CcsIncPathList_Open(list, infnpath, sizeof(infnpath),
				   includer, infn)))
	goto errquit0;
    if (!(self = CcsMalloc(sizeof(CfScanInput_t) + strlen(infnpath) + 1)))
	goto errquit1;
    strcpy(self->fname = (char *)(self + 1), infnpath);
    if (!CfScanInput_Init(self, scanner, fp)) goto errquit2;
    return self;
 errquit2:
    CcsFree(self);
 errquit1:
    fclose(fp);
 errquit0:
    return NULL;
}

static void
CfScanInput_Destruct(CfScanInput_t * self)
{
    CcsToken_t * cur, * next;

#ifdef CfScanner_INDENTATION
    CcsFree(self->indent);
#endif
    for (cur = self->busyTokenList; cur; cur = next) {
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    CcsBuffer_Destruct(&self->buffer);
    if (self->fname) fclose(self->fp);
    CcsFree(self);
}

static void
CfScanInput_IncRef(CfScanInput_t * self)
{
    ++self->refcnt;
}

static void
CfScanInput_DecRef(CfScanInput_t * self)
{
    if (--self->refcnt > 0) return;
    CfScanInput_Destruct(self);
}

static CcsToken_t *
CfScanInput_NewToken0(CfScanInput_t * self, int kind,
		       int pos, int line, int col,
		       const char * val, size_t vallen)
{
    CcsToken_t * t;
    if ((t = CcsToken(self, kind, self->fname, pos, line, col, val, vallen)))
	CfScanInput_IncRef(self);
    return t;
}
#ifdef CfScanner_INDENTATION
static CcsToken_t *
CfScanInput_NewToken(CfScanInput_t * self, int kind)
{
    return CfScanInput_NewToken0(self, kind, self->pos,
				  self->line, self->col, NULL, 0);
}
#endif
static void
CfScanInput_GetCh(CfScanInput_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
#ifdef CfScanner_INDENTATION
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
CfScanInput_Scan(CfScanInput_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = CfScanInput_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

static CcsToken_t *
CfScanInput_Peek(CfScanInput_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = CfScanInput_NextToken(self);
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
CfScanInput_ResetPeek(CfScanInput_t * self)
{
    self->peekToken = self->curToken;
}

static void
CfScanInput_TokenIncRef(CfScanInput_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

static void
CfScanInput_TokenDecRef(CfScanInput_t * self, CcsToken_t * token)
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
	if (self->refcnt > 1) CfScanInput_DecRef(self);
	else {
	    CcsAssert(self->busyTokenList == NULL);
	    CfScanInput_DecRef(self);
	    return;
	}
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

#ifdef CfScanner_INDENTATION
static void
CfScanInput_IndentLimit(CfScanInput_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->kind == CfScanner_INDENT_IN);
    self->indentLimit = indentIn->loc.col;
}
#endif

static CcsPosition_t *
CfScanInput_GetPosition(CfScanInput_t * self, const CcsToken_t * begin,
			 const CcsToken_t * end)
{
    int len;
    CcsAssert(self == begin->input);
    CcsAssert(self == end->input);
    len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->loc.col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

static CcsPosition_t *
CfScanInput_GetPositionBetween(CfScanInput_t * self,
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
CfScanner_Init(CfScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    /*---- declarations ----*/
    self->eofSym = 0;
    self->maxT = 13;
    self->noSym = 13;
    /*---- enable ----*/
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

CfScanner_t *
CfScanner(CfScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CfScanInput(self, fp))) goto errquit0;
    if (!CfScanner_Init(self, errpool)) goto errquit1;
    CfScanInput_GetCh(self->cur);
    return self;
 errquit1:
    CfScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

CfScanner_t *
CfScanner_ByName(CfScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur = CfScanInput_ByName(self, NULL, NULL, fn)))
	goto errquit0;
    if (!CfScanner_Init(self, errpool)) goto errquit1;
    CfScanInput_GetCh(self->cur);
    return self;
 errquit1:
    CfScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

void
CfScanner_Destruct(CfScanner_t * self)
{
    CfScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
	CfScanInput_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
CfScanner_GetDummy(CfScanner_t * self)
{
    CfScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
CfScanner_Scan(CfScanner_t * self)
{
    CcsToken_t * token; CfScanInput_t * next;
    for (;;) {
	token = CfScanInput_Scan(self->cur);
	if (token->kind != self->eofSym) break;
	if (self->cur->next == NULL) break;
	CfScanInput_TokenDecRef(token->input, token);
	next = self->cur->next;
	CfScanInput_DecRef(self->cur);
	self->cur = next;
    }
    return token;
}

CcsToken_t *
CfScanner_Peek(CfScanner_t * self)
{
    CcsToken_t * token; CfScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = CfScanInput_Peek(self->cur);
	if (token->kind != self->eofSym) break;
	if (cur->next == NULL) break;
	CfScanInput_TokenDecRef(token->input, token);
	cur = cur->next;
    }
    return token;
}

void
CfScanner_ResetPeek(CfScanner_t * self)
{
    CfScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CfScanInput_ResetPeek(cur);
}

void
CfScanner_TokenIncRef(CfScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CfScanInput_TokenIncRef(token->input, token);
}

void
CfScanner_TokenDecRef(CfScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CfScanInput_TokenDecRef(token->input, token);
}

#ifdef CfScanner_INDENTATION
void
CfScanner_IndentLimit(CfScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    CfScanInput_IndentLimit(self->cur, indentIn);
}
#endif

CcsPosition_t *
CfScanner_GetPosition(CfScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CfScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
CfScanner_GetPositionBetween(CfScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CfScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsBool_t
CfScanner_Include(CfScanner_t * self, FILE * fp)
{
    CfScanInput_t * input;
    if (!(input = CfScanInput(self, fp))) return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

CcsBool_t
CfScanner_IncludeByName(CfScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn)
{
    CfScanInput_t * input;
    if (!(input = CfScanInput_ByName(self, list, self->cur->fname, infn)))
	return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

/*------------------------------- ScanInput --------------------------------*/
/* All the following things are used by CfScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 10, 10, 14 },	/* '\n' '\n' */
    { 13, 13, 13 },	/* '\r' '\r' */
    { 34, 34, 10 },	/* '"' '"' */
    { 35, 35, 19 },	/* '#' '#' */
    { 48, 48, 15 },	/* '0' '0' */
    { 49, 57, 8 },	/* '1' '9' */
    { 61, 61, 16 },	/* '=' '=' */
    { 67, 67, 1 },	/* 'C' 'C' */
    { 105, 105, 20 },	/* 'i' 'i' */
    { 109, 109, 18 },	/* 'm' 'm' */
    { 110, 110, 22 },	/* 'n' 'n' */
    { 115, 115, 25 },	/* 's' 's' */
    { 121, 121, 17 },	/* 'y' 'y' */
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

#ifdef CfScanner_KEYWORD_USED
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
#ifndef CfScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[CfScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > CfScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef CfScanner_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    return i2k ? i2k->val : defaultVal;
}

static int
GetKWKind(CfScanInput_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}
#endif /* CfScanner_KEYWORD_USED */

typedef struct {
    int ch, chBytes;
    int pos, line, col;
}  SLock_t;
static void
CfScanInput_LockCh(CfScanInput_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
CfScanInput_UnlockCh(CfScanInput_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
CfScanInput_ResetCh(CfScanInput_t * self, SLock_t * slock)
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
CfScanInput_Comment(CfScanInput_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	CfScanInput_LockCh(self, &slock); CfScanInput_GetCh(self);
	if (self->ch != c->start[1]) {
	    CfScanInput_ResetCh(self, &slock);
	    return FALSE;
	}
	CfScanInput_UnlockCh(self, &slock);
    }
    CfScanInput_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		CfScanInput_LockCh(self, &slock); CfScanInput_GetCh(self);
		if (self->ch == c->end[1]) {
		    CfScanInput_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    CfScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		CfScanInput_LockCh(self, &slock); CfScanInput_GetCh(self);
		if (self->ch == c->start[1]) {
		    CfScanInput_UnlockCh(self, &slock);
		    ++level;
		} else {
		    CfScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	CfScanInput_GetCh(self);
    }
    self->oldEols = self->line - line0;
    CfScanInput_GetCh(self);
    return TRUE;
}

#ifdef CfScanner_INDENTATION
static CcsToken_t *
CfScanInput_IndentGenerator(CfScanInput_t * self)
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
	    cur = CfScanInput_NewToken(self, CfScanner_INDENT_OUT);
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
	    newLen = (self->indentLast - self->indent) + CfScanner_INDENT_START;
	    newIndent = CcsRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = self->col;
	return CfScanInput_NewToken(self, CfScanner_INDENT_IN);
    }
    for (curIndent = self->indentUsed - 1; self->col < *curIndent; --curIndent);
    if (self->col > *curIndent)
	return CfScanInput_NewToken(self, CfScanner_INDENT_ERR);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CfScanInput_NewToken(self, CfScanner_INDENT_OUT);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
#endif

static CcsToken_t *
CfScanInput_NextToken(CfScanInput_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       /*---- scan1 ----*/
	       || self->ch == '\t'
	       /*---- enable ----*/
	       ) CfScanInput_GetCh(self);
#ifdef CfScanner_INDENTATION
	if ((t = CfScanInput_IndentGenerator(self))) return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		CfScanInput_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    CfScanInput_GetCh(self);
    kind = -2; /* Avoid gcc warning */
    switch (state) {
    case -1: kind = self->scanner->eofSym; break;
    case 0: kind = self->scanner->noSym; break;
    /*---- scan3 ----*/
    case 1:
	if (self->ch == 'O') {
	    CfScanInput_GetCh(self); goto case_2;
	} else { kind = self->scanner->noSym; break; }
    case 2: case_2:
	if (self->ch == 'N') {
	    CfScanInput_GetCh(self); goto case_3;
	} else { kind = self->scanner->noSym; break; }
    case 3: case_3:
	if (self->ch == 'F') {
	    CfScanInput_GetCh(self); goto case_4;
	} else { kind = self->scanner->noSym; break; }
    case 4: case_4:
	if (self->ch == 'I') {
	    CfScanInput_GetCh(self); goto case_5;
	} else { kind = self->scanner->noSym; break; }
    case 5: case_5:
	if (self->ch == 'G') {
	    CfScanInput_GetCh(self); goto case_6;
	} else { kind = self->scanner->noSym; break; }
    case 6: case_6:
	if (self->ch == '_') {
	    CfScanInput_GetCh(self); goto case_7;
	} else { kind = self->scanner->noSym; break; }
    case 7: case_7:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'Z') ||
	    self->ch == '_' ||
	    (self->ch >= 'a' && self->ch <= 'z')) {
	    CfScanInput_GetCh(self); goto case_7;
	} else { kind = 1; break; }
    case 8: case_8:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    CfScanInput_GetCh(self); goto case_8;
	} else { kind = 2; break; }
    case 9: case_9:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'F') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    CfScanInput_GetCh(self); goto case_9;
	} else { kind = 3; break; }
    case 10: case_10:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '!') ||
	    (self->ch >= '#' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    CfScanInput_GetCh(self); goto case_10;
	} else if (self->ch == '"') {
	    CfScanInput_GetCh(self); goto case_12;
	} else if (self->ch == '\\') {
	    CfScanInput_GetCh(self); goto case_11;
	} else { kind = self->scanner->noSym; break; }
    case 11: case_11:
	if ((self->ch >= ' ' && self->ch <= '~')) {
	    CfScanInput_GetCh(self); goto case_10;
	} else { kind = self->scanner->noSym; break; }
    case 12: case_12:
	{ kind = 4; break; }
    case 13:
	if (self->ch == '\n') {
	    CfScanInput_GetCh(self); goto case_14;
	} else { kind = self->scanner->noSym; break; }
    case 14: case_14:
	{ kind = 5; break; }
    case 15:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    CfScanInput_GetCh(self); goto case_8;
	} else if (self->ch == 'x') {
	    CfScanInput_GetCh(self); goto case_9;
	} else { kind = 2; break; }
    case 16:
	{ kind = 6; break; }
    case 17:
	{ kind = 7; break; }
    case 18:
	{ kind = 8; break; }
    case 19:
	{ kind = 9; break; }
    case 20:
	if (self->ch == 's') {
	    CfScanInput_GetCh(self); goto case_21;
	} else { kind = self->scanner->noSym; break; }
    case 21: case_21:
	{ kind = 10; break; }
    case 22:
	if (self->ch == 'o') {
	    CfScanInput_GetCh(self); goto case_23;
	} else { kind = self->scanner->noSym; break; }
    case 23: case_23:
	if (self->ch == 't') {
	    CfScanInput_GetCh(self); goto case_24;
	} else { kind = self->scanner->noSym; break; }
    case 24: case_24:
	{ kind = 11; break; }
    case 25:
	if (self->ch == 'e') {
	    CfScanInput_GetCh(self); goto case_26;
	} else { kind = self->scanner->noSym; break; }
    case 26: case_26:
	if (self->ch == 't') {
	    CfScanInput_GetCh(self); goto case_27;
	} else { kind = self->scanner->noSym; break; }
    case 27: case_27:
	{ kind = 12; break; }
    /*---- enable ----*/
    }
    CcsAssert(kind != -2);
    t = CfScanInput_NewToken0(self, kind, pos, line, col,
			       CcsBuffer_GetString(&self->buffer,
						   pos, self->pos - pos),
			       self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
