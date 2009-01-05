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
#include  "Parser.h"

int
main(int argc, char * argv[])
{
    FILE * infp;
    CExprParser_t parser;

    if (argc != 2) {
	fprintf(stderr, "%s FILENAME\n", argv[0]);
	goto errquit0;
    }
    if (!strcmp(argv[1], "-")) infp = stdin;
    else if (!(infp = fopen(argv[1], "r"))) goto errquit0;
    if (!CExprParser(&parser, infp, stderr)) goto errquit1;
    CExprParser_Parse(&parser);
    printf("parser.value = %d\n", parser.value);
    CExprParser_Destruct(&parser);
    if (strcmp(argv[1], "-")) fclose(infp);
    return 0;
 errquit1:
    if (strcmp(argv[1], "-")) fclose(infp);
 errquit0:
    return -1;
}
