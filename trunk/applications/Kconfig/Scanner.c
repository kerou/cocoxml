/*---- license ----*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Scanner.h"

/*---- defines ----*/
/*---- enable ----*/

static int Char2State(int chr);
static int Identifier2KWKind(const char * key, size_t keylen, int defaultVal);
static void KcScanner_Init(KcScanner_t * self);
static CcsToken_t * KcScanner_NextToken(KcScanner_t * self);
static void KcScanner_GetCh(KcScanner_t * self);

static const char * dummyval = "dummy";
KcScanner_t *
KcScanner(KcScanner_t * self, CcsErrorPool_t * errpool,
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
    KcScanner_Init(self);
    return self;
 errquit2:
    fclose(fp);
 errquit1:
    CcsToken_Destruct(self->dummyToken);
 errquit0:
    return NULL;
}

static void
KcScanner_Init(KcScanner_t * self)
{
    /*---- declarations ----*/
    /*---- enable ----*/

    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
    KcScanner_GetCh(self);
}

void
KcScanner_Destruct(KcScanner_t * self)
{
    CcsToken_t * cur, * next;
    for (cur = self->busyTokenList; cur; cur = next) {
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    CcsToken_Destruct(self->dummyToken);
    CcsBuffer_Destruct(&self->buffer);
}

CcsToken_t *
KcScanner_GetDummy(KcScanner_t * self)
{
    return self->dummyToken;
}

CcsToken_t *
KcScanner_Scan(KcScanner_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = KcScanner_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

CcsToken_t *
KcScanner_Peek(KcScanner_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = KcScanner_NextToken(self);
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
KcScanner_ResetPeek(KcScanner_t * self)
{
    *self->peekToken = *self->curToken;
}

void
KcScanner_IncRef(KcScanner_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
KcScanner_DecRef(KcScanner_t * self, CcsToken_t * token)
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
KcScanner_GetPosition(KcScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    int len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

CcsPosition_t *
KcScanner_GetPositionBetween(KcScanner_t * self, const CcsToken_t * begin,
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

/* All the following things are used by KcScanner_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
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
#ifndef CASE_SENSITIVE
    char * cur;
#endif
    Identifier2KWKind_t * i2k;

    if (!(keystr = CcsMalloc(keylen + 1))) exit(-1);
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef CASE_SENSITIVE
    for (cur = keystr; *cur; ++cur) *cur = tolower(*cur);
#endif
    i2k = bsearch(keystr, i2kArr, i2kNum, sizeof(Identifier2KWKind_t), i2kCmp);
    CcsFree(keystr);
    return i2k ? i2k->val : defaultVal;
}

static int
KcScanner_GetKWKind(KcScanner_t * self, int start, int end, int defaultVal)
{
    return Identifier2KWKind(CcsBuffer_GetString(&self->buffer,
						 start, end - start),
			     end - start, defaultVal);
}

static void
KcScanner_GetCh(KcScanner_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL= 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
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
KcScanner_LockCh(KcScanner_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
KcScanner_UnlockCh(KcScanner_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
KcScanner_ResetCh(KcScanner_t * self, SLock_t * slock)
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
KcScanner_Comment(KcScanner_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	KcScanner_LockCh(self, &slock); KcScanner_GetCh(self);
	if (self->ch != c->start[1]) {
	    KcScanner_ResetCh(self, &slock);
	    return FALSE;
	}
	KcScanner_UnlockCh(self, &slock);
    }
    KcScanner_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		KcScanner_LockCh(self, &slock); KcScanner_GetCh(self);
		if (self->ch == c->end[1]) {
		    KcScanner_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    KcScanner_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		KcScanner_LockCh(self, &slock); KcScanner_GetCh(self);
		if (self->ch == c->start[1]) {
		    KcScanner_UnlockCh(self, &slock);
		    ++level;
		} else {
		    KcScanner_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	KcScanner_GetCh(self);
    }
    self->oldEols = self->line - line0;
    KcScanner_GetCh(self);
    return TRUE;
}

CcsToken_t *
KcScanner_NextToken(KcScanner_t * self)
{
    int pos, line, col, state, kind; CcsToken_t * t;
    const CcsComment_t * curComment;
    for (;;) {
	while (self->ch == ' '
	       || self->ch == '\r'
	       /*---- enable ----*/
	       ) KcScanner_GetCh(self);
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		KcScanner_Comment(self, curComment)) break;
	if (curComment >= commentsLast) break;
    }
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    state = Char2State(self->ch);
    KcScanner_GetCh(self);
    switch (state) {
    case -1: kind = self->eofSym; break;
    case 0: kind = self->noSym; break;
    /*---- scan3 ----*/
    /*---- enable ----*/
    }
    t = CcsToken(kind, pos, col, line,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
