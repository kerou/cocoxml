/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "BitArray.h"

#define NB2SZ(nb)   (((nb) + 7) >> 3)
static int bitmask[] = { 0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };

CcBitArray_t *
CcBitArray(CcBitArray_t * self, int numbits)
{
    self->numbits = numbits;
    if (numbits) {
	self->data = CcMalloc(NB2SZ(numbits));
	memset(self->data, 0, NB2SZ(numbits));
    } else {
	self->data = NULL;
    }
    return self;
}

CcBitArray_t *
CcBitArray1(CcBitArray_t * self, int numbits)
{
    self->numbits = numbits;
    if (numbits) {
	self->data = CcMalloc(NB2SZ(numbits));
	memset(self->data, 0xFF, NB2SZ(numbits));
    } else {
	self->data = NULL;
    }
    return self;
}

CcBitArray_t *
CcBitArray_Clone(CcBitArray_t * self, const CcBitArray_t * value)
{
    if (value->data) {
	self->data = CcMalloc(NB2SZ(value->numbits));
	memcpy(self->data, value->data, NB2SZ(value->numbits));
    } else {
	self->data = NULL;
    }
    self->numbits = value->numbits;
    return self;
}

void
CcBitArray_Destruct(CcBitArray_t * self)
{
    if (self->data) {
	CcFree(self->data);
	self->data = NULL;
    }
    self->numbits = 0;
}

int
CcBitArray_getCount(const CcBitArray_t * self)
{
    return self->numbits;
}

int
CcBitArray_Elements(const CcBitArray_t * self)
{
    int bit;
    int elements = 0;
    int bits = self->numbits;
    const unsigned char * cur = self->data;
    while (bits >= 8) {
	for (bit = 0; bit < 8; ++bit)
	    if ((*cur & (1 << bit))) ++elements;
	bits = bits - 8;
	++cur;
    }
    for (bit = 0; bit < bits; ++bit)
	if ((*cur & (1 << bit))) ++elements;
    return elements;
}

CcsBool_t
CcBitArray_Get(const CcBitArray_t * self, int index)
{
    if (index < 0 || index >= self->numbits) return -1;
    return (self->data[index >> 3] & (1 << (index & 0x07))) != 0;
}

void
CcBitArray_Set(CcBitArray_t * self, int index, CcsBool_t value)
{
    if (value) self->data[index >> 3] |= 1 << (index & 0x07);
    else self->data[index >> 3] &= ~(1 << (index & 0x07));
}

void
CcBitArray_SetAll(CcBitArray_t * self, CcsBool_t value)
{
    if (self->data) memset(self->data, value ? 0xFF : 0, NB2SZ(self->numbits));
}

CcsBool_t
CcBitArray_Equal(const CcBitArray_t * self1, const CcBitArray_t * self2)
{
    int boffset, bmask;
    if (self1->numbits != self2->numbits) return FALSE;
    if (!self1->data && !self2->data) return TRUE;
    if (!self1->data || !self2->data) return FALSE;
    if (self1->numbits > 8 &&
	memcmp(self1->data, self2->data, NB2SZ(self1->numbits) - 1))
	return FALSE;
    boffset = NB2SZ(self1->numbits) - 1;
    bmask = bitmask[self1->numbits & 0x07];
    if ((self1->data[boffset] & bmask) != (self2->data[boffset] & bmask))
	return FALSE;
    return TRUE;
}

void
CcBitArray_Not(CcBitArray_t * self)
{
    unsigned char * cur;
    for (cur = self->data; cur - self->data < NB2SZ(self->numbits); ++cur)
	*cur ^= 0xFF;
}

int
CcBitArray_And(CcBitArray_t * self, const CcBitArray_t * value)
{
    unsigned char * cur0, * cur1;
    if (self->numbits > value->numbits) return -1;
    for (cur0 = self->data, cur1 = value->data;
	 cur0 - self->data < NB2SZ(self->numbits); ++cur0, ++cur1)
	*cur0 &= *cur1;
    return 0;
}

int
CcBitArray_Or(CcBitArray_t * self, const CcBitArray_t * value)
{
    unsigned char * cur0, * cur1;
    if (self->numbits > value->numbits) return -1;
    for (cur0 = self->data, cur1 = value->data;
	 cur0 - self->data < NB2SZ(self->numbits); ++cur0, ++cur1)
	*cur0 |= *cur1;
    return 0;
}

int
CcBitArray_Xor(CcBitArray_t * self, const CcBitArray_t * value)
{
    unsigned char * cur0, * cur1;
    if (self->numbits > value->numbits) return -1;
    for (cur0 = self->data, cur1 = value->data;
	 cur0 - self->data < NB2SZ(self->numbits); ++cur0, ++cur1)
	*cur0 ^= *cur1;
    return 0;
}

CcsBool_t
CcBitArray_Intersect(const CcBitArray_t * self1, const CcBitArray_t * self2)
{
    /* assert(self1->numbits == self2->numbits2); */
    int idx, numbytes = NB2SZ(self1->numbits);
    if (numbytes == 0) return FALSE;
    for (idx = 0; idx < numbytes - 1; ++idx)
	if ((self1->data[idx] & self2->data[idx])) return TRUE;
    if ((self1->data[numbytes - 1] & self2->data[numbytes - 1] &
	 bitmask[self1->numbits & 0x07]))
	return TRUE;
    return FALSE;
}

void
CcBitArray_Subtract(CcBitArray_t * self, const CcBitArray_t * b)
{
    /* assert(self->numbits == b->numbits); */
    int idx;
    for (idx = 0; idx < NB2SZ(self->numbits); ++idx)
	self->data[idx] &= ~ b->data[idx];
}

/*
void
CcBitArray_Dump(const CcBitArray_t * self, DumpBuffer_t * buf)
{
    int idx, numbits = CcBitArray_getCount(self);
    for (idx = 0; idx < numbits; ++idx)
	DumpBuffer_Print(buf, "%c", CcBitArray_Get(self, idx) ? '1' : '.');
}
*/
