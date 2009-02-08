/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#include  "Parser.h"

int
main(int argc, char * argv[])
{
    PatchParser_t parser;

    if (argc != 2) {
	fprintf(stderr, "%s PATCH-FILENAME\n", argv[0]);
	goto errquit0;
    }
    if (!strcmp(argv[1], "-")) {
	if (!PatchParser(&parser, stdin, stderr)) goto errquit0;
    } else {
	if (!PatchParser_ByName(&parser, argv[1], stderr)) goto errquit0;
    }
    PatchParser_Parse(&parser);
    PatchParser_Destruct(&parser);
    return 0;
 errquit0:
    return -1;
}
