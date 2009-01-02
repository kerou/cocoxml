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
    int passRow;
    PgnPiece_t king;
    PgnPiece_t queen;
    PgnPiece_t rook;
    PgnPiece_t bishop;
    PgnPiece_t knight;
    PgnPiece_t pawn;
}  PgnInfo_t;
static const PgnInfo_t whiteInfo = {
    0, 4, wKing, wQueen, wRook, wBishop, wKnight, wPawn
};
static const PgnInfo_t blackInfo = {
    7, 3, bKing, bQueen, bRook, bBishop, bKnight, bPawn
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
    char * curtgt; const char * cursrc; size_t valuelen;
    const PgnInfo_t * info = WhiteOrNot ? &whiteInfo : &blackInfo;

    if (!(self = CcsMalloc(sizeof(PgnMove_t) + strlen(value) + 2)))
	return NULL;
    memset(self, 0, sizeof(PgnMove_t));
    self->WhiteOrNot = WhiteOrNot;

    curtgt = (char *)(self + 1); cursrc = value;
    valuelen = strspn(value, "KQRNBabcdefgh12345678xO-");
    self->value = curtgt;
    memcpy(curtgt, cursrc, valuelen);
    curtgt[valuelen] = 0;
    curtgt += valuelen + 1; cursrc += valuelen;

    self->upgrade = pgnBlank;
    if (cursrc[0] == '=') {
	switch (cursrc[1]) {
	case 'Q': self->upgrade = info->queen; cursrc += 2; break;
	case 'R': self->upgrade = info->rook; cursrc += 2; break;
	case 'B': self->upgrade = info->bishop; cursrc += 2; break;
	case 'N': self->upgrade = info->knight; cursrc += 2; break;
	}
    }

    self->annotation = curtgt;
    strcpy(curtgt, cursrc);
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

static const char * Xstr = "abcdefgh";
static const char * Ystr = "12345678";

static void
PgnGame_SearchKing(const PgnGame_t * self, PgnMove_t * move)
{
}
static void
PgnGame_SearchQueen(const PgnGame_t * self, PgnMove_t * move)
{
}
static void
PgnGame_SearchRook(const PgnGame_t * self, PgnMove_t * move)
{
}
static void
PgnGame_SearchBishop(const PgnGame_t * self, PgnMove_t * move)
{
}
static void
PgnGame_SearchKnight(const PgnGame_t * self, PgnMove_t * move)
{
}
static void
PgnGame_SearchPawn(const PgnGame_t * self, PgnMove_t * move)
{
}

static CcsBool_t
PgnGame_FillMove(const PgnGame_t * self, PgnMove_t * move)
{
    const char * curb, * cure, * c;
    const PgnGameStatus_t * status = &self->status;
    const PgnSide_t * side = move->WhiteOrNot ? &status->white : &status->black;
    const PgnInfo_t * info = move->WhiteOrNot ? &whiteInfo : &blackInfo;
    const PgnInfo_t * opinfo = move->WhiteOrNot ? &blackInfo : &whiteInfo;

    move->castling = side->castling; move->castlingL = side->castlingL;
    if (!strcmp(move->value, "O-O")) {
	if (!side->castling) return FALSE;
	move->fPiece = move->tPiece = info->king;
	move->fY = move->tY = info->castlingRow;
	move->fX = 4; move->tX = 6;
	move->kPiece = pgnBlank;
	return TRUE;
    } else if (!strcmp(move->value, "O-O-O")) {
	if (!side->castlingL) return FALSE;
	move->fPiece = move->tPiece = info->king;
	move->fY = move->tY = info->castlingRow;
	move->fX = 4; move->tX = 2;
	move->kPiece = pgnBlank;
	return TRUE;
    }
    /* Set move->fPiece, move->tPiece. */
    curb = move->value; cure = move->value + strlen(move->value) - 1;
    switch (*curb) {
    case 'K': move->fPiece = info->king; ++curb; break;
    case 'Q': move->fPiece = info->queen; ++curb; break;
    case 'R': move->fPiece = info->rook; ++curb; break;
    case 'B': move->fPiece = info->bishop; ++curb; break;
    case 'N': move->fPiece = info->knight; ++curb; break;
    default: move->fPiece = info->pawn; break;
    }
    move->tPiece = move->upgrade == pgnBlank ? move->fPiece : move->upgrade;
    /* Set move->fX, move->fY, move->tX, move->tY */
    move->fX = move->fY = move->tX = move->tY = -1;
    if (curb <= cure && (c = strchr(Ystr, *cure))) {
	move->tY = c - Ystr; --cure;
    }
    if (curb <= cure && (c = strchr(Xstr, *cure))) {
	move->tX = c - Xstr; --cure;
    }
    if (curb <= cure && (c = strchr(Xstr, *curb))) {
	move->fX = c - Xstr; ++curb;
    }
    if (curb <= cure && (c = strchr(Ystr, *curb))) {
	move->fY = c - Ystr; ++curb;
    }
    if (move->fX == -1 || move->fY == -1 || move->tX == -1 || move->tY == -1) {
	if (move->fPiece == info->king) PgnGame_SearchKing(self, move);
	else if (move->fPiece == info->queen) PgnGame_SearchQueen(self, move);
	else if (move->fPiece == info->rook) PgnGame_SearchRook(self, move);
	else if (move->fPiece == info->bishop) PgnGame_SearchBishop(self, move);
	else if (move->fPiece == info->knight) PgnGame_SearchKnight(self, move);
	else if (move->fPiece == info->pawn) PgnGame_SearchPawn(self, move);
    }
    if (move->fX == -1 || move->fY == -1 || move->tX == -1 || move->tY == -1)
	return FALSE;
    /* Set move->kPiece, move->kX, move->kY */
    if (move->fPiece == info->pawn && move->fY == info->passRow &&
	move->fX != move->tX && status->board[move->tY][move->tX] == pgnBlank) {
	/* Pawn Pass detected. */
	if (status->board[move->fY][move->tX] == opinfo->pawn) return FALSE;
	move->kPiece = opinfo->pawn; move->kX = move->tX; move->kY = move->fY;
    } else {
	move->kPiece = status->board[move->tY][move->tX];
	move->kX = move->tX; move->kY = move->tY;
    }
    return TRUE;
}

CcsBool_t
PgnGame_AppendMove(PgnGame_t * self, PgnMove_t * move)
{
    PgnMovesArr_t * newMovesArr;

    PgnGame_ToEnd(self);
    if (!PgnGame_FillMove(self, move)) return FALSE;
    if (self->moveLast - self->movesArrLast->moves >= SZ_MOVES_ARR) {
	if (!(newMovesArr = CcsMalloc(sizeof(PgnMovesArr_t)))) return FALSE;
	newMovesArr->prev = self->movesArrLast;
	newMovesArr->next = NULL;
	self->movesArrLast->next = newMovesArr;
	self->movesArrLast = newMovesArr;
	self->moveLast = self->movesArrLast->moves;
    }
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
    status->board[cur->fY][cur->fX] = cur->fPiece;
    status->board[cur->tY][cur->tX] = pgnBlank;
    if (cur->fPiece != cur->tPiece)
	side->material -=
	    PgnPiece2Material(cur->tPiece) - PgnPiece2Material(cur->fPiece);
    if (cur->kPiece != pgnBlank) {
	CcsAssert(status->board[cur->kY][cur->kX] == pgnBlank);
	status->board[cur->kY][cur->kX] = cur->kPiece;
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
	CcsAssert(status->board[cur->kY][cur->kX] == cur->kPiece);
	status->board[cur->kY][cur->kX] = pgnBlank;
	opside->material -= PgnPiece2Material(cur->kPiece);
    }
    status->board[cur->fY][cur->fX] = pgnBlank;
    status->board[cur->tY][cur->tX] = cur->tPiece;
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
