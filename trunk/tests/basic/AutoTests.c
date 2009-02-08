/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  <stdio.h>
#include  <stdlib.h>
#include  "AutoTests.h"

void TestBitArray(FILE * fp);
void TestCharSet(FILE * fp);

int
main(void)
{
    FILE * fp;

    if (!(fp = fopen("AutoTests.log", "w"))) {
	fprintf(stderr, "Open AutoTests.log for written failed.\n");
	exit(-1);
    }
    TestBitArray(fp);
    TestCharSet(fp);
    fclose(fp);
    return 0;
}

void
coco_assert_failed(const char * fname, int lineno)
{
    /* Breakpoint can be set here. */
    fprintf(stderr, "Assert failed at: %s#%d.\n", fname, lineno);
    exit(-1);
}
