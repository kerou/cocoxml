/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

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
#include  "Buffer.h"

/* CcsBuffer_t private members. */
/*#define BUFSTEP  8*/
#define BUFSTEP 4096
static int CcsBuffer_Load(CcsBuffer_t * self);
static int CcsBuffer_ReadByte(CcsBuffer_t * self, int * value);

CcsBuffer_t *
CcsBuffer(CcsBuffer_t * self, FILE * fp)
{
    self->fp = fp;
    self->start = 0;
    if (!(self->buf = CcsMalloc(BUFSTEP))) goto errquit0;
    self->busyFirst = self->lockCur = NULL;
    self->cur = NULL;
    self->next = self->loaded = self->buf;
    self->last = self->buf + BUFSTEP;
    if (CcsBuffer_Load(self) < 0) goto errquit1;
    return self;
 errquit1:
    CcsFree(self->buf);
 errquit0:
    return NULL;
}

void
CcsBuffer_Destruct(CcsBuffer_t * self)
{
    fclose(self->fp);
    CcsFree(self->buf);
}

long
CcsBuffer_GetPos(CcsBuffer_t * self)
{
    return self->cur ? self->start + (self->cur - self->buf) : 0L;
}

int
CcsBuffer_Read(CcsBuffer_t * self, int * retBytes)
{
    int ch, c1, c2, c3, c4;
    /* self->start might be changed in CcsBuffer_ReadByte */
    long next = self->start + (self->next - self->buf);

    self->cur = self->next;

    if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;

    if (ch < 128) goto quit;

    if ((ch & 0xC0) != 0xC0) /* Inside UTF-8 character! */
	return ErrorChr;
    if ((ch & 0xF0) == 0xF0) {
	/* 1110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	c1 = ch & 0x07;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c2 = ch & 0x3F;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c3 = ch & 0x3F;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c4 = ch & 0x3F;
	ch = (((((c1 << 6) | c2) << 6) | c3) << 6) | c4;
    } else if ((ch & 0xE0) == 0xE0) {
	/* 1110xxxx 10xxxxxx 10xxxxxx */
	c1 = ch & 0x0F;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c2 = ch & 0x3F;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c3 = ch & 0x3F;
	ch = (((c1 << 6) | c2) << 6) | c3;
    } else {
	/* (ch & 0xC0) == 0xC0 */
	/* 110xxxxx 10xxxxxx */
	c1 = ch & 0x1F;
	if (CcsBuffer_ReadByte(self, &ch) < 0) goto quit;
	c2 = ch & 0x3F;
	ch = (c1 << 6) | c2;
    }
 quit:
    *retBytes = self->start + (self->next - self->buf) - next;
    return ch;
}

const char *
CcsBuffer_GetString(CcsBuffer_t * self, long start, size_t size)
{
    if (size == 0) return NULL;
    if (start < self->start || start >= self->start + (self->cur - self->buf)){
	fprintf(stderr, "start is out of range!\n");
	exit(-1);
    }
    return self->buf + (start - self->start);
}

void
CcsBuffer_SetBusy(CcsBuffer_t * self, long startBusy)
{
    CcsAssert(startBusy >= self->start);
    self->busyFirst = self->buf + (startBusy - self->start);
    CcsAssert(self->busyFirst <= self->cur);
}

void
CcsBuffer_ClearBusy(CcsBuffer_t * self)
{
    self->busyFirst = NULL;
}

void
CcsBuffer_Lock(CcsBuffer_t * self)
{
    CcsAssert(self->lockCur == NULL);
    self->lockCur = self->cur;
    self->lockNext = self->next;
}

void
CcsBuffer_LockReset(CcsBuffer_t * self)
{
    CcsAssert(self->lockCur != NULL);
    self->cur = self->lockCur;
    self->next = self->lockNext;
    self->lockCur = NULL;
}

void
CcsBuffer_Unlock(CcsBuffer_t * self)
{
    CcsAssert(self->lockCur != NULL);
    self->lockCur = NULL;
}

static int
CcsBuffer_Load(CcsBuffer_t * self)
{
    size_t rc = fread(self->loaded, 1, self->last - self->loaded, self->fp);
    if (rc > 0) self->loaded += rc;
    else if (ferror(self->fp)) return -1;
    return 0;
}

static int
CcsBuffer_ReadByte(CcsBuffer_t * self, int * value)
{
    int delta; char * keptFirst, * newbuf;
    while (self->next >= self->loaded) {
	/* Calculate keptFirst */
	keptFirst = self->cur;
	if (self->busyFirst && self->busyFirst < keptFirst)
	    keptFirst = self->busyFirst;
	if (self->lockCur && self->lockCur < keptFirst)
	    keptFirst = self->lockCur;
	if (self->buf < keptFirst) { /* Remove the unprotected data. */
	    delta = keptFirst - self->buf;
	    memmove(self->buf, keptFirst, self->loaded - keptFirst);
	    self->start += delta;
	    if (self->busyFirst) self->busyFirst -= delta;
	    if (self->lockCur) {
		self->lockCur -= delta; self->lockNext -= delta;
	    }
	    self->cur -= delta;
	    self->next -= delta;
	    self->loaded -= delta;
	}
	if (feof(self->fp)) { *value = EoF; return -1; }
	/* Try to extend the storage space */
	while (self->loaded >= self->last) {
	    if (!(newbuf =
		  CcsRealloc(self->buf, self->last - self->buf + BUFSTEP))) {
		*value = ErrorChr;
		return -1;
	    }
	    if (self->busyFirst)
		self->busyFirst = newbuf + (self->busyFirst - self->buf);
	    if (self->lockCur) {
		self->lockCur = newbuf + (self->lockCur - self->buf);
		self->lockNext = newbuf + (self->lockNext - self->buf);
	    }
	    self->cur = newbuf + (self->cur - self->buf);
	    self->next = newbuf + (self->next - self->buf);
	    self->loaded = newbuf + (self->loaded - self->buf);
	    self->last = newbuf + (self->last - self->buf + BUFSTEP);
	    self->buf = newbuf;
	}
	if (CcsBuffer_Load(self) < 0) { *value = ErrorChr; return -1; }
    }
    *value = *self->next++;
    return 0;
}
