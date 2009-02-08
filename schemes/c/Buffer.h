/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_BUFFER_H
#define  COCO_BUFFER_H

#ifndef  COCO_CDEFS_H
#include  "CDefs.h"
#endif

EXTC_BEGIN

typedef struct CcsBuffer_s CcsBuffer_t;
struct CcsBuffer_s {
    FILE * fp;
    long   start;
    char * buf;
    char * busyFirst; /* The first char used by Token_t. */
    char * lockCur;   /* The value of of cur when locked. */
    char * lockNext;  /* The value of next when locked. */
    char * cur;       /* The first char of the current char in ScanInput_t. */
    char * next;      /* The first char of the next char in ScanInput_t. */
    char * loaded;
    char * last;
};

/* CcsBuffer_t members for Scanner_t. */
/* fp should be opened with 'r' mode to deal with CR/LF. */
CcsBuffer_t * CcsBuffer(CcsBuffer_t * self, FILE * fp);
void CcsBuffer_Destruct(CcsBuffer_t * self);
long CcsBuffer_GetPos(CcsBuffer_t * self);
int CcsBuffer_Read(CcsBuffer_t * self, int * retBytes);
long CcsBuffer_StringTo(CcsBuffer_t * self, size_t * len, const char * needle);
const char * CcsBuffer_GetString(CcsBuffer_t * self, long start, size_t size);
void CcsBuffer_Consume(CcsBuffer_t * self, long start, size_t size);
void CcsBuffer_SetBusy(CcsBuffer_t * self, long startBusy);
void CcsBuffer_ClearBusy(CcsBuffer_t * self);
void CcsBuffer_Lock(CcsBuffer_t * self);
void CcsBuffer_LockReset(CcsBuffer_t * self);
void CcsBuffer_Unlock(CcsBuffer_t * self);

EXTC_END

#endif /* COCO_BUFFER_H */
