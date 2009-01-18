/*---- license ----*/
/*-------------------------------------------------------------------------
Kconfig.atg
Copyright (C) 2008, Charles Wang
Author: Charles Wang  <charlesw123456@gmail.com>
License: LGPLv2
-------------------------------------------------------------------------*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Scanner.h"
#include  "c/ScanInput.h"
#include  "c/Indent.h"

static CcsBool_t KcScanner_AddInit(void * additional, void * scanner);
static void KcScanner_AddDestruct(void * additional);
static CcsToken_t * KcScanner_Skip(void * scanner, CcsScanInput_t * input);
static int KcScanner_Kind(void * scanner, CcsScanInput_t * input);

static const CcsSI_Info_t Scanner_Info = {
    /*---- declarations ----*/
    24, /* additionalSpace */
    0, /* eofSym */
    43, /* maxT */
    43, /* noSym */
    /*---- enable ----*/
    KcScanner_AddInit,
    KcScanner_AddDestruct,
    KcScanner_Skip,
    KcScanner_Kind
};

#ifdef KcScanner_INDENTATION
static const CcsIndentInfo_t Scanner_IndentInfo = {
    KcScanner_INDENT_IN, KcScanner_INDENT_OUT, KcScanner_INDENT_ERR
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
KcScanner_Init(KcScanner_t * self, CcsErrorPool_t * errpool) {
    self->errpool = errpool;
    if (!(self->dummyToken =
	  CcsToken(NULL, 0, NULL, 0, 0, 0, dummyval, strlen(dummyval))))
	return FALSE;
    return TRUE;
}

KcScanner_t *
KcScanner(KcScanner_t * self, CcsErrorPool_t * errpool, FILE * fp)
{
    if (!(self->cur = CcsScanInput(self, &Scanner_Info, fp)))
	goto errquit0;
    if (!KcScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

KcScanner_t *
KcScanner_ByName(KcScanner_t * self, CcsErrorPool_t * errpool,
		  const char * fn)
{
    if (!(self->cur =
	  CcsScanInput_ByName(self, &Scanner_Info, NULL, NULL, fn)))
	goto errquit0;
    if (!KcScanner_Init(self, errpool)) goto errquit1;
    CcsGetCh(self->cur);
    return self;
 errquit1:
    CcsScanInput_Destruct(self->cur);
 errquit0:
    return NULL;
}

void
KcScanner_Destruct(KcScanner_t * self)
{
    CcsScanInput_t * cur, * next;
    for (cur = self->cur; cur; cur = next) {
	next = cur->next;
	/* May be trigged by .atg semantic code. */
	CcsAssert(cur->refcnt == 1);
	CcsScanInput_Destruct(cur);
    }
    /* May be trigged by .atg semantic code. */
    CcsAssert(self->dummyToken->refcnt == 1);
    CcsToken_Destruct(self->dummyToken);
}

CcsToken_t *
KcScanner_GetDummy(KcScanner_t * self)
{
    KcScanner_TokenIncRef(self, self->dummyToken);
    return self->dummyToken;
}

CcsToken_t *
KcScanner_Scan(KcScanner_t * self)
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
KcScanner_Peek(KcScanner_t * self)
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
KcScanner_ResetPeek(KcScanner_t * self)
{
    CcsScanInput_t * cur;
    for (cur = self->cur; cur; cur = cur->next)
	CcsScanInput_ResetPeek(cur);
}

void
KcScanner_TokenIncRef(KcScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) ++token->refcnt;
    else CcsScanInput_TokenIncRef(token->input, token);
}

void
KcScanner_TokenDecRef(KcScanner_t * self, CcsToken_t * token)
{
    if (token == self->dummyToken) --token->refcnt;
    else CcsScanInput_TokenDecRef(token->input, token);
}

#ifdef KcScanner_INDENTATION
void
KcScanner_IndentLimit(KcScanner_t * self, const CcsToken_t * indentIn)
{
    CcsAssert(indentIn->input == self->cur);
    CcsAssert(indentIn->kind == KcScanner_INDENT_IN);
    CcsIndent_SetLimit((CcsIndent_t *)(self->cur + 1), indentIn);
}
#endif

CcsPosition_t *
KcScanner_GetPosition(KcScanner_t * self, const CcsToken_t * begin,
		       const CcsToken_t * end)
{
    return CcsScanInput_GetPosition(begin->input, begin, end);
}

CcsPosition_t *
KcScanner_GetPositionBetween(KcScanner_t * self, const CcsToken_t * begin,
			      const CcsToken_t * end)
{
    return CcsScanInput_GetPositionBetween(begin->input, begin, end);
}

CcsBool_t
KcScanner_Include(KcScanner_t * self, FILE * fp, CcsToken_t ** token)
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
KcScanner_IncludeByName(KcScanner_t * self, const CcsIncPathList_t * list,
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

/* All the following things are used by CcsScanInput_NextToken. */
typedef struct {
    int keyFrom;
    int keyTo;
    int val;
}  Char2State_t;

static const Char2State_t c2sArr[] = {
    /*---- chars2states ----*/
    { EoF, EoF, -1 },
    { 10, 10, 6 },	/* '\n' '\n' */
    { 13, 13, 5 },	/* '\r' '\r' */
    { 33, 33, 25 },	/* '!' '!' */
    { 34, 34, 2 },	/* '"' '"' */
    { 38, 38, 20 },	/* '&' '&' */
    { 40, 40, 22 },	/* '(' '(' */
    { 41, 41, 23 },	/* ')' ')' */
    { 45, 45, 8 },	/* '-' '-' */
    { 48, 57, 1 },	/* '0' '9' */
    { 61, 61, 7 },	/* '=' '=' */
    { 65, 90, 1 },	/* 'A' 'Z' */
    { 95, 95, 1 },	/* '_' '_' */
    { 97, 122, 1 },	/* 'a' 'z' */
    { 124, 124, 18 },	/* '|' '|' */
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

#ifdef KcScanner_KEYWORD_USED
typedef struct {
    const char * key;
    int val;
}  Identifier2KWKind_t;

static const Identifier2KWKind_t i2kArr[] = {
    /*---- identifiers2keywordkinds ----*/
    { "bool", 18 },
    { "choice", 11 },
    { "comment", 13 },
    { "config", 7 },
    { "def_bool", 25 },
    { "def_tristate", 26 },
    { "default", 24 },
    { "defconfig_list", 34 },
    { "depends", 27 },
    { "endchoice", 12 },
    { "endif", 15 },
    { "endmenu", 10 },
    { "env", 32 },
    { "help", 35 },
    { "hex", 21 },
    { "if", 14 },
    { "int", 22 },
    { "mainmenu", 16 },
    { "menu", 9 },
    { "menuconfig", 8 },
    { "on", 28 },
    { "option", 31 },
    { "prompt", 23 },
    { "range", 30 },
    { "select", 29 },
    { "source", 17 },
    { "string", 20 },
    { "tristate", 19 },
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
#ifndef KcScanner_CASE_SENSITIVE
    char * cur;
#endif
    char keystr[KcScanner_MAX_KEYWORD_LEN + 1];
    Identifier2KWKind_t * i2k;

    if (keylen > KcScanner_MAX_KEYWORD_LEN) return defaultVal;
    memcpy(keystr, key, keylen);
    keystr[keylen] = 0;
#ifndef KcScanner_CASE_SENSITIVE
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
#endif /* KcScanner_KEYWORD_USED */

static CcsBool_t
KcScanner_AddInit(void * additional, void * scanner)
{
#ifdef KcScanner_INDENTATION
    if (!CcsIndent_Init(additional, &Scanner_IndentInfo)) return FALSE;
#endif
    return TRUE;
}

static void
KcScanner_AddDestruct(void * additional)
{
#ifdef KcScanner_INDENTATION
    CcsIndent_Destruct(additional);
#endif
}

static const CcsComment_t comments[] = {
/*---- comments ----*/
    { { '#', 0 }, { '\n', 0 }, FALSE },
/*---- enable ----*/
};
static const CcsComment_t * commentsLast =
    comments + sizeof(comments) / sizeof(comments[0]);

static CcsToken_t *
KcScanner_Skip(void * scanner, CcsScanInput_t * input)
{
#ifdef KcScanner_INDENTATION
    CcsToken_t * t;
#endif
    const CcsComment_t * curComment;
    for (;;) {
	while (input->ch == ' '
	       /*---- scan1 ----*/
	       || input->ch == '\t'
	       /*---- enable ----*/
	       )  CcsGetCh(input);
#ifdef KcScanner_INDENTATION
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

static int KcScanner_Kind(void * scanner, CcsScanInput_t * input)
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
	} else { kind = GetKWKind(input, pos, input->pos, 4); break; }
    case 2: case_2:
	if ((input->ch >= 0 && input->ch <= '\t') ||
	    (input->ch >= '\v' && input->ch <= '\f') ||
	    (input->ch >= 14 && input->ch <= '!') ||
	    (input->ch >= '#' && input->ch <= '[') ||
	    (input->ch >= ']' && input->ch <= 65535)) {
	    CcsGetCh(input); goto case_2;
	} else if (input->ch == '"') {
	    CcsGetCh(input); goto case_4;
	} else if (input->ch == '\\') {
	    CcsGetCh(input); goto case_3;
	} else { kind = Scanner_Info.noSym; break; }
    case 3: case_3:
	if ((input->ch >= ' ' && input->ch <= '~')) {
	    CcsGetCh(input); goto case_2;
	} else { kind = Scanner_Info.noSym; break; }
    case 4: case_4:
	{ kind = 5; break; }
    case 5:
	if (input->ch == '\n') {
	    CcsGetCh(input); goto case_6;
	} else { kind = Scanner_Info.noSym; break; }
    case 6: case_6:
	{ kind = 6; break; }
    case 7:
	{ kind = 33; break; }
    case 8:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_9;
	} else { kind = Scanner_Info.noSym; break; }
    case 9: case_9:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_10;
	} else { kind = Scanner_Info.noSym; break; }
    case 10: case_10:
	if (input->ch == 'h') {
	    CcsGetCh(input); goto case_11;
	} else { kind = Scanner_Info.noSym; break; }
    case 11: case_11:
	if (input->ch == 'e') {
	    CcsGetCh(input); goto case_12;
	} else { kind = Scanner_Info.noSym; break; }
    case 12: case_12:
	if (input->ch == 'l') {
	    CcsGetCh(input); goto case_13;
	} else { kind = Scanner_Info.noSym; break; }
    case 13: case_13:
	if (input->ch == 'p') {
	    CcsGetCh(input); goto case_14;
	} else { kind = Scanner_Info.noSym; break; }
    case 14: case_14:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_15;
	} else { kind = Scanner_Info.noSym; break; }
    case 15: case_15:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_16;
	} else { kind = Scanner_Info.noSym; break; }
    case 16: case_16:
	if (input->ch == '-') {
	    CcsGetCh(input); goto case_17;
	} else { kind = Scanner_Info.noSym; break; }
    case 17: case_17:
	{ kind = 36; break; }
    case 18:
	if (input->ch == '|') {
	    CcsGetCh(input); goto case_19;
	} else { kind = Scanner_Info.noSym; break; }
    case 19: case_19:
	{ kind = 37; break; }
    case 20:
	if (input->ch == '&') {
	    CcsGetCh(input); goto case_21;
	} else { kind = Scanner_Info.noSym; break; }
    case 21: case_21:
	{ kind = 38; break; }
    case 22:
	{ kind = 40; break; }
    case 23:
	{ kind = 41; break; }
    case 24: case_24:
	{ kind = 42; break; }
    case 25:
	if (input->ch == '=') {
	    CcsGetCh(input); goto case_24;
	} else { kind = 39; break; }
    /*---- enable ----*/
    }
    return kind;
}
