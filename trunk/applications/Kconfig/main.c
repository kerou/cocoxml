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
-------------------------------------------------------------------------*/
#include  "desc/Parser.h"
#include  "file/Parser.h"

int
main(int argc, char * argv[])
{
    FILE * kcfp, * cffp;
    KcParser_t kcparser;
    CfParser_t cfparser;

    if (argc != 3) {
	fprintf(stderr, "%s Kconfig-FILENAME Config-FILENAME\n", argv[0]);
	goto errquit0;
    }

    if (!strcmp(argv[1], "-")) kcfp = stdin;
    else if (!(kcfp = fopen(argv[1], "r"))) goto errquit0;
    if (!KcParser(&kcparser, kcfp, stderr)) goto errquit1;
    KcParser_Parse(&kcparser);

    if (!strcmp(argv[2], "-")) cffp = stdin;
    else if (!(cffp = fopen(argv[2], "r"))) goto errquit2;
    if (!CfParser(&cfparser, cffp, stderr)) goto errquit3;
    CfParser_Parse(&cfparser);

    CfParser_Destruct(&cfparser);
    if (strcmp(argv[2], "-")) fclose(cffp);

    KcParser_Destruct(&kcparser);
    if (strcmp(argv[1], "-")) fclose(kcfp);
    return 0;
 errquit3:
    if (strcmp(argv[2], "-")) fclose(cffp);
 errquit2:
    KcParser_Destruct(&kcparser);
 errquit1:
    if (strcmp(argv[1], "-")) fclose(kcfp);
 errquit0:
    return -1;
}
