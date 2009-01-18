/*-------------------------------------------------------------------------
 Author (C) 2009, Charles Wang <charlesw123456@gmail.com>

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
#include  <limits.h>
#include  "c/ScanInput.h"
#include  "c/IncPathList.h"

static CcsToken_t * CcsScanInput_NextToken(CcsScanInput_t * self);
static CcsBool_t
CcsScanInput_Init(CcsScanInput_t * self, void * scanner,
		  const CcsSI_Info_t * info, FILE * fp)
{
    self->next = NULL;
    self->refcnt = 1;
    self->scanner = scanner;
    self->info = info;
    self->fp = fp;
    if (!CcsBuffer(&self->buffer, fp)) goto errquit0;
    self->busyTokenList = NULL;
    self->curToken = &self->busyTokenList;
    self->peekToken = &self->busyTokenList;

    self->ch = 0; self->chBytes = 0;
    self->pos = 0; self->line = 1; self->col = 0;
    self->oldEols = 0; self->oldEolsEOL = 0;
    if (info->additionalInit && !info->additionalInit(self + 1, scanner))
	goto errquit1;
    return TRUE;
 errquit1:
    CcsBuffer_Destruct(&self->buffer);
 errquit0:
    return FALSE;
}

CcsScanInput_t *
CcsScanInput(void * scanner, const CcsSI_Info_t * info, FILE * fp)
{
    CcsScanInput_t * self;
    if (!(self = CcsMalloc(sizeof(CcsScanInput_t) + info->additionalSpace)))
	goto errquit0;
    self->fname = NULL;
    if (!CcsScanInput_Init(self, scanner, info, fp)) goto errquit1;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
}

CcsScanInput_t *
CcsScanInput_ByName(void * scanner, const CcsSI_Info_t * info,
		    const CcsIncPathList_t * list, const char * includer,
		    const char * infn)
{
    FILE * fp;
    CcsScanInput_t * self;
    char infnpath[PATH_MAX];
    if (!(fp = CcsIncPathList_Open(list, infnpath, sizeof(infnpath),
				   includer, infn)))
	goto errquit0;
    if (!(self = CcsMalloc(sizeof(CcsScanInput_t) + info->additionalSpace +
			   strlen(infnpath) + 1)))
	goto errquit1;
    strcpy(self->fname = (char *)(self + 1) + info->additionalSpace, infnpath);
    if (!CcsScanInput_Init(self, scanner, info, fp)) goto errquit2;
    return self;
 errquit2:
    CcsFree(self);
 errquit1:
    fclose(fp);
 errquit0:
    return NULL;
}

void
CcsScanInput_Destruct(CcsScanInput_t * self)
{
    CcsToken_t * cur, * next;
    if (self->info->additionalDestruct)
	self->info->additionalDestruct(self + 1);
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
CcsScanInput_IncRef(CcsScanInput_t * self)
{
    ++self->refcnt;
}

void
CcsScanInput_DecRef(CcsScanInput_t * self)
{
    if (--self->refcnt > 0) return;
    CcsScanInput_Destruct(self);
}

void
CcsScanInput_GetCh(CcsScanInput_t * self)
{
    if (self->oldEols > 0) {
	self->ch = '\n'; --self->oldEols; self->oldEolsEOL = 1;
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

CcsToken_t *
CcsScanInput_NewToken(CcsScanInput_t * self, int kind)
{
    CcsToken_t * t;
    t = CcsToken(self, kind, self->fname, self->pos,
		 self->line, self->col, NULL, 0);
    if (t) CcsScanInput_IncRef(self);
    return t;
}

CcsToken_t *
CcsScanInput_Scan(CcsScanInput_t * self)
{
    CcsToken_t * cur;
    if (*self->curToken == NULL) {
	*self->curToken = CcsScanInput_NextToken(self);
	if (self->curToken == &self->busyTokenList)
	    CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
    }
    cur = *self->curToken;
    self->peekToken = self->curToken = &cur->next;
    ++cur->refcnt;
    return cur;
}

void
CcsScanInput_WithDraw(CcsScanInput_t * self, CcsToken_t * token)
{
    CcsToken_t ** cur;
    CcsAssert(self == token->input);
    CcsAssert(token->refcnt > 1);
    CcsAssert(&token->next == self->curToken);
    for (cur = &self->busyTokenList; *cur != token; cur = &(*cur)->next)
	CcsAssert(*cur != NULL);
    --token->refcnt;
    if (self->peekToken == self->curToken) self->peekToken = cur;
    self->curToken = cur;
}

CcsToken_t *
CcsScanInput_Peek(CcsScanInput_t * self)
{
    CcsToken_t * cur;
    do {
	if (*self->peekToken == NULL) {
	    *self->peekToken = CcsScanInput_NextToken(self);
	    if (self->peekToken == &self->busyTokenList)
		CcsBuffer_SetBusy(&self->buffer, self->busyTokenList->pos);
	}
	cur = *self->peekToken;
	self->peekToken = &cur->next;
    } while (cur->kind > self->info->maxT); /* Skip pragmas */
    ++cur->refcnt;
    return cur;
}

void
CcsScanInput_ResetPeek(CcsScanInput_t * self)
{
    self->peekToken = self->curToken;
}

void
CcsScanInput_TokenIncRef(CcsScanInput_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
CcsScanInput_TokenDecRef(CcsScanInput_t * self, CcsToken_t * token)
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
	if (self->refcnt > 1) CcsScanInput_DecRef(self);
	else {
	    CcsAssert(self->busyTokenList == NULL);
	    CcsScanInput_DecRef(self);
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

CcsPosition_t *
CcsScanInput_GetPosition(CcsScanInput_t * self, const CcsToken_t * begin,
			 const CcsToken_t * end)
{
    int len;
    CcsAssert(self == begin->input);
    CcsAssert(self == end->input);
    len = end->pos - begin->pos;
    return CcsPosition(begin->pos, len, begin->loc.col,
		       CcsBuffer_GetString(&self->buffer, begin->pos, len));
}

CcsPosition_t *
CcsScanInput_GetPositionBetween(CcsScanInput_t * self,
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

static CcsToken_t *
CcsScanInput_NextToken(CcsScanInput_t * self)
{
    int pos, line, col, kind; CcsToken_t * t;

    if ((t = self->info->skip(self->scanner, self))) return t;
    pos = self->pos; line = self->line; col = self->col;
    CcsBuffer_Lock(&self->buffer);
    kind = self->info->kind(self->scanner, self);
    t = CcsToken(self, kind, self->fname, pos, line, col,
		 CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),
		 self->pos - pos);
    if (t) CcsScanInput_IncRef(self);
    CcsBuffer_Unlock(&self->buffer);
    return t;
}

typedef struct {
    int ch, chBytes;
    int pos, line, col;
}  SLock_t;
static void
CcsScanInput_LockCh(CcsScanInput_t * self, SLock_t * slock)
{
    slock->ch = self->ch;
    slock->chBytes = self->chBytes;
    slock->pos = self->pos;
    slock->line = self->line;
    slock->col = self->col;
    CcsBuffer_Lock(&self->buffer);
}
static void
CcsScanInput_UnlockCh(CcsScanInput_t * self, SLock_t * slock)
{
    CcsBuffer_Unlock(&self->buffer);
}
static void
CcsScanInput_ResetCh(CcsScanInput_t * self, SLock_t * slock)
{
    self->ch = slock->ch;
    self->chBytes = slock->chBytes;
    self->pos = slock->pos;
    self->line = slock->line;
    CcsBuffer_LockReset(&self->buffer);
}

CcsBool_t
CcsScanInput_Comment(CcsScanInput_t * self, const CcsComment_t * c)
{
    SLock_t slock;
    int level = 1, line0 = self->line;

    if (c->start[1]) {
	CcsScanInput_LockCh(self, &slock); CcsScanInput_GetCh(self);
	if (self->ch != c->start[1]) {
	    CcsScanInput_ResetCh(self, &slock);
	    return FALSE;
	}
	CcsScanInput_UnlockCh(self, &slock);
    }
    CcsScanInput_GetCh(self);
    for (;;) {
	if (self->ch == c->end[0]) {
	    if (c->end[1] == 0) {
		if (--level == 0) break;
	    } else {
		CcsScanInput_LockCh(self, &slock); CcsScanInput_GetCh(self);
		if (self->ch == c->end[1]) {
		    CcsScanInput_UnlockCh(self, &slock);
		    if (--level == 0) break;
		} else {
		    CcsScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (c->nested && self->ch == c->start[0]) {
	    if (c->start[1] == 0) {
		++level;
	    } else {
		CcsScanInput_LockCh(self, &slock); CcsScanInput_GetCh(self);
		if (self->ch == c->start[1]) {
		    CcsScanInput_UnlockCh(self, &slock);
		    ++level;
		} else {
		    CcsScanInput_ResetCh(self, &slock);
		}
	    }
	} else if (self->ch == EoF) {
	    return TRUE;
	}
	CcsScanInput_GetCh(self);
    }
    self->oldEols = self->line - line0;
    CcsScanInput_GetCh(self);
    return TRUE;
}
