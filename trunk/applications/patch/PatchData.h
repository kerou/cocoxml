/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#ifndef  COCO_PATCHDATA_H
#define  COCO_PATCHDATA_H

#ifndef  COCO_CDEFS_H
#include "c/CDefs.h"
#endif

#ifndef  COCO_PatchScanner_H
#include "Scanner.h"
#endif

EXTC_BEGIN

typedef struct PatchLine_s PatchLine_t;
typedef struct PatchPiece_s PatchPiece_t;
typedef struct PatchFile_s PatchFile_t;

struct PatchLine_s {
    PatchLine_t * next;
    char * content;
};

struct PatchPiece_s {
    PatchPiece_t * next;
    int subStart, subNum;
    int addStart, addNum;
    PatchLine_t * lines;
    CcsBool_t subLastEol;
    CcsBool_t addLastEol;
};

struct PatchFile_s {
    PatchFile_t * next;
    char * subFname;
    char * addFname;
    PatchPiece_t * firstPiece;
    PatchPiece_t * lastPiece;
};

PatchLine_t *
PatchLineList(PatchScanner_t * scanner, int subStart, int subNum, int addStart,
	      int addNum, CcsBool_t * subLastEol, CcsBool_t * addLastEol);
void PatchLineList_Destruct(PatchLine_t * self);

PatchPiece_t *
PatchPiece(int subStart, int subNum, int addStart, int addNum,
	   PatchLine_t * lines, CcsBool_t subLastEol, CcsBool_t addLastEol);
void PatchPiece_Destruct(PatchPiece_t * self);

PatchFile_t * PatchFile(const char * subFname, const char * addFname);
void PatchFile_Destruct(PatchFile_t * self);
void PatchFile_Append(PatchFile_t * self, PatchPiece_t * piece);

EXTC_END

#endif /* COCO_PATCHDATA_H */
