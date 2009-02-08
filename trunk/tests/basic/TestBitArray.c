/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "BitArray.h"
#include  "AutoTests.h"

static void
CcBitArray_RandomSet(CcBitArray_t * self)
{
    long int rnd;
    int idx, bit, numbits = CcBitArray_getCount(self);
    for (idx = 0; idx < numbits; idx += 16) {
	rnd = rand();
	for (bit = 0; bit < 16 && idx + bit < self->numbits; ++bit)
	    if ((rnd & (1 << bit))) CcBitArray_Set(self, idx + bit, 1);
	    else CcBitArray_Set(self, idx + bit, 0);
    }
}

static void
CcBitArray_Dump(const CcBitArray_t * self, FILE * fp,
		const char * prefix, const char * suffix)
{
    int curbits, bit;
    char buf[1024], * curbuf;
    const unsigned char * curdata;

    fprintf(fp, "%s", prefix);
    curbuf = buf; curdata = self->data;
    for (curbits = self->numbits; curbits > 0; curbits -= 8) {
	for (bit = 0; bit < 8 && bit < curbits; ++bit) {
	    if (curbuf == buf + sizeof(buf)) {
		fwrite(buf, 1, sizeof(buf), fp);
		curbuf = buf;
	    }
	    *curbuf++ = (*curdata & (1 << bit)) ? '*' : '.';
	}
	++curdata;
    }
    if (buf < curbuf) fwrite(buf, 1, curbuf - buf, fp);
    fprintf(fp, "%s", suffix);
}

static void
ATest(FILE * fp, CcBitArray_t * ba0, CcBitArray_t * ba1)
{
    CcBitArray_t ba2;
    int idx, cnt, numbits = CcBitArray_getCount(ba0);

    CcBitArray_Dump(ba0, fp, "BitArray 0: ", "\n");
    CcBitArray_Dump(ba1, fp, "BitArray 1: ", "\n");

    cnt = 0;
    for (idx = 0; idx < numbits; ++idx)
	if (CcBitArray_Get(ba0, idx) && CcBitArray_Get(ba1, idx)) cnt = 1;
    COCO_ASSERT((CcBitArray_Intersect(ba0, ba1) == cnt));

    COCO_ASSERT((CcBitArray_Clone(&ba2, ba0)));
    COCO_ASSERT((CcBitArray_Equal(ba0, &ba2)));
    cnt = 0;
    for (idx = 0; idx < numbits; ++idx) {
	COCO_ASSERT(CcBitArray_Get(ba0, idx) == CcBitArray_Get(&ba2, idx));
	if (CcBitArray_Get(ba0, idx)) ++cnt;
    }
    COCO_ASSERT((CcBitArray_Elements(ba0) == cnt));

    COCO_ASSERT((CcBitArray_Or(&ba2, ba1) == 0));
    CcBitArray_Dump(&ba2, fp, "Or: ", "\n");
    for (idx = 0; idx < numbits; ++idx)
	COCO_ASSERT((CcBitArray_Get(ba0, idx) || CcBitArray_Get(ba1, idx)) == CcBitArray_Get(&ba2, idx));
    CcBitArray_Destruct(&ba2);

    COCO_ASSERT((CcBitArray_Clone(&ba2, ba0)));
    COCO_ASSERT((CcBitArray_And(&ba2, ba1) == 0));
    CcBitArray_Dump(&ba2, fp, "And: ", "\n");
    for (idx = 0; idx < numbits; ++idx)
	COCO_ASSERT((CcBitArray_Get(ba0, idx) && CcBitArray_Get(ba1, idx)) == CcBitArray_Get(&ba2, idx));
    CcBitArray_Destruct(&ba2);

    COCO_ASSERT((CcBitArray_Clone(&ba2, ba0)));
    CcBitArray_Subtract(&ba2, ba1);
    CcBitArray_Dump(&ba2, fp, "Subtract: ", "\n");
    for (idx = 0; idx < numbits; ++idx)
	COCO_ASSERT((CcBitArray_Get(ba0, idx) && !CcBitArray_Get(ba1, idx)) == CcBitArray_Get(&ba2, idx));
    COCO_ASSERT(!CcBitArray_Intersect(ba1, &ba2));
    CcBitArray_Destruct(&ba2);

    fprintf(fp, "\n");
}

void
TestBitArray(FILE * fp)
{
    int idx, numbits;
    CcBitArray_t ba0, ba1, ba2;

    for (numbits = 0; numbits < 32; ++numbits) {
	COCO_ASSERT((CcBitArray(&ba0, numbits)));
	COCO_ASSERT((CcBitArray(&ba1, numbits)));
	ATest(fp, &ba0, &ba1);
	CcBitArray_Destruct(&ba0); CcBitArray_Destruct(&ba1);

	COCO_ASSERT((CcBitArray(&ba0, numbits))); CcBitArray_SetAll(&ba0, 1);
	COCO_ASSERT((CcBitArray(&ba1, numbits)));
	ATest(fp, &ba0, &ba1);
	CcBitArray_Destruct(&ba0); CcBitArray_Destruct(&ba1);

	COCO_ASSERT((CcBitArray(&ba0, numbits)));
	COCO_ASSERT((CcBitArray(&ba1, numbits))); CcBitArray_SetAll(&ba1, 1);
	ATest(fp, &ba0, &ba1);
	CcBitArray_Destruct(&ba0); CcBitArray_Destruct(&ba1);

	COCO_ASSERT((CcBitArray(&ba0, numbits))); CcBitArray_SetAll(&ba0, 1);
	COCO_ASSERT((CcBitArray(&ba1, numbits))); CcBitArray_SetAll(&ba1, 1);
	ATest(fp, &ba0, &ba1);
	CcBitArray_Destruct(&ba0); CcBitArray_Destruct(&ba1);

	for (idx = 0; idx < 128; ++idx) {
	    COCO_ASSERT((CcBitArray(&ba0, numbits))); CcBitArray_RandomSet(&ba0);
	    COCO_ASSERT((CcBitArray(&ba1, numbits))); CcBitArray_RandomSet(&ba1);
	    ATest(fp, &ba0, &ba1);

	    COCO_ASSERT((CcBitArray_Clone(&ba2, &ba0)));
	    CcBitArray_Subtract(&ba2, &ba1);

	    ATest(fp, &ba0, &ba2);
	    ATest(fp, &ba2, &ba0);
	    ATest(fp, &ba1, &ba2);
	    ATest(fp, &ba2, &ba1);

	    CcBitArray_Destruct(&ba0); CcBitArray_Destruct(&ba1); CcBitArray_Destruct(&ba2);
	}
    }
}
