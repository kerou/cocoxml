/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "lexical/CharSet.h"
#include  "AutoTests.h"

#define   NUMBITS  32

static void
CcCharSet_AllSet(CcCharSet_t * self)
{
    int idx;
    for (idx = 0; idx < NUMBITS; ++idx)
	CcCharSet_Set(self, idx);
}

static void
CcCharSet_RandomSet(CcCharSet_t * self)
{
    long int rnd;
    int idx, bit;
    for (idx = 0; idx < NUMBITS; idx += 16) {
	rnd = rand();
	for (bit = 0; bit < 16; ++bit)
	    if ((rnd & (1 << bit))) CcCharSet_Set(self, idx + bit);
    }
}

static void
CcCharSet_Dump(const CcCharSet_t * self, FILE * fp,
	       const char * prefix, const char * suffix)
{
    const CcRange_t * cur;
    fprintf(fp, "%s", prefix);
    for (cur = self->head; cur; cur = cur->next) {
	if (cur->from == cur->to) {
	    fprintf(fp, "%d", cur->from);
	} else {
	    fprintf(fp, "%d..%d", cur->from, cur->to);
	}
	if (cur->next) fprintf(fp, ", ");
    }
    fprintf(fp, "%s", suffix);
}

static void
ATest(FILE * fp, CcCharSet_t * cs0, CcCharSet_t * cs1)
{
    CcCharSet_t * cs2;
    int idx, cnt;

    CcCharSet_Dump(cs0, fp, "Charset 0: ", "\n");
    CcCharSet_Dump(cs1, fp, "Charset 1: ", "\n");

    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx)
	if (CcCharSet_Get(cs0, idx) && CcCharSet_Get(cs1, idx)) cnt = 1;
    COCO_ASSERT((CcCharSet_Intersects(cs0, cs1) == cnt));

    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx)
	if (!CcCharSet_Get(cs0, idx) && CcCharSet_Get(cs1, idx)) cnt |= 1;
	else if (CcCharSet_Get(cs0, idx) && !CcCharSet_Get(cs1, idx)) cnt |= 2;
    switch (cnt) {
    case 0: COCO_ASSERT((CcCharSet_Equals(cs0, cs1))); break;
    case 1: COCO_ASSERT((CcCharSet_Includes(cs1, cs0))); break;
    case 2: COCO_ASSERT((CcCharSet_Includes(cs0, cs1))); break;
    case 3:
	COCO_ASSERT((!CcCharSet_Includes(cs0, cs1)));
	COCO_ASSERT((!CcCharSet_Includes(cs1, cs0)));
	break;
    }

    COCO_ASSERT((cs2 = CcCharSet_Clone(cs0)));
    COCO_ASSERT((CcCharSet_Equals(cs0, cs2)));
    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx) {
	COCO_ASSERT(CcCharSet_Get(cs0, idx) == CcCharSet_Get(cs2, idx));
	if (CcCharSet_Get(cs0, idx)) ++cnt;
    }
    COCO_ASSERT((CcCharSet_Elements(cs0) == cnt));

    CcCharSet_Or(cs2, cs1);
    CcCharSet_Dump(cs2, fp, "Or: ", "\n");
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CcCharSet_Get(cs0, idx) || CcCharSet_Get(cs1, idx)) == CcCharSet_Get(cs2, idx));
    COCO_ASSERT((CcCharSet_Includes(cs2, cs0)));
    COCO_ASSERT((CcCharSet_Includes(cs2, cs1)));
    CcCharSet_Destruct(cs2);

    COCO_ASSERT((cs2 = CcCharSet_Clone(cs0)));
    CcCharSet_And(cs2, cs1);
    CcCharSet_Dump(cs2, fp, "And: ", "\n");
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CcCharSet_Get(cs0, idx) && CcCharSet_Get(cs1, idx)) == CcCharSet_Get(cs2, idx));
    COCO_ASSERT((CcCharSet_Includes(cs0, cs2)));
    COCO_ASSERT((CcCharSet_Includes(cs1, cs2)));
    CcCharSet_Destruct(cs2);

    COCO_ASSERT((cs2 = CcCharSet_Clone(cs0)));
    CcCharSet_Subtract(cs2, cs1);
    CcCharSet_Dump(cs2, fp, "Subtract: ", "\n");
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CcCharSet_Get(cs0, idx) && !CcCharSet_Get(cs1, idx)) == CcCharSet_Get(cs2, idx));
    COCO_ASSERT(CcCharSet_Includes(cs0, cs2));
    COCO_ASSERT(!CcCharSet_Intersects(cs1, cs2));
    CcCharSet_Destruct(cs2);

    fprintf(fp, "\n");
}

void
TestCharSet(FILE * fp)
{
    CcCharSet_t * cs0,  * cs1, * cs2;
    int idx;

    COCO_ASSERT((cs0 = CcCharSet()));
    COCO_ASSERT((cs1 = CcCharSet()));
    ATest(fp, cs0, cs1);
    CcCharSet_Destruct(cs0); CcCharSet_Destruct(cs1);

    COCO_ASSERT((cs0 = CcCharSet())); CcCharSet_AllSet(cs0);
    COCO_ASSERT((cs1 = CcCharSet()));
    ATest(fp, cs0, cs1);
    CcCharSet_Destruct(cs0); CcCharSet_Destruct(cs1);

    COCO_ASSERT((cs0 = CcCharSet()));
    COCO_ASSERT((cs1 = CcCharSet())); CcCharSet_AllSet(cs1);
    ATest(fp, cs0, cs1);
    CcCharSet_Destruct(cs0); CcCharSet_Destruct(cs1);

    COCO_ASSERT((cs0 = CcCharSet())); CcCharSet_AllSet(cs0);
    COCO_ASSERT((cs1 = CcCharSet())); CcCharSet_AllSet(cs1);
    ATest(fp, cs0, cs1);
    CcCharSet_Destruct(cs0); CcCharSet_Destruct(cs1);

    for (idx = 0; idx < 128; ++idx) {
	COCO_ASSERT((cs0 = CcCharSet())); CcCharSet_RandomSet(cs0);
	COCO_ASSERT((cs1 = CcCharSet())); CcCharSet_RandomSet(cs1);
	ATest(fp, cs0, cs1);

	COCO_ASSERT((cs2 = CcCharSet_Clone(cs0)));
	CcCharSet_Subtract(cs2, cs1);

	ATest(fp, cs0, cs2);
	ATest(fp, cs2, cs0);
	ATest(fp, cs1, cs2);
	ATest(fp, cs2, cs1);

	CcCharSet_Destruct(cs0); CcCharSet_Destruct(cs1);
	CcCharSet_Destruct(cs2);
    }
}
