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
#include  "pgnoper.h"

PgnBoard_t *
PgnBoard(PgnBoard_t * self)
{
    strcpy(self->board[0], "RNBQKBNR");
    strcpy(self->board[1], "PPPPPPPP");
    strcpy(self->board[2], "        ");
    strcpy(self->board[3], "        ");
    strcpy(self->board[4], "        ");
    strcpy(self->board[5], "        ");
    strcpy(self->board[6], "pppppppp");
    strcpy(self->board[6], "rnbqkbnr");
    self->white_material = 39;
    self->white_material = 39;
    return self;
}

void PgnBoard_Destruct(PgnBoard_t * self)
{
}

CcsBool_t
PgnBoard_Move(PgnBoard_t * self, const PgnMove_t * move)
{
}

CcsBool_t
PgnBoard_Revoke(PgnBoard_t * self, const PgnMove_t * move)
{
}

PgnMove_t *
PgnMove(CcsBool_t WhiteOrNot, const char * value)
{
    PgnMove_t * self;
    if (!(self = CcsMalloc(sizeof(PgnMove_t) + strlen(value) + 1)))
	return NULL;
    self->WhiteOrNot = WhiteOrNot;
    self->start = -1; /* FIX ME: */
    self->end = -1; /* FIX ME: */
    self->preferment = pgnBlank; /* FIX ME: */
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
    strcpy(*cur, str); *cur += strlen(str);
    return ret;
}

PgnGame_t *
PgnGame(const char * Event, const char * Site, const char * Date,
	const char * Round, const char * White, const char * Black,
	const char * WhiteElo, const char * BlackElo, const char * TimeControl,
	const char * Result)
{
    PgnGame_t * self;
    char * cur;
    size_t len = safe_strlen(Event) + safe_strlen(Site) + safe_strlen(Date) +
	safe_strlen(Round) + safe_strlen(White) + safe_strlen(Black) +
	safe_strlen(WhiteElo) + safe_strlen(BlackElo) +
	safe_strlen(TimeControl) + safe_strlen(Result);
    if (!(self = CcsMalloc(sizeof(PgnGame_t) + len))) goto errquit0;
    if (!PgnBoard(&self->board)) goto errquit1;
    cur = (char *)(self + 1);
    self->Event = safe_append(&cur, Event);
    self->Site = safe_append(&cur, Site);
    self->Date = safe_append(&cur, Date);
    self->Round = safe_append(&cur, Round);
    self->White = safe_append(&cur, White);
    self->Black = safe_append(&cur, Black);
    self->WhiteElo = safe_append(&cur, WhiteElo);
    self->BlackElo = safe_append(&cur, BlackElo);
    self->TimeControl = safe_append(&cur, TimeControl);
    self->Result = safe_append(&cur, Result);
    self->resultInfo = NULL;
    self->movesArr.next = NULL;
    self->movesArr.cur = self->movesArr.moves;
    self->movesArrLast = &self->movesArr;
    return self;
 errquit1:
    CcsFree(self);
 errquit0:
    return NULL;
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
    PgnBoard_Destruct(&self->board);
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
    }
    *self->movesArrLast->cur++ = move;
    return TRUE;
}
