/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_LEXICAL_COMMENT_H
#define  COCO_LEXICAL_COMMENT_H

#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif

EXTC_BEGIN

struct CcComment_s {
    int           start[3];
    int           stop[3];
    CcsBool_t     nested;
    CcComment_t * next;
};

CcComment_t *
CcComment(const int * start, const int * stop, CcsBool_t nested);
void CcComment_Destruct(CcComment_t * self);

EXTC_END

#endif  /* COCO_LEXICAL_COMMENT_H */
