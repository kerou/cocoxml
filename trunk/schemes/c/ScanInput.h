/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef COCO_SCANINPUT_H
#define COCO_SCANINPUT_H

#ifndef  COCO_TOKEN_H
#include "c/Token.h"
#endif

#ifndef  COCO_BUFFER_H
#include "c/Buffer.h"
#endif

#ifndef  COCO_POSITION_H
#include "c/Position.h"
#endif

EXTC_BEGIN

typedef struct {
    int additionalSpace;
    int eofSym;
    int maxT;
    int noSym;
    CcsBool_t (* additionalInit)(void * additional, void * scanner);
    void (* additionalDestruct)(void * additional);
    CcsToken_t * (* skip)(void * scanner, CcsScanInput_t * input);
    int (* kind)(void * scanner, CcsScanInput_t * input);
}   CcsSI_Info_t;

struct CcsScanInput_s {
    CcsScanInput_t      * next;

    void               * scanner;
    const CcsSI_Info_t * info;
    char               * fname;
    FILE               * fp;
    CcsBuffer_t          buffer;

    /* The list of tokens returned to Parser. */
    CcsToken_t         * busyFirst;
    CcsToken_t         * busyLast;
    /* The list of tokens which are prepared to be returned. */
    CcsToken_t         * readyFirst;
    CcsToken_t         * readyPeek;
    CcsToken_t         * readyLast;

    /* Current status: Basic set. */
    int                  ch;
    int                  chBytes;
    int                  pos;
    int                  line;
    int                  col;
    /* Current status: supplement set. */
    CcsBool_t            inComment;
    int                  numCommentEols;
    int                  chAfterComment;
    int                  chLastNonblank;
};

CcsScanInput_t *
CcsScanInput(void * scanner, const CcsSI_Info_t * info, FILE * fp);
CcsScanInput_t *
CcsScanInput_ByName(void * scanner, const CcsSI_Info_t * info,
		    const CcsIncPathList_t * list, const char * includer,
		    const char * infn);

void CcsScanInput_GetCh(CcsScanInput_t * self);
CcsToken_t * CcsScanInput_NewToken(CcsScanInput_t * self, int kind);

/* Detach from Scanner */
void CcsScanInput_Detach(CcsScanInput_t * self);

/* Return the first token in readyList and put it into busyList.
 * CcsScanInput_TokenDecRef has to be called to release it from busyList. */
CcsToken_t * CcsScanInput_Scan(CcsScanInput_t * self);
/* WithDraw the last returned token, cause it to be removed from busyList to
 * readyList again. refcnt of token must be 2. */
void CcsScanInput_WithDraw(CcsScanInput_t * self, CcsToken_t * token);
/* Prepend a new generated token at the beginning of readyList. */
CcsBool_t CcsScanInput_Prepend(CcsScanInput_t * self, int kind,
			       const char * val, size_t vallen);
/* Increase the refcnt of the provided token which must be located in busyList.
 * These is used to get original text in usual. */
void CcsScanInput_TokenIncRef(CcsScanInput_t * self, CcsToken_t * token);
/* Decrease the refcnt of the provided token which must be located in busyList.
 * All tokens returned by CcsScanInput_Scan must be released by
 * CcsScanInput_TokenDecRef. */
void CcsScanInput_TokenDecRef(CcsScanInput_t * self, CcsToken_t * token);

/* StringToV is used to get the string from current position to the provided
 * needle, if the provided needle is not encountered, return when maximum
 * length is reached or the end of file is reached.
 * The start position is returned with its length.
 *
 * The available needles are is provided by needle array.
 * The maximum length is set to *len, and the returned string length is
 * saved to *len too. */
long
CcsScanInput_StringTo(CcsScanInput_t * self, size_t * len, const char * needle);
/* Return the requested string from the return value of StringToV.
 * The returned string is not terminated by '\0'. */
const char *
CcsScanInput_GetString(CcsScanInput_t * self, long start, size_t len);
/* Consume the StringTo returned string with the provided length, the characters
 * in this scope is not be parsed by Scanner anymore. The line number and column
 * number is calculated automatically in Consume too. */
void CcsScanInput_Consume(CcsScanInput_t * self, long start, size_t len);

/* Get original text between the provided tokens. Both of these tokens
 * must be located in busyList. */
CcsPosition_t *
CcsScanInput_GetPosition(CcsScanInput_t * self, const CcsToken_t * begin,
			 const CcsToken_t * end);
CcsPosition_t *
CcsScanInput_GetPositionBetween(CcsScanInput_t * self,
				const CcsToken_t * begin,
				const CcsToken_t * end);

/* Go forward in readyList. Do not use CcsScanInput_TokenIncRef or
 * CcsScanInput_TokenDecRef on the token returned by CcsScanInput_Peek,
 * just use CcsScanInput_ResetPeek. */
CcsToken_t * CcsScanInput_Peek(CcsScanInput_t * self);
void CcsScanInput_ResetPeek(CcsScanInput_t * self);

typedef struct {
    int start[2];
    int end[2];
    CcsBool_t nested;
}  CcsComment_t;
CcsBool_t CcsScanInput_Comment(CcsScanInput_t * self, const CcsComment_t * c);

EXTC_END

#endif  /* COCO_SCANINPUT_H */
