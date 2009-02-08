/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "c/Scanner.h"
#include  "c/ScanInput.h"

int
main(int argc, char * argv[])
{
    FILE * infp;
    CcsScanner_t scanner;
    CcsToken_t * t, * la;

    if (argc != 2) {
	fprintf(stderr, "argc != 2\n");
	return -1;
    }
    if (!(infp = fopen(argv[1], "r"))) {
	fprintf(stderr, "fopen(%s, \"r\") failed\n", argv[1]);
	return -1;
    }
    CcsScanner(&scanner, NULL, infp);
    t = NULL; la = CcsScanner_GetDummy(&scanner);
    for (;;) {
	if (t) CcsScanner_TokenDecRef(&scanner, t);
	t = la;
	if (t) {
	    printf("CcsToken(%s:%d,%d): kind = %d pos = %d: [%s]\n",
		   t->loc.fname, t->loc.line, t->loc.col,
		   t->kind, t->pos, t->val);
	}
	la = CcsScanner_Scan(&scanner);
	if (la->kind == scanner.cur->info->eofSym) break;
    }
    if (t) CcsScanner_TokenDecRef(&scanner, t);
    if (la) CcsScanner_TokenDecRef(&scanner, la);
    CcsScanner_Destruct(&scanner);
    fclose(infp);
    return 0;
}
