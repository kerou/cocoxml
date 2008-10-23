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
#include  <stdio.h>
#include  <stdlib.h>
#include  "CharSet.h"
#include  "AutoTests.h"

#define   NUMBITS  32

static void
CharSet_AllSet(CharSet_t * self)
{
    int idx;
    for (idx = 0; idx < NUMBITS; ++idx)
	CharSet_Set(self, idx);
}

static void
CharSet_RandomSet(CharSet_t * self)
{
    long int rnd;
    int idx, bit;
    for (idx = 0; idx < NUMBITS; idx += 16) {
	rnd = random();
	for (bit = 0; bit < 16; ++bit)
	    if ((rnd & (1 << bit))) CharSet_Set(self, idx + bit);
    }
}

static void
DumpCharSet(FILE * fp, const char * format, const CharSet_t * cs)
{
    DumpBuffer_t dbuf; char buf[1024];
    DumpBuffer(&dbuf, buf, sizeof(buf));
    CharSet_DumpInt(cs, &dbuf);
    fprintf(fp, format, buf);
}

static void
ATest(FILE * fp, CharSet_t * cs0, CharSet_t * cs1)
{
    CharSet_t cs2;
    int idx, cnt;

    DumpCharSet(fp, "Charset 0: %s\n", cs0);
    DumpCharSet(fp, "Charset 1: %s\n", cs1);

    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx)
	if (CharSet_Get(cs0, idx) && CharSet_Get(cs1, idx)) cnt = 1;
    COCO_ASSERT((CharSet_Intersects(cs0, cs1) == cnt));

    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx)
	if (!CharSet_Get(cs0, idx) && CharSet_Get(cs1, idx)) cnt |= 1;
	else if (CharSet_Get(cs0, idx) && !CharSet_Get(cs1, idx)) cnt |= 2;
    switch (cnt) {
    case 0: COCO_ASSERT((CharSet_Equals(cs0, cs1))); break;
    case 1: COCO_ASSERT((CharSet_Includes(cs1, cs0))); break;
    case 2: COCO_ASSERT((CharSet_Includes(cs0, cs1))); break;
    case 3:
	COCO_ASSERT((!CharSet_Includes(cs0, cs1)));
	COCO_ASSERT((!CharSet_Includes(cs1, cs0)));
	break;
    }

    COCO_ASSERT((CharSet_Clone(&cs2, cs0)));
    COCO_ASSERT((CharSet_Equals(cs0, &cs2)));
    cnt = 0;
    for (idx = 0; idx < NUMBITS; ++idx) {
	COCO_ASSERT(CharSet_Get(cs0, idx) == CharSet_Get(&cs2, idx));
	if (CharSet_Get(cs0, idx)) ++cnt;
    }
    COCO_ASSERT((CharSet_Elements(cs0) == cnt));

    COCO_ASSERT((CharSet_Or(&cs2, cs1) == 0));
    DumpCharSet(fp, "Or: %s\n", &cs2);
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CharSet_Get(cs0, idx) || CharSet_Get(cs1, idx)) == CharSet_Get(&cs2, idx));
    COCO_ASSERT((CharSet_Includes(&cs2, cs0)));
    COCO_ASSERT((CharSet_Includes(&cs2, cs1)));
    CharSet_Destruct(&cs2);

    COCO_ASSERT((CharSet_Clone(&cs2, cs0)));
    COCO_ASSERT((CharSet_And(&cs2, cs1) == 0));
    DumpCharSet(fp, "And: %s\n", &cs2);
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CharSet_Get(cs0, idx) && CharSet_Get(cs1, idx)) == CharSet_Get(&cs2, idx));
    COCO_ASSERT((CharSet_Includes(cs0, &cs2)));
    COCO_ASSERT((CharSet_Includes(cs1, &cs2)));
    CharSet_Destruct(&cs2);

    COCO_ASSERT((CharSet_Clone(&cs2, cs0)));
    COCO_ASSERT((CharSet_Subtract(&cs2, cs1) == 0));
    DumpCharSet(fp, "Subtract: %s\n", &cs2);
    for (idx = 0; idx < NUMBITS; ++idx)
	COCO_ASSERT((CharSet_Get(cs0, idx) && !CharSet_Get(cs1, idx)) == CharSet_Get(&cs2, idx));
    COCO_ASSERT(CharSet_Includes(cs0, &cs2));
    COCO_ASSERT(!CharSet_Intersects(cs1, &cs2));
    CharSet_Destruct(&cs2);

    fprintf(fp, "\n");
}

void
TestCharSet(FILE * fp)
{
    CharSet_t cs0, cs1, cs2;
    int idx;

    COCO_ASSERT((CharSet(&cs0)));
    COCO_ASSERT((CharSet(&cs1)));
    ATest(fp, &cs0, &cs1);
    CharSet_Destruct(&cs0); CharSet_Destruct(&cs1);

    COCO_ASSERT((CharSet(&cs0))); CharSet_AllSet(&cs0);
    COCO_ASSERT((CharSet(&cs1)));
    ATest(fp, &cs0, &cs1);
    CharSet_Destruct(&cs0); CharSet_Destruct(&cs1);

    COCO_ASSERT((CharSet(&cs0)));
    COCO_ASSERT((CharSet(&cs1))); CharSet_AllSet(&cs1);
    ATest(fp, &cs0, &cs1);
    CharSet_Destruct(&cs0); CharSet_Destruct(&cs1);

    COCO_ASSERT((CharSet(&cs0))); CharSet_AllSet(&cs0);
    COCO_ASSERT((CharSet(&cs1))); CharSet_AllSet(&cs1);
    ATest(fp, &cs0, &cs1);
    CharSet_Destruct(&cs0); CharSet_Destruct(&cs1);

    for (idx = 0; idx < 128; ++idx) {
	COCO_ASSERT((CharSet(&cs0))); CharSet_RandomSet(&cs0);
	COCO_ASSERT((CharSet(&cs1))); CharSet_RandomSet(&cs1);
	ATest(fp, &cs0, &cs1);

	COCO_ASSERT((CharSet_Clone(&cs2, &cs0)));
	COCO_ASSERT((CharSet_Subtract(&cs2, &cs1) == 0));

	ATest(fp, &cs0, &cs2);
	ATest(fp, &cs2, &cs0);
	ATest(fp, &cs1, &cs2);
	ATest(fp, &cs2, &cs1);

	CharSet_Destruct(&cs0); CharSet_Destruct(&cs1); CharSet_Destruct(&cs2);
    }
}
