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

typedef struct {
    int castlingRow;
    PgnPiece_t king;
    PgnPiece_t queen;
    PgnPiece_t rook;
    PgnPiece_t bishop;
    PgnPiece_t knight;
    PgnPiece_t pawn;
}  PgnInfo_t;
static const PgnInfo_t whiteInfo = {
    0, wKing, wQueen, wRook, wBishop, wKnight, wPawn
};
static const PgnInfo_t blackInfo = {
    0, bKing, bQueen, bRook, bBishop, bKnight, bPawn
};

static int
PgnPiece2Material(PgnPiece_t piece)
{
    switch (piece) {
    case wQueen: case bQueen: return 9;
    case wRook: case bRook: return 5;
    case wBishop: case bBishop: return 3;
    case wKnight: case bKnight: return 3;
    case wPawn: case bPawn: return 1;
    default: break;
    }
    return 0;
}

void
PgnGameStatus_Clone(PgnGameStatus_t * self, const PgnGameStatus_t * status)
{
    memcpy(self, status, sizeof(*self));
}

const PgnGameStatus_t PgnStandardStart = {
    { "RNBQKBNR",
      "PPPPPPPP",
      "........",
      "........",
      "........",
      "........",
      "pppppppp",
      "rnbqkbnr" },
    { 39, TRUE, TRUE },
    { 39, TRUE, TRUE }
};

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
PgnGame(const PgnGameStatus_t * status,
	const char * Event, const char * Site, const char * Date,
	const char * Round, const char * White, const char * Black,
	const char * WhiteElo, const char * BlackElo, const char * TimeControl)
{
    PgnGame_t * self;  char * cur;
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
    self->WhiteElo = WhiteElo && *WhiteElo == '"' ? atoi(WhiteElo + 1) : -1;
    self->BlackElo = BlackElo && *BlackElo == '"' ? atoi(BlackElo + 1) : -1;
    self->TimeControl = safe_append(&cur, TimeControl);
    self->Result = NULL;
    self->resultInfo = NULL;

    PgnGameStatus_Clone(&self->startStatus, status);
    PgnGameStatus_Clone(&self->status, status);

    self->movesArr.prev = NULL;
    self->movesArr.next = NULL;
    self->movesArrLast = &self->movesArr;
    self->moveCur = self->moveLast = self->movesArrLast->moves;
    return self;
}

void
PgnGame_Destruct(PgnGame_t * self)
{
    PgnMovesArr_t * curarr, * nextarr;
    PgnMove_t ** cur, ** last;

    for (curarr = &self->movesArr; curarr; curarr = nextarr) {
	nextarr = curarr->next;
	last = nextarr ? curarr->moves + SZ_MOVES_ARR : self->moveLast;
	for (cur = curarr->moves; cur < last; ++cur)
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

    PgnGame_ToEnd(self);
    if (self->moveLast - self->movesArrLast->moves >= SZ_MOVES_ARR) {
	if (!(newMovesArr = CcsMalloc(sizeof(PgnMovesArr_t)))) return FALSE;
	newMovesArr->prev = self->movesArrLast;
	newMovesArr->next = NULL;
	self->movesArrLast->next = newMovesArr;
	self->movesArrLast = newMovesArr;
	self->moveLast = self->movesArrLast->moves;
    }
    /* Fill the other members of move according to
     * the current status of board. */
    *self->moveLast++ = move;
    PgnGame_Forward(self);
    return TRUE;
}

void
PgnGame_ToStart(PgnGame_t * self)
{
    PgnGameStatus_Clone(&self->status, &self->startStatus);
    self->movesArrCur = &self->movesArr;
    self->moveCur = self->movesArrCur->moves;
}
void
PgnGame_Backward(PgnGame_t * self)
{
    PgnMove_t * cur;
    PgnSide_t * side, * opside;
    const PgnInfo_t * info;
    PgnGameStatus_t * status = &self->status;

    if (self->moveCur == self->movesArr.moves) return;
    /* Backward cur */
    if (self->moveCur > self->movesArrCur->moves) {
	--self->moveCur;
    } else {
	CcsAssert(self->movesArrCur->prev != NULL);
	self->movesArrCur = self->movesArrCur->prev;
	self->moveCur = self->movesArrCur->moves + (SZ_MOVES_ARR - 1);
    }
    cur = *self->moveCur;
    side = cur->WhiteOrNot ? &status->white : &status->black;
    opside = cur->WhiteOrNot ? &status->black : &status->white;
    info = cur->WhiteOrNot ? &whiteInfo : &blackInfo;
    /* Basic moves */
    status->board[cur->fX][cur->fY] = cur->fPiece;
    status->board[cur->tX][cur->tY] = pgnBlank;
    if (cur->fPiece != cur->tPiece)
	side->material -=
	    PgnPiece2Material(cur->tPiece) - PgnPiece2Material(cur->fPiece);
    if (cur->kPiece != pgnBlank) {
	CcsAssert(status->board[cur->kX][cur->kY] == pgnBlank);
	status->board[cur->kX][cur->kY] = cur->kPiece;
	opside->material += PgnPiece2Material(cur->kPiece);
    }
    /* Deal with castling & castlingL */
    side->castling = cur->castling; side->castlingL = cur->castlingL;
    if (cur->fY == info->castlingRow && cur->tY == info->castlingRow &&
	cur->fPiece == info->king && cur->fX == 4) {
	if (cur->tX == 6) { /* castling */
	    CcsAssert(status->board[info->castlingRow][7] == info->rook);
	    CcsAssert(status->board[info->castlingRow][5] == pgnBlank);
	    status->board[info->castlingRow][7] = pgnBlank;
	    status->board[info->castlingRow][5] = info->rook;
	} else if (cur->tY == 2) { /* castlingL */
	    CcsAssert(status->board[info->castlingRow][0] == info->rook);
	    CcsAssert(status->board[info->castlingRow][1] == pgnBlank);
	    CcsAssert(status->board[info->castlingRow][3] == pgnBlank);
	    status->board[info->castlingRow][0] = pgnBlank;
	    status->board[info->castlingRow][3] = info->rook;
	}
    }
}

void
PgnGame_Forward(PgnGame_t * self)
{
    PgnGameStatus_t * status = &self->status;
    PgnMove_t * cur = *self->moveCur;
    PgnSide_t * side = cur->WhiteOrNot ? &status->white : &status->black;
    PgnSide_t * opside = cur->WhiteOrNot ? &status->black : &status->white;
    const PgnInfo_t * info = cur->WhiteOrNot ? &whiteInfo : &blackInfo;

    if (self->moveCur == self->moveLast) return;
    /* Basic moves */
    if (cur->kPiece != pgnBlank) {
	CcsAssert(status->board[cur->kX][cur->kY]
		  == cur->kPiece);
	status->board[cur->kX][cur->kY] = pgnBlank;
	opside->material -= PgnPiece2Material(cur->kPiece);
    }
    status->board[cur->fX][cur->fY] = pgnBlank;
    status->board[cur->tX][cur->tX] = cur->tPiece;
    if (cur->fPiece != cur->tPiece)
	side->material +=
	    PgnPiece2Material(cur->tPiece) - PgnPiece2Material(cur->fPiece);
    /* Deal with castling & castlingL */
    CcsAssert(cur->castling == side->castling);
    CcsAssert(cur->castlingL == side->castlingL);
    if ((side->castling || side->castlingL) &&
	cur->fY == info->castlingRow) {
	if (cur->fPiece == info->king) {
	    if (cur->fX == 4 && cur->tX == 6) { /* Short castling */
		CcsAssert(side->castling);
		CcsAssert(status->board[info->castlingRow][0] == info->rook);
		CcsAssert(status->board[info->castlingRow][1] == pgnBlank);
		CcsAssert(status->board[info->castlingRow][3] == pgnBlank);
		status->board[info->castlingRow][0] = pgnBlank;
		status->board[info->castlingRow][3] = info->rook;
	    } else if (cur->fX == 4 && cur->tX == 2) { /* Long castling */
		CcsAssert(side->castlingL);
		CcsAssert(status->board[info->castlingRow][7] == info->rook);
		CcsAssert(status->board[info->castlingRow][5] == pgnBlank);
		status->board[info->castlingRow][7] = pgnBlank;
		status->board[info->castlingRow][5] = info->rook;
	    }
	    side->castling = side->castlingL = FALSE;
	} else if (cur->fPiece == info->rook) {
	    if (side->castling && cur->fX == 7) side->castling = FALSE;
	    else if (side->castlingL && cur->fX == 0) side->castlingL = FALSE;
	}
    }
    /* Forward cur */
    ++self->moveCur;
    if (self->moveCur >= self->movesArrCur->moves + SZ_MOVES_ARR) {
	CcsAssert(self->movesArrCur->next != NULL);
	self->movesArrCur = self->movesArrCur->next;
	self->moveCur = self->movesArrCur->moves;
    }
}

void
PgnGame_ToEnd(PgnGame_t * self)
{
    while (self->moveCur != self->moveLast) PgnGame_Forward(self);
}
