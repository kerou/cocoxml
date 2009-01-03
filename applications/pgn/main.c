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

#define LNWIDTH  60
#define LNWIDTH2 30

static void
PgnGame_Show(PgnGame_t * game)
{
    char lnbuf[LNWIDTH + 1], sidebuf[LNWIDTH2 + 1];
    lnbuf[LNWIDTH] = 0;

    memset(lnbuf, '-', LNWIDTH);
    printf("%s\n", lnbuf);

    memset(lnbuf, ' ', LNWIDTH);
    lnbuf[LNWIDTH2 - 1] = 'v'; lnbuf[LNWIDTH2] = 's';
    snprintf(sidebuf, LNWIDTH2 - 3, "%s(%d)", game->White, game->WhiteElo);
    memcpy(lnbuf + 1 + (LNWIDTH2 - 3 - strlen(sidebuf)) / 2,
	   sidebuf, strlen(sidebuf));
    snprintf(sidebuf, sizeof(sidebuf), "%s(%d)", game->Black, game->BlackElo);
    memcpy(lnbuf + LNWIDTH2 + 2 + (LNWIDTH2 - 3 - strlen(sidebuf)) / 2,
	   sidebuf, strlen(sidebuf));
    printf("%s\n", lnbuf);
    printf("------------------------------------------------------------\n");
}

int
main(int argc, char * argv[])
{
    PgnParser_t parser;
    PgnGame_t * cur;

    if (argc != 2) {
	fprintf(stderr, "%s PGN-FILENAME\n", argv[0]);
	goto errquit0;
    }
    if (!PgnParser(&parser, argv[1], stderr)) goto errquit0;
    PgnParser_Parse(&parser);
    for (cur = parser.firstGame; cur; cur = cur->next)
	PgnGame_Show(cur);
    PgnParser_Destruct(&parser);
    return 0;
 errquit0:
    return -1;
}
