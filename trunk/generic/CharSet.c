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
#include  <stdlib.h>
#include  "CharSet.h"

/* Define COCO_WCHAR_MAX here now, it should be placed in Scanner.frame. */
#define   COCO_WCHAR_MAX  65535

struct Range_s {
    int from, to;
    Range_t * next;
};
Range_t *
new_Range(int from, int to)
{
    Range_t * self = CocoMalloc(sizeof(Range_t));
    self->from = from; self->to = to; self->next = NULL;
    return self;
}
#define Del_Range(range)  CocoFree(range)

CharSet_t *
CharSet(CharSet_t * self)
{
    self = AllocObject(self, sizeof(CharSet_t));
    self->head = NULL;
    return self;
}

void
CharSet_Destruct(CharSet_t * self)
{
    CharSet_Clear(self);
}

Bool_t
CharSet_Get(const CharSet_t * self, int i)
{
    Range_t * cur = self->head;
    for (cur = self->head; cur; cur = cur->next) {
	if (i < cur->from) return FALSE;
	else if (i <= cur->to) return TRUE;
    }
    return FALSE;
}

void
CharSet_Set(CharSet_t * self, int i)
{
    Range_t * cur, * prev = NULL, * tmp;

    for (cur = self->head; cur; cur = cur->next) {
	if (i < cur->from - 1) { /* New Range has to be setup. */
	    break;
	} else if (i == cur->from - 1) { /* Extend cur->from is ok. */
	    --cur->from;
	    return;
	} else if (i <= cur->to) { /* In cur already. */
	    return;
	} else if (i == cur->to + 1) { /* Extend cur->to is ok. */
	    if (cur->next != NULL && i == cur->next->from - 1) {
		/* Combine cur and cur->next. */
		tmp = cur->next;
		cur->to = cur->next->to;
		cur->next = cur->next->next;
		Del_Range(tmp);
	    } else {
		++cur->to;
	    }
	    return;
	}
	prev = cur;
    }
    tmp = new_Range(i, i);
    tmp->next = cur;
    if (prev == NULL) self->head = tmp;
    else prev->next = tmp;
}

CharSet_t *
CharSet_Clone(CharSet_t * self, const CharSet_t * s)
{
    Range_t * prev, * curnew;
    const Range_t * cur1;

    self = AllocObject(self, sizeof(CharSet_t));
    self->head = NULL; prev = NULL;
    for (cur1 = s->head; cur1; cur1 = cur1->next) {
	curnew = new_Range(cur1->from, cur1->to);
	if (prev == NULL) self->head = curnew;
	else prev->next = curnew;
	prev = curnew;
    }
    return self;
}

Bool_t
CharSet_Equals(const CharSet_t * self, const CharSet_t * s)
{
    const Range_t * cur0, * cur1;
    cur0 = self->head; cur1 = s->head;
    while (cur0 && cur1) {
	if (cur0->from != cur1->from || cur0->to != cur1->to) return FALSE;
	cur0 = cur0->next; cur1 = cur1->next;
    }
    return cur0 == cur1;
}

int
CharSet_Elements(const CharSet_t * self)
{
    int cnt = 0;
    const Range_t * cur;
    for (cur = self->head; cur; cur = cur->next)
	cnt += cur->to - cur->from + 1;
    return cnt;
}

int
CharSet_First(const CharSet_t * self)
{
    if (self->head) return self->head->from;
    return -1;
}

void
CharSet_Or(CharSet_t * self, const CharSet_t * s)
{
    Range_t * tmp, * cur0 = self->head, * prev = NULL;
    const Range_t * cur1 = s->head;

    while (cur0 && cur1) {
	if (cur0->from > cur1->to + 1) {
	    /* cur1 has to be inserted before cur0. */
	    tmp = new_Range(cur1->from, cur1->to);
	    if (prev == NULL) self->head = tmp;
	    else prev->next = tmp;
	    tmp->next = cur0;
	    prev = tmp; cur1 = cur1->next;
	} else if (cur0->to + 1 >= cur1->from) {
	    /* cur0, cur1 overlapped, expand cur0. */
	    if (cur0->from > cur1->from) cur0->from = cur1->from;
	    if (cur0->to < cur1->to) {
		cur0->to = cur1->to;
		/* Try to combine cur0->next. */
		while (cur0->next && cur0->next->from <= cur0->to + 1) {
		    tmp = cur0->next;
		    cur0->next = cur0->next->next;
		    if (cur0->to < tmp->to) cur0->to = tmp->to;
		    Del_Range(tmp);
		}
	    }
	    cur1 = cur1->next;
	} else { /* cur0->to + 1 < cur1->from */
	    prev = cur0; cur0 = cur0->next;
	}
    }
    while (cur1) { /* Add all of the remaining. */
	tmp = new_Range(cur1->from, cur1->to);
	if (prev == NULL) self->head = tmp;
	else prev->next = tmp;
	prev = tmp; cur1 = cur1->next;
    }
}

void
CharSet_And(CharSet_t * self, const CharSet_t * s)
{
    Range_t * tmp, * cur0 = self->head, * prev = NULL;
    const Range_t * cur1 = s->head;

    while (cur0 && cur1) {
	if (cur0->from > cur1->to) {
	    cur1 = cur1->next;
	} else if (cur0->to < cur1->from) {
	    tmp = cur0; cur0 = cur0->next;
	    if (prev == NULL) self->head = cur0;
	    else prev->next = cur0;
	    Del_Range(tmp);
	} else if (cur0->from < cur1->from && cur0->to <= cur1->to) {
	    cur0->from = cur1->from;
	    prev = cur0; cur0 = cur0->next;
	} else if (cur0->from < cur1->from && cur0->to > cur1->to) {
	    tmp = new_Range(cur1->to + 2, cur0->to);
	    tmp->next = cur0->next; cur0->next = tmp;
	    cur0->from = cur1->from; cur0->to = cur1->to;
	    prev = cur0; cur0 = cur0->next;
	    cur1 = cur1->next;
	} else if (cur0->from >= cur1->from && cur0->to <= cur1->to) {
	    prev = cur0; cur0 = cur0->next;
	} else if (cur0->from >= cur1->from && cur0->to > cur1->to) {
	    tmp = new_Range(cur1->to + 2, cur0->to);
	    tmp->next = cur0->next; cur0->next = tmp;
	    cur0->to = cur1->to;
	    prev = cur0; cur0 = cur0->next;
	    cur1 = cur1->next;
	}
    }
    if (cur0) {
	if (prev == NULL) self->head = NULL;
	else prev->next = NULL;
	while (cur0) {
	    tmp = cur0; cur0 = cur0->next;
	    Del_Range(tmp);
	}
    }
}

void
CharSet_Subtract(CharSet_t * self, const CharSet_t * s)
{
    Range_t * tmp, * cur0 = self->head, * prev = NULL;
    const Range_t * cur1 = s->head;

    while (cur0 && cur1) {
	if (cur0->from > cur1->to) {
	    cur1 = cur1->next;
	} else if (cur0->to < cur1->from) {
	    prev = cur0; cur0 = cur0->next;
	} else if (cur0->from < cur1->from && cur0->to <= cur1->to) {
	    cur0->to = cur1->from - 1;
	    prev = cur0; cur0 = cur0->next;
	} else if (cur0->from < cur1->from && cur0->to > cur1->to) {
	    tmp = new_Range(cur1->to + 1, cur0->to);
	    cur0->to = cur1->from - 1;
	    tmp->next = cur0->next; cur0->next = tmp;
	    prev = cur0; cur0 = cur0->next;
	    cur1 = cur1->next;
	} else if (cur0->from >= cur1->from && cur0->to <= cur1->to) {
	    tmp = cur0; cur0 = cur0->next;
	    if (prev == NULL) self->head = cur0;
	    else prev->next = cur0;
	    Del_Range(tmp);
	} else if (cur0->from >= cur1->from && cur0->to > cur1->to) {
	    cur0->from = cur1->to + 1;
	    cur1 = cur1->next;
	}
    }
}

Bool_t
CharSet_Includes(const CharSet_t * self, const CharSet_t * s)
{
    const Range_t * cur0 = self->head, * cur1 = s->head;

    while (cur0 && cur1) {
	if (cur0->to < cur1->from)  cur0 = cur0->next;
	else if (cur0->from <= cur1->from && cur0->to >= cur1->to)
	    cur1 = cur1->next;
	else  return FALSE;
    }
    return cur1 == NULL;
}

Bool_t
CharSet_Intersects(const CharSet_t * self, const CharSet_t * s)
{
    const Range_t * cur0 = self->head, * cur1 = s->head;

    while (cur0 && cur1) {
	if (cur0->from > cur1->to) cur1 = cur1->next;
	else if (cur0->to < cur1->from) cur0 = cur0->next;
	else return TRUE;
    }
    return FALSE;
}

void
CharSet_Clear(CharSet_t * self)
{
    Range_t * tmp;
    while (self->head != NULL) {
	tmp = self->head;
	self->head = self->head->next;
	Del_Range(tmp);
    }
}

void
CharSet_Fill(CharSet_t * self)
{
    CharSet_Clear(self);
    self->head = new_Range(0, COCO_WCHAR_MAX);
}

void
CharSet_Dump(const CharSet_t * self, DumpBuffer_t * buf)
{
    const Range_t * cur;
    for (cur = self->head; cur && !DumpBuffer_Full(buf); cur = cur->next) {
	if (cur->from == cur->to) {
	    DumpBuffer_Print(buf, "'"); EscapeCh(buf, cur->from); DumpBuffer_Print(buf, "'");
	} else {
	    DumpBuffer_Print(buf, "'"); EscapeCh(buf, cur->from); DumpBuffer_Print(buf, "'");
	    DumpBuffer_Print(buf, "..");
	    DumpBuffer_Print(buf, "'"); EscapeCh(buf, cur->to); DumpBuffer_Print(buf, "'");
	}
	if (cur->next) DumpBuffer_Print(buf, ", ");
    }
}

void
CharSet_DumpInt(const CharSet_t * self, DumpBuffer_t * buf)
{
    const Range_t * cur;
    for (cur = self->head; cur && !DumpBuffer_Full(buf); cur = cur->next) {
	if (cur->from == cur->to) {
	    DumpBuffer_Print(buf, "%d", cur->from);
	} else {
	    DumpBuffer_Print(buf, "%d..%d", cur->from, cur->to);
	}
	if (cur->next) DumpBuffer_Print(buf, ", ");
    }
}
