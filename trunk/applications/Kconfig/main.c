/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#include  "desc/Parser.h"
#include  "file/Parser.h"
#include  "c/IncPathList.h"
#include  <limits.h>

int
main(int argc, char * argv[])
{
    KcParser_t kcparser;
    CfParser_t cfparser;
    char configdir[PATH_MAX];

    if (argc != 3) {
	fprintf(stderr, "%s Kconfig-FILENAME Config-FILENAME\n", argv[0]);
	goto errquit0;
    }

    CcsDirname(configdir, sizeof(configdir), argv[2]);
    if (!strcmp(argv[1], "-")) {
	if (!KcParser(&kcparser, stdin, stderr)) goto errquit0;
    } else {
	if (!KcParser_ByName(&kcparser, argv[1], stderr)) goto errquit0;
    }
    kcparser.incdirs = CcsIncPathListV(FALSE, FALSE, configdir, NULL);
    if (!strcmp(argv[2], "-")) {
	if (!CfParser(&cfparser, stdin, stderr)) goto errquit1;
    } else {
	if (!CfParser_ByName(&cfparser, argv[2], stderr)) goto errquit1;
    }
    KcParser_Parse(&kcparser);
    CfParser_Parse(&cfparser);

    CfParser_Destruct(&cfparser);
    KcParser_Destruct(&kcparser);
    return 0;
 errquit1:
    KcParser_Destruct(&kcparser);
 errquit0:
    return -1;
}
