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
struct JsonScanInput_s {
    JsonScanInput_t * next;

    int              refcnt;
    JsonScanner_t   * scanner;
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

#ifdef JsonScanner_INDENTATION
    CcsBool_t        lineStart;
    int            * indent;
    int            * indentUsed;
    int            * indentLast;
    int              indentLimit;
#endif
};

static CcsToken_t * JsonScanInput_NextToken(JsonScanInput_t * self);

static CcsBool_t
JsonScanInput_Init(JsonScanInput_t * self, JsonScanner_t * scanner, FILE * fp)
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
#ifdef JsonScanner_INDENTATION
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * JsonScanner_INDENT_START)))
	goto errquit1;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + JsonScanner_INDENT_START;
    *self->indentUsed++ = 0;
    self->indentLimit = -1;
#endif
    return TRUE;
#ifdef JsonScanner_INDENTATION
 errquit1:
    CcsBuffer_Destruct(&self->buffer);
#endif
 errquit0:
    return FALSE;
}

static JsonScanInput_t *
JsonScanInput(JsonScanner_t * scanner, FILE * fp)
{
    JsonScanInput_t * self;
    if (!(self = CcsMalloc(sizeof(JsonScanInput_t)))) goto errquit0;
    self->fname = NULL;
    if (!JsonScanInput_Init(self, scanner, fp)) goto errquit1;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
}

static JsonScanInput_t *
JsonScanInput_ByName(JsonScanner_t * scanner, const CcsIncPathList_t * list,
		    const char * includer, const char * infn)
{
    FILE * fp;
    JsonScanInput_t * self;
    char infnpath[PATH_MAX];
    if (!(fp = CcsIncPathList_Open(list, infnpath, sizeof(infnpath),
				   includer, infn)))
	goto errquit0;
    if (!(self = CcsMalloc(sizeof(JsonScanInput_t) + strlen(infnpath) + 1)))
	goto errquit1;
    strcpy(self->fname = (char *)(self + 1), infnpath);
    if (!JsonScanInput_Init(self, scanner, fp)) goto errquit2;
    return self;
 errquit2:
    CcsFree(self);
 errquit1:
    fclose(fp);
 errquit0:
    return NULL;
}

static void
JsonScanInput_Destruct(JsonScanInput_t * self)
{
    CcsToken_t * cur, * next;

#ifdef JsonScanner_INDENTATION
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
JsonScanInput_GetCh(JsonScanInput_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
#ifdef JsonScanner_INDENTATION
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
JsonScanInput_Scan(JsonScanInput_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = JsonScanInput_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

static CcsToken_t *
JsonScanInput_Peek(JsonScanInput_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = JsonScanInput_NextToken(self);
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
JsonScanInput_ResetPeek(JsonScanInput_t * self)
{
    self->peekToken = self->curToken;
}

static void
JsonScanInput_TokenIncRef(JsonScanInput_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

static void
JsonScanInput_TokenDecRef(JsonScanInput_t * self, CcsToken_t * token)
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

#ifdef JsonScanner_INDENTATION
static void
JsonScanInput_IndentLimit(JsonScanInput_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->kind == JsonScanner_INDENT_IN);
    self->indentLimit = indentIn->loc.col;
}
#endif

static CcsPosition_t *
JsonScanInput_GetPosition(JsonScanInput_t * self, const CcsToken_t * begin,
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
JsonScanInput_GetPositionBetween(JsonScanInput_t * self,
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
JsonScanner_Init(JsonScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    /*---- declarations ----*/
    self->eofSym = 0;
    self->maxT = 12;
    self->noSym = 12;
    /*---- enable ----*/
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

JsonScanner_t *
JsonScanner(JsonScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = JsonScanInput(self, fp))) goto errquit0;
    if (!JsonScanner_Init(self, errpool)) goto errquit1;
    JsonScanInput_GetCh(self->cur);
    return self;
 errquit1:
    JsonScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

JsonScanner_t *
JsonScanner_ByName(JsonScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur = JsonScanInput_ByName(self, NULL, NULL, fn)))
	goto errquit0;
    if (!JsonScanner_Init(self, errpool)) goto errquit1;
    JsonScanInput_GetCh(self->cur);
    return self;
 errquit1:
    JsonScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

void
JsonScanner_Destruct(JsonScanner_t * self)
{
    JsonScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	JsonScanInput_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
JsonScanner_GetDummy(JsonScanner_t * self)
{
    JsonScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
JsonScanner_Scan(JsonScanner_t * self)
{
    CcsToken_t * token; JsonScanInput_t * next;
    for (;;) {
	token = JsonScanInput_Scan(self->cur);
	if (token->kind != self->eofSym) break;
	if (self->cur->next == NULL) break;
	JsonScanInput_TokenDecRef(token->input, token);
	next = self->cur->next;
	JsonScanInput_Destruct(self->cur);
	self->cur = next;
    }
    return token;
}

CcsToken_t *
JsonScanner_Peek(JsonScanner_t * self)
{
    CcsToken_t * token; JsonScanInput_t * cur;
    cur = self->cur;
    for (;;) {
	token = JsonScanInput_Peek(self->cur);
	if (token->kind != self->eofSym) break;
	if (cur->next == NULL) break;
	JsonScanInput_TokenDecRef(token->input, token);
	cur = cur->next;
    }
    return token;
}

void
JsonScanner_ResetPeek(JsonScanner_t * self)
{
    JsonScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	JsonScanInput_ResetPeek(cur);
}

void
JsonScanner_TokenIncRef(JsonScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else JsonScanInput_TokenIncRef(token->input, token);
}

void
JsonScanner_TokenDecRef(JsonScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else JsonScanInput_TokenDecRef(token->input, token);
}

#ifdef JsonScanner_INDENTATION
void
JsonScanner_IndentLimit(JsonScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    JsonScanInput_IndentLimit(self->cur, indentIn);
}
#endif

CcsPosition_t *
JsonScanner_GetPosition(JsonScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return JsonScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
JsonScanner_GetPositionBetween(JsonScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return JsonScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsBool_t
JsonScanner_Include(JsonScanner_t * self, FILE * fp)
{
    JsonScanInput_t * input;
    if (!(input = JsonScanInput(self, fp))) return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

CcsBool_t
JsonScanner_IncludeByName(JsonScanner_t * self, const CcsIncPathList_t * list,
			 const char * infn)
{
    JsonScanInput_t * input;
    if (!(input = JsonScanInput_ByName(self, list, self->cur->fname, infn)))
	return FALSE;
    input->next = self->cur;
    self->cur = input;
    return TRUE;
}

/*------------------------------- ScanInput --------------------------------*/
/* All the following things are used by JsonScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 34, 34, 8 },	/* '"' '"' */
    { 44, 44, 17 },	/* ',' ',' */
    { 45, 45, 1 },	/* '-' '-' */
    { 48, 48, 2 },	/* '0' '0' */
    { 49, 57, 7 },	/* '1' '9' */
    { 58, 58, 18 },	/* ':' ':' */
    { 91, 91, 19 },	/* '[' '[' */
    { 93, 93, 20 },	/* ']' ']' */
    { 102, 102, 25 },	/* 'f' 'f' */
    { 110, 110, 30 },	/* 'n' 'n' */
    { 116, 116, 21 },	/* 't' 't' */
    { 123, 123, 15 },	/* '{' '{' */
    { 125, 125, 16 },	/* '}' '}' */
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

#ifdef JsonScanner_KEYWORD_USED
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
#ifndef JsonScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[JsonScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > JsonScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef JsonScanner_CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    return i2k ? i2k->val : defaultVal;
}

static int
GetKWKind(JsonScanInput_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}
#endif /* JsonScanner_KEYWORD_USED */

typedef struct {
    int ch, chBytes;
    int pos, line, col;
}  SLock_t;
static void
JsonScanInput_LockCh(JsonScanInput_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
JsonScanInput_UnlockCh(JsonScanInput_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
JsonScanInput_ResetCh(JsonScanInput_t * self, SLock_t * slock)
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
JsonScanInput_Comment(JsonScanInput_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	JsonScanInput_LockCh(self, &slock); JsonScanInput_GetCh(self);
	if (self->ch != c->start[1]) {
	    JsonScanInput_ResetCh(self, &slock);
	    return FALSE;
	}
	JsonScanInput_UnlockCh(self, &slock);
    }
    JsonScanInput_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		JsonScanInput_LockCh(self, &slock); JsonScanInput_GetCh(self);
		if (self->ch == c->end[1]) {
		    JsonScanInput_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    JsonScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		JsonScanInput_LockCh(self, &slock); JsonScanInput_GetCh(self);
		if (self->ch == c->start[1]) {
		    JsonScanInput_UnlockCh(self, &slock);
		    ++level;
		} else {
		    JsonScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	JsonScanInput_GetCh(self);
    }
    self->oldEols = self->line - line0;
    JsonScanInput_GetCh(self);
    return TRUE;
}

#ifdef JsonScanner_INDENTATION
static CcsToken_t *
JsonScanInput_IndentGenerator(JsonScanInput_t * self)
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
	    cur = CcsToken(self, JsonScanner_INDENT_OUT, self->fname, self->pos,
			   self->line, self->col, NULL, 0);
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
	    newLen = (self->indentLast - self->indent) + JsonScanner_INDENT_START;
	    newIndent = CcsRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = self->col;
	return CcsToken(self, JsonScanner_INDENT_IN, self->fname, self->pos,
			self->line, self->col, NULL, 0);
    }
    for (curIndent = self->indentUsed - 1; self->col < *curIndent; --curIndent);
    if (self->col > *curIndent)
	return CcsToken(self, JsonScanner_INDENT_ERR, self->fname, self->pos,
			self->line, self->col, NULL, 0);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CcsToken(self, JsonScanner_INDENT_OUT, self->fname, self->pos,
		       self->line, self->col, NULL, 0);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
#endif

static CcsToken_t *
JsonScanInput_NextToken(JsonScanInput_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       /*---- scan1 ----*/
	       || (self->ch >= '\t' && self->ch <= '\n')
	       || self->ch == '\r'
	       /*---- enable ----*/
	       ) JsonScanInput_GetCh(self);
#ifdef JsonScanner_INDENTATION
	if ((t = JsonScanInput_IndentGenerator(self))) return t;
#endif
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		JsonScanInput_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    JsonScanInput_GetCh(self);
    kind = -2; /* Avoid gcc warning */
    switch (state) {
    case -1: kind = self->scanner->eofSym; break;
    case 0: kind = self->scanner->noSym; break;
    /*---- scan3 ----*/
    case 1:
	if ((self->ch >= '1' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_7;
	} else if (self->ch == '0') {
	    JsonScanInput_GetCh(self); goto case_2;
	} else { kind = self->scanner->noSym; break; }
    case 2: case_2:
	if (self->ch == 'E' ||
	    self->ch == 'e') {
	    JsonScanInput_GetCh(self); goto case_3;
	} else if (self->ch == '.') {
	    JsonScanInput_GetCh(self); goto case_6;
	} else { kind = 1; break; }
    case 3: case_3:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_5;
	} else if (self->ch == '+' ||
	    self->ch == '-') {
	    JsonScanInput_GetCh(self); goto case_4;
	} else { kind = self->scanner->noSym; break; }
    case 4: case_4:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_5;
	} else { kind = self->scanner->noSym; break; }
    case 5: case_5:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_5;
	} else { kind = 1; break; }
    case 6: case_6:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_6;
	} else if (self->ch == 'E' ||
	    self->ch == 'e') {
	    JsonScanInput_GetCh(self); goto case_3;
	} else { kind = 1; break; }
    case 7: case_7:
	if ((self->ch >= '0' && self->ch <= '9')) {
	    JsonScanInput_GetCh(self); goto case_7;
	} else if (self->ch == 'E' ||
	    self->ch == 'e') {
	    JsonScanInput_GetCh(self); goto case_3;
	} else if (self->ch == '.') {
	    JsonScanInput_GetCh(self); goto case_6;
	} else { kind = 1; break; }
    case 8: case_8:
	if ((self->ch >= 0 && self->ch <= '\t') ||
	    (self->ch >= '\v' && self->ch <= '\f') ||
	    (self->ch >= 14 && self->ch <= '!') ||
	    (self->ch >= '#' && self->ch <= '[') ||
	    (self->ch >= ']' && self->ch <= 65535)) {
	    JsonScanInput_GetCh(self); goto case_8;
	} else if (self->ch == '"') {
	    JsonScanInput_GetCh(self); goto case_13;
	} else if (self->ch == '\\') {
	    JsonScanInput_GetCh(self); goto case_14;
	} else { kind = self->scanner->noSym; break; }
    case 9: case_9:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'F') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    JsonScanInput_GetCh(self); goto case_10;
	} else { kind = self->scanner->noSym; break; }
    case 10: case_10:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'F') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    JsonScanInput_GetCh(self); goto case_11;
	} else { kind = self->scanner->noSym; break; }
    case 11: case_11:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'F') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    JsonScanInput_GetCh(self); goto case_12;
	} else { kind = self->scanner->noSym; break; }
    case 12: case_12:
	if ((self->ch >= '0' && self->ch <= '9') ||
	    (self->ch >= 'A' && self->ch <= 'F') ||
	    (self->ch >= 'a' && self->ch <= 'f')) {
	    JsonScanInput_GetCh(self); goto case_8;
	} else { kind = self->scanner->noSym; break; }
    case 13: case_13:
	{ kind = 2; break; }
    case 14: case_14:
	if (self->ch == '"' ||
	    self->ch == '/' ||
	    self->ch == '\\' ||
	    self->ch == 'b' ||
	    self->ch == 'f' ||
	    self->ch == 'n' ||
	    self->ch == 'r' ||
	    self->ch == 't') {
	    JsonScanInput_GetCh(self); goto case_8;
	} else if (self->ch == 'u') {
	    JsonScanInput_GetCh(self); goto case_9;
	} else { kind = self->scanner->noSym; break; }
    case 15:
	{ kind = 3; break; }
    case 16:
	{ kind = 4; break; }
    case 17:
	{ kind = 5; break; }
    case 18:
	{ kind = 6; break; }
    case 19:
	{ kind = 7; break; }
    case 20:
	{ kind = 8; break; }
    case 21:
	if (self->ch == 'r') {
	    JsonScanInput_GetCh(self); goto case_22;
	} else { kind = self->scanner->noSym; break; }
    case 22: case_22:
	if (self->ch == 'u') {
	    JsonScanInput_GetCh(self); goto case_23;
	} else { kind = self->scanner->noSym; break; }
    case 23: case_23:
	if (self->ch == 'e') {
	    JsonScanInput_GetCh(self); goto case_24;
	} else { kind = self->scanner->noSym; break; }
    case 24: case_24:
	{ kind = 9; break; }
    case 25:
	if (self->ch == 'a') {
	    JsonScanInput_GetCh(self); goto case_26;
	} else { kind = self->scanner->noSym; break; }
    case 26: case_26:
	if (self->ch == 'l') {
	    JsonScanInput_GetCh(self); goto case_27;
	} else { kind = self->scanner->noSym; break; }
    case 27: case_27:
	if (self->ch == 's') {
	    JsonScanInput_GetCh(self); goto case_28;
	} else { kind = self->scanner->noSym; break; }
    case 28: case_28:
	if (self->ch == 'e') {
	    JsonScanInput_GetCh(self); goto case_29;
	} else { kind = self->scanner->noSym; break; }
    case 29: case_29:
	{ kind = 10; break; }
    case 30:
	if (self->ch == 'u') {
	    JsonScanInput_GetCh(self); goto case_31;
	} else { kind = self->scanner->noSym; break; }
    case 31: case_31:
	if (self->ch == 'l') {
	    JsonScanInput_GetCh(self); goto case_32;
	} else { kind = self->scanner->noSym; break; }
    case 32: case_32:
	if (self->ch == 'l') {
	    JsonScanInput_GetCh(self); goto case_33;
	} else { kind = self->scanner->noSym; break; }
    case 33: case_33:
	{ kind = 11; break; }
    /*---- enable ----*/
    }
    CcsAssert(kind != -2);
    t = CcsToken(self, kind, self->fname, pos, line, col,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
