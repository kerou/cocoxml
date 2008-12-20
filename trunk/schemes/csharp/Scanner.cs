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

/*---- defines ----*/
#define CASE_SENSITIVE
/*---- enable ----*/

public class CcsScanner_t {
    CcsErrorPool_t errpool;
    int            eofSym;
    int            noSym;
    int            maxT;
    CcsToken_t     dummyToken;
    CcsToken_t     busyTokenList;
    CcsToken_t     curToken;
    CcsToken_t     peekToken;

    int            ch;
    int            chBytes;
    int            pos;
    int            line;
    int            col;
    int            oldEols;
    int            oldEolsEOL;
    CcsBuffer_t    buffer;

    CcsScanner_t(CcsErrorPool_t errpool, string filename)
    {
	Stream stream;
	this.errpool = errpool;
	stream = FileStream(filename, FileMode.Open);
	dummyToken = CcsToken_t(0, 0, 0, 0, "dummy");
	buffer = CcsBuffer_t(stream);
	Init();
    }

    private void Init()
    {
	/*---- declarations ----*/
	eofSym = 0;
	maxT = 47;
	noSym = 47;
	/*---- enable ----*/

	busyTokenList = null;
	curToken = null;
	peekToken = null;

	ch = 0; chBytes = 0;
	pos = 0; line = 1; col = 0;
	oldEols = 0; oldEolsEOL = 0;
	GetCh();
    }

    ~CcsScanner_t()
    {
    }    

    CcsToken_t GetDummy()
    {
	return dummyToken;
    }

    CcsToken_t Scan()
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

    CcsToken_t Peek()
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

    void ResetPeek()
    {
	peekToken = curToken;
    }

    void IncRef(CcsToken_t token)
    {
	++token.refcnt;
    }

    void DecRef(CcsToken_t token)
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

    CcsPosition_t GetPosition(const CcsToken_t begin, const CcsToken_t end)
    {
	int len = end->pos - begin->pos;
	return CcsPosition(begin->pos, len, begin->col,
			   CcsBuffer_GetString(&self->buffer, begin->pos, len));
    }

    CcsPosition_t GetPositionBetween(const CcsToken_t begin, const CcsToken_t end)
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
    private class Char2State_t {
	private int keyFrom;
	private int keyTo;
	public  int val;

	public int Compare(Char2State_t x, Char2State_t y)
	{
	    if (x.keyFrom < y.keyFrom) return -1;
	    if (x.keyFrom > y.keyFrom) return 1;
	    return 0;
	}
    };

    private const Char2State_t[] c2sArr = {
	/*---- chars2states ----*/
	{ CcsBuffer_t.EoF, CcsBuffer_t.EoF, -1 },
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

    private int Char2State(int chr)
    {
	int index = Array.BinarySearch(c2sArr, chr);
	return index >= 0 && index < c2sArr.Count ? c2sArr[index].val : 0;
    }

    private clsss Identifier2KWKind_t {
	private const string key;
	public int val;

	public int Compare(Identifier2KWKind_t x, Identifier2KWKind_t y)
	{
	    return Compare(x.key, y.key);
	}
    };

    private const Identifier2KWKind_t i2kArr[] = {
	/*---- identifiers2keywordkinds ----*/
	{ "ANY", 29 },
	{ "CHARACTERS", 11 },
	{ "COMMENTS", 14 },
	{ "COMPILER", 6 },
	{ "CONSTRUCTOR", 8 },
	{ "CONTEXT", 44 },
	{ "DESTRUCTOR", 9 },
	{ "END", 22 },
	{ "FROM", 15 },
	{ "IF", 43 },
	{ "IGNORE", 18 },
	{ "IGNORECASE", 10 },
	{ "MEMBERS", 7 },
	{ "NESTED", 17 },
	{ "PRAGMAS", 13 },
	{ "PRODUCTIONS", 19 },
	{ "SCHEME", 23 },
	{ "SECTION", 24 },
	{ "SYNC", 42 },
	{ "TO", 16 },
	{ "TOKENS", 12 },
	{ "UPDATES", 25 },
	{ "WEAK", 35 },
	/*---- enable ----*/
    };

    private int Identifier2KWKind(string key, int defaultVal)
    {
	int index;
#ifndef CASE_SENSITIVE
	key = key.lower();
#endif
	index = Array.BinarySearch(i2kArr, key);
	return index >= 0 && index < i2kArr.Count ?
	    i2kArr[index].val : defaultVal;
    }

    private int GetKWKind(int start, int end, int defaultVal)
    {
	return Identifier2KWKind(buffer.GetString(start, end - start),
				 defaultVal);
    }

    private void GetCh()
    {
	if (oldEols > 0) {
	    ch = '\n'; --oldEols; oldEolsEOL= 1;
	} else {
	    if (ch == '\n') {
		if (oldEolsEOL) oldEolsEOL = 0;
		else {
		    ++line; col = 0;
		}
	    } else if (ch == '\t') {
		col += 8 - col % 8;
	    } else {
		/* FIX ME: May be the width of some specical character
		 * is NOT self->chBytes. */
		col += chBytes;
	    }
	    ch = buffer.Read(chBytes);
	    pos = buffer.GetPos();
	}
    }

    private struct SLock_t {
	int ch, chBytes;
	int pos, line, col;
    };

    private void LockCh(SLock_t slock)
    {
	slock.ch = ch;
	slock.chBytes = chBytes;
	slock.pos = pos;
	slock.line = line;
	slock.col = col;
	buffer.Lock();
    }

    private void UnlockCh(SLock_t slock)
    {
	buffer.Unlock();
    }

    priate void ResetCh(SLock_t * slock)
    {
	ch = slock.ch;
	chBytes = slock.chBytes;
	pos = slock.pos;
	line = slock.line;
	buffer.LockReset();
    }

    private struct CcsComment_t {
	int start[2];
	int end[2];
	bool nested;
    }

    private const CcsComment_t comments[] = {
	/*---- comments ----*/
	{ { '/', '/' }, { '\n', 0 }, FALSE },
	{ { '/', '*' }, { '*', '/' }, TRUE },
	/*---- enable ----*/
    };

    private bool Comment(const CcsComment_t c)
    {
	SLock_t slock;
	int level = 1, line0 = line;

	if (c->start[1]) {
	    CcsScanner_LockCh(self, &slock); GetCh();
	    if (self->ch != c->start[1]) {
		CcsScanner_ResetCh(self, &slock);
		return FALSE;
	    }
	    CcsScanner_UnlockCh(self, &slock);
	}
	GetCh();
	for (;;) {
	    if (self->ch == c->end[0]) {
		if (c->end[1] == 0) {
		    if (--level == 0) break;
		} else {
		    CcsScanner_LockCh(self, &slock); GetCh();
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
		    CcsScanner_LockCh(self, &slock); GetCh();
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
	    GetCh();
	}
	self->oldEols = self->line - line0;
	GetCh();
	return TRUE;
    }

    public CcsToken_t NextToken()
    {
	int pos, line, col, state, kind; CcsToken_t t;
	const CcsComment_t curComment;
	for (;;) {
	    while (ch == ' '
		   /*---- scan1 ----*/
		   || (ch >= '\t' && ch <= '\n')
		   || ch == '\r'
		   /*---- enable ----*/
		   ) GetCh();
	    for (curComment = comments; curComment < commentsLast; ++curComment)
		if (ch == curComment.start[0] &&
		    CcsScanner_Comment(self, curComment)) break;
	    if (curComment < commentsLast) continue;
	    break;
	}
	pos = self->pos; line = self->line; col = self->col;
	CcsBuffer_Lock(&self->buffer);
	state = Char2State(self->ch);
	GetCh();
	switch (state) {
	case -1: kind = self->eofSym; break;
	case 0: kind = self->noSym; break;
	    /*---- scan3 ----*/
	case 1: case_1:
	    if ((self->ch >= '0' && self->ch <= '9') ||
		(self->ch >= 'A' && self->ch <= 'Z') ||
		self->ch == '_' ||
		(self->ch >= 'a' && self->ch <= 'z')) {
		GetCh(); goto case_1;
	    } else { kind = CcsScanner_GetKWKind(self, pos, self->pos, 1); break; }
	case 2: case_2:
	    if ((self->ch >= '0' && self->ch <= '9')) {
		GetCh(); goto case_2;
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
		GetCh(); goto case_6;
	    } else if (self->ch == '\\') {
		GetCh(); goto case_7;
	    } else { kind = self->noSym; break; }
	case 6: case_6:
	    if (self->ch == '\'') {
		GetCh(); goto case_9;
	    } else { kind = self->noSym; break; }
	case 7: case_7:
	    if ((self->ch >= ' ' && self->ch <= '~')) {
		GetCh(); goto case_8;
	    } else { kind = self->noSym; break; }
	case 8: case_8:
	    if ((self->ch >= '0' && self->ch <= '9') ||
		(self->ch >= 'a' && self->ch <= 'f')) {
		GetCh(); goto case_8;
	} else if (self->ch == '\'') {
		GetCh(); goto case_9;
	    } else { kind = self->noSym; break; }
	case 9: case_9:
	    { kind = 5; break; }
	case 10: case_10:
	    if ((self->ch >= '0' && self->ch <= '9') ||
		(self->ch >= 'A' && self->ch <= 'Z') ||
		self->ch == '_' ||
		(self->ch >= 'a' && self->ch <= 'z')) {
		GetCh(); goto case_10;
	    } else { kind = 48; break; }
	case 11: case_11:
	    if ((self->ch >= 0 && self->ch <= '\t') ||
		(self->ch >= '\v' && self->ch <= '\f') ||
		(self->ch >= 14 && self->ch <= '!') ||
		(self->ch >= '#' && self->ch <= '[') ||
		(self->ch >= ']' && self->ch <= 65535)) {
		GetCh(); goto case_11;
	    } else if (self->ch == '"') {
		GetCh(); goto case_3;
	    } else if (self->ch == '\\') {
		GetCh(); goto case_12;
	    } else if (self->ch == '\n' ||
		       self->ch == '\r') {
		GetCh(); goto case_4;
	    } else { kind = self->noSym; break; }
	case 12: case_12:
	    if ((self->ch >= ' ' && self->ch <= '~')) {
		GetCh(); goto case_11;
	    } else { kind = self->noSym; break; }
	case 13:
	    { kind = 20; break; }
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
		GetCh(); goto case_16;
	    } else if (self->ch == '>') {
		GetCh(); goto case_19;
	    } else if (self->ch == ')') {
		GetCh(); goto case_27;
	    } else { kind = 21; break; }
	case 29:
	    if (self->ch == '.') {
		GetCh(); goto case_18;
	    } else { kind = 30; break; }
	case 30:
	    if (self->ch == '.') {
		GetCh(); goto case_26;
	    } else { kind = 36; break; }
	    /*---- enable ----*/
	}
	t = CcsToken(kind, pos, col, line,
		     buffer.GetString(pos, this.pos - pos),
		     this.pos - pos);
	buffer.Unlock();
	return t;
    }
}
