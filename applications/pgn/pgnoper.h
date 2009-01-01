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
#ifndef COCO_PGNOPER_H
#define COCO_PGNOPER_H

#ifndef COCO_CDEFS_H
#include "c/CDefs.h"
#endif

EXTC_BEGIN

typedef enum {
    pgnBlank = ' ',
    wKind = 'K', wQueen = 'Q', wRook = 'R',
    wBishop = 'B', wKnight = 'N', wPawn = 'P',
    bKind = 'k', bQueen = 'q', bRook = 'r',
    bBishop = 'b', bKnight = 'n', bPawn = 'p'
}  PgnPiece_t;

typedef struct PgnBoard_s PgnBoard_t;
typedef struct PgnMove_s PgnMove_t;
typedef struct PgnMovesArr_s PgnMovesArr_t;
typedef struct PgnGame_s PgnGame_t;

struct PgnBoard_s {
    char board[8][9];
    int white_material;
    int black_material;
};
PgnBoard_t * PgnBoard(PgnBoard_t * self);
void PgnBoard_Destruct(PgnBoard_t * self);

CcsBool_t PgnBoard_Move(PgnBoard_t * self, const PgnMove_t * move);
CcsBool_t PgnBoard_Revoke(PgnBoard_t * self, const PgnMove_t * move);

struct PgnMove_s {
    CcsBool_t WhiteOrNot;
    int start, end;
    PgnPiece_t preferment;
    char * value;
};
PgnMove_t * PgnMove(CcsBool_t WhiteOrNot, const char * value);
void PgnMove_Destruct(PgnMove_t * self);

#define  SZ_MOVES_ARR 32
struct PgnMovesArr_s {
    PgnMovesArr_t * next;
    PgnMove_t ** cur;
    PgnMove_t * moves[SZ_MOVES_ARR];
};

struct PgnGame_s {
    PgnGame_t * next;

    PgnBoard_t board;

    char * Event;
    char * Site;
    char * Date;
    char * Round;
    char * White;
    char * Black;
    char * WhiteElo;
    char * BlackElo;
    char * TimeControl;
    char * Result;
    char * resultInfo;

    PgnMovesArr_t movesArr;
    PgnMovesArr_t * movesArrLast;
};

PgnGame_t *
PgnGame(const char * Event, const char * Site, const char * Date,
	const char * Round, const char * White, const char * Black,
	const char * WhiteElo, const char * BlackElo, const char * TimeControl,
	const char * Result);
void PgnGame_Destruct(PgnGame_t * self);

void PgnGame_AppendRound(PgnGame_t * self, PgnMove_t * move);

EXTC_END

#endif /* COCO_PGNOPER_H */
