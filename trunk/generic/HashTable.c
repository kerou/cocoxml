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
#include  <string.h>
#include  "HashTable.h"

struct HTEntry_s {
    char * key;
    void * value;
};

static int
strhash(const char * str, int szhash)
{
    int value = 0;
    while (*str) value += *str++;
    return value % szhash;
}

HashTable_t *
HashTable(HashTable_t * self, size_t size)
{
    Bool_t malloced;
    if (!(self = AllocObject(self, sizeof(HashTable_t), &malloced)))
	goto errquit0;
    if (!(self->first = malloc(sizeof(HTEntry_t *) * size))) goto errquit1;
    self->last = self->first + size;
    bzero(self->first, sizeof(HTEntry_t *) * size);
    return self;
 errquit1:
    if (malloced) free(self);
 errquit0:
    return NULL;
}

void HashTable_Destruct(HashTable_t * self)
{
    if (self->first)  free(self->first);
}

int
HashTable_Set(HashTable_t * self, const char * key, void * value)
{
    HTEntry_t ** start, ** cur;
    start = cur = self->first + strhash(key, self->last - self->first);
    for (;;) {
	if (*cur == NULL) {
	    if (!(*cur = malloc(sizeof(HTEntry_t) + strlen(key) + 1)))
		return -1;
	    (*cur)->key = (char *)(*cur + 1); strcpy((*cur)->key, key);
	    (*cur)->value = value;
	    return 0;
	}
	if (++cur == self->last) cur = self->first;
	if (cur == start) break;
    }
    return -1; /* Full */
}

void *
HashTable_Get(const HashTable_t * self, const char * key)
{
    HTEntry_t ** start, ** cur;
    start = cur = self->first + strhash(key, self->last - self->first);
    for (;;) {
	if (*cur == NULL) return NULL;
	if (!strcmp((*cur)->key, key)) return (*cur)->value;
	if (++cur == self->last) cur = self->first;
	if (cur == start) break;
    }
    return NULL;
}

HTIterator_t *
HashTable_GetIterator(const HashTable_t * self, HTIterator_t * iter)
{
    iter->cur = self->first;
    iter->last = self->last;
    return iter;
}

Bool_t
HTIterator_Forward(HTIterator_t * self)
{
    while (self->cur < self->last)
	if (*self->cur) return TRUE;
	else ++self->cur;
    return FALSE;
}

const char *
HTIterator_Key(HTIterator_t * iter)
{
    return (*(iter->cur))->key;
}

void *
HTIterator_Value(HTIterator_t * iter)
{
    return (*(iter->cur))->value;
}