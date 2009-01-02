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
#include  "pgngame.h"

PgnMove_t *
PgnMove(CcsBool_t WhiteOrNot, const char * value)
{
    PgnMove_t * self;
    if (!(self = CcsMalloc(sizeof(PgnMove_t) + strlen(value) + 1)))
	return NULL;
    memset(self, 0, sizeof(PgnMove_t));
    self->WhiteOrNot = WhiteOrNot;
    self->value = (char *)(self + 1);
    strcpy(self->value, value);
    return self;
}

void
PgnMove_Destruct(PgnMove_t * self)
{
    CcsFree(self);
}

static size_t safe_strlen(const char * str)
{
    return str ? strlen(str) + 1: 0;
}
static char * safe_append(char ** cur, const char * str)
{
    char * ret;
    if (!str) return NULL;
    ret = *cur;
    strcpy(*cur, str); *cur += strlen(str) + 1;
    return ret;
}

PgnGame_t *
PgnGame(const char * Event, const char * Site, const char * Date,
	const char * Round, const char * White, const char * Black,
	const char * WhiteElo, const char * BlackElo, const char * TimeControl)
{
    PgnGame_t * self;
    char * cur;
    int whiteScore, blackScore;
    size_t len = safe_strlen(Event) + safe_strlen(Site) + safe_strlen(Date) +
	safe_strlen(Round) + safe_strlen(White) + safe_strlen(Black) +
	safe_strlen(TimeControl);
    if (!(self = CcsMalloc(sizeof(PgnGame_t) + len))) return NULL;
    self->next = NULL;
    
    cur = (char *)(self + 1);
    self->Event = safe_append(&cur, Event);
    self->Site = safe_append(&cur, Site);
    self->Date = safe_append(&cur, Date);
    self->Round = safe_append(&cur, Round);
    self->White = safe_append(&cur, White);
    self->Black = safe_append(&cur, Black);
    whiteScore = WhiteElo && *WhiteElo == '"' ? atoi(WhiteElo + 1) : -1;
    blackScore = BlackElo && *BlackElo == '"' ? atoi(BlackElo + 1) : -1;
    self->TimeControl = safe_append(&cur, TimeControl);
    self->Result = NULL;
    self->resultInfo = NULL;

    strcpy(self->board[0], "RNBQKBNR");
    strcpy(self->board[1], "PPPPPPPP");
    strcpy(self->board[2], "        ");
    strcpy(self->board[3], "        ");
    strcpy(self->board[4], "        ");
    strcpy(self->board[5], "        ");
    strcpy(self->board[6], "pppppppp");
    strcpy(self->board[6], "rnbqkbnr");
    self->white.playerName = self->White;
    self->white.playerScore = whiteScore;
    self->white.material = 39;
    self->white.castling = TRUE;
    self->white.castlingL = TRUE;
    self->black.playerName = self->Black;
    self->black.playerScore = blackScore;
    self->black.material = 39;
    self->black.castling = TRUE;
    self->black.castlingL = TRUE;

    self->movesArr.next = NULL;
    self->movesArr.cur = self->movesArr.moves;
    self->movesArrLast = &self->movesArr;
    return self;
}

void
PgnGame_Destruct(PgnGame_t * self)
{
    PgnMovesArr_t * curarr, * nextarr;
    PgnMove_t ** cur;

    for (curarr = &self->movesArr; curarr; curarr = nextarr) {
	nextarr = curarr->next;
	for (cur = curarr->moves; cur < curarr->cur; ++cur)
	    PgnMove_Destruct(*cur);
	if (curarr != &self->movesArr) CcsFree(curarr);
    }
    if (self->resultInfo) CcsFree(self->resultInfo);
    if (self->Result) CcsFree(self->Result);
    CcsFree(self);
}

CcsBool_t
PgnGame_AppendMove(PgnGame_t * self, PgnMove_t * move)
{
    PgnMovesArr_t * newMovesArr;
    if (self->movesArrLast->cur - self->movesArrLast->moves >= SZ_MOVES_ARR) {
	if (!(newMovesArr = CcsMalloc(sizeof(PgnMovesArr_t)))) return FALSE;
	newMovesArr->next = NULL;
	newMovesArr->cur = newMovesArr->moves;
	self->movesArrLast->next = newMovesArr;
	self->movesArrLast = newMovesArr;
    }
    *self->movesArrLast->cur++ = move;
    /* Fill the other members of move according to
     * the current status of board. */
    return TRUE;
}
