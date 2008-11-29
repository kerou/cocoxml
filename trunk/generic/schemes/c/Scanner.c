/*---- license ----*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Scanner.h"

/*---- defines ----*/
#define CASE_SENSITIVE
/*---- enable ----*/

static int Char2State(int chr);
static int Identifier2KWKind(const char * key, size_t keylen, int defaultVal);
static void CcsScanner_Init(CcsScanner_t * self);
static CcsToken_t * CcsScanner_NextCcsToken(CcsScanner_t * self);
static void CcsScanner_GetCh(CcsScanner_t * self);

static const char * dummyval = "dummy";
CcsScanner_t *
CcsScanner(CcsScanner_t * self, CcsGlobals_t * globals, const char * filename)
{
    FILE * fp;
    self->globals = globals;
    if (!(fp = fopen(filename, "r"))) {
	fprintf(stderr, "Can not open '%s'.\n", filename);
	goto errquit0;
    }
    if (!(self->dummyToken = CcsToken(0, 0, 0, 0, dummyval, strlen(dummyval))))
	goto errquit1;
    if (CcsBuffer(&self->buffer, fp) == NULL) goto errquit2;
    CcsScanner_Init(self);
    return self;
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
    self->maxT = 44;
    self->noSym = 44;
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
    for (cur = self->busyTokenList; cur; cur = next) {
	next = cur->next;
	CcsToken_Destruct(cur);
    }
    CcsToken_Destruct(self->dummyToken);
    CcsBuffer_Destruct(&self->buffer);
}

CcsToken_t *
CcsScanner_GetDummy(CcsScanner_t * self)
{
    return self->dummyToken;
}

CcsToken_t *
CcsScanner_Scan(CcsScanner_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = CcsScanner_NextCcsToken(self);
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
	    *self->peekToken = CcsScanner_NextCcsToken(self);
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
    *self->peekToken = *self->curToken;
}

void
CcsScanner_IncRef(CcsScanner_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
CcsScanner_DecRef(CcsScanner_t * self, CcsToken_t * token)
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
CcsScanner_GetPosition(CcsScanner_t * self,
		       CcsToken_t * begin, CcsToken_t * end)
{
    int len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

CcsPosition_t *
CcsScanner_GetPositionWithTail(CcsScanner_t * self,
			       CcsToken_t * begin, CcsToken_t * end)
{
    int len = (end->pos + strlen(end->val)) - begin->pos;
    return CcsPosition(begin->pos, len, begin->col,
		    CcsBuffer_GetString(&self->buffer, begin->pos, len));
}


/* All the following things are used by CcsScanner_NextCcsToken. */
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
    { "ANY", 26 },
    { "CHARACTERS", 11 },
    { "COMMENTS", 14 },
    { "COMPILER", 6 },
    { "CONSTRUCTOR", 8 },
    { "CONTEXT", 41 },
    { "DESTRUCTOR", 9 },
    { "END", 22 },
    { "FROM", 15 },
    { "IF", 40 },
    { "IGNORE", 18 },
    { "IGNORECASE", 10 },
    { "MEMBERS", 7 },
    { "NESTED", 17 },
    { "PRAGMAS", 13 },
    { "PRODUCTIONS", 19 },
    { "SYNC", 39 },
    { "TO", 16 },
    { "TOKENS", 12 },
    { "WEAK", 32 },
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
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL= 1;
    } else {
	if (self->ch == '\n') {
	    if (self->oldEolsEOL) self->oldEolsEOL = 0;
	    else {
		++self->line; self->col = 0;
	    }
	} else {
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

CcsToken_t *
CcsScanner_NextCcsToken(CcsScanner_t * self)
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
	for (curComment = comments; curComment < commentsLast; ++curComment)
	    if (self->ch == curComment->start[0] &&
		CcsScanner_Comment(self, curComment)) break;
	if (curComment < commentsLast) continue;
	break;
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
	} else { kind = 45; break; }
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
	{ kind = 20; break; }
    case 14:
	{ kind = 23; break; }
    case 15:
	{ kind = 24; break; }
    case 16: case_16:
	{ kind = 25; break; }
    case 17:
	{ kind = 28; break; }
    case 18: case_18:
	{ kind = 29; break; }
    case 19: case_19:
	{ kind = 30; break; }
    case 20:
	{ kind = 31; break; }
    case 21:
	{ kind = 34; break; }
    case 22:
	{ kind = 35; break; }
    case 23:
	{ kind = 36; break; }
    case 24:
	{ kind = 37; break; }
    case 25:
	{ kind = 38; break; }
    case 26: case_26:
	{ kind = 42; break; }
    case 27: case_27:
	{ kind = 43; break; }
    case 28:
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_16;
	} else if (self->ch == '>') {
	    CcsScanner_GetCh(self); goto case_19;
	} else if (self->ch == ')') {
	    CcsScanner_GetCh(self); goto case_27;
	} else { kind = 21; break; }
    case 29:
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_18;
	} else { kind = 27; break; }
    case 30:
	if (self->ch == '.') {
	    CcsScanner_GetCh(self); goto case_26;
	} else { kind = 33; break; }
    /*---- enable ----*/
    }
    t = CcsToken(kind, pos, col, line,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}
