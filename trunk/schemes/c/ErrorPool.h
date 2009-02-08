/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_ERRORPOOL_H
#define  COCO_ERRORPOOL_H

#ifndef   COCO_CDEFS_H
#include  "CDefs.h"
#endif

EXTC_BEGIN

struct CcsLocation_s {
    const char * fname;
    int line;
    int col;
};

struct CcsErrorPool_s {
    FILE * fp;
    int    warningCount;
    int    errorCount;
};

CcsErrorPool_t * CcsErrorPool(CcsErrorPool_t * self, FILE * fp);
void CcsErrorPool_Destruct(CcsErrorPool_t * self);

void CcsErrorPool_Info(CcsErrorPool_t *, const char * format, ...);
/* token == NULL is permitted. */
void CcsErrorPool_Warning(CcsErrorPool_t * self, const CcsLocation_t * token,
			  const char * format, ...);
void CcsErrorPool_VWarning(CcsErrorPool_t * self, const CcsLocation_t * token,
			   const char * format, va_list ap);
void CcsErrorPool_Error(CcsErrorPool_t * self, const CcsLocation_t * token,
			const char * format, ...);
void CcsErrorPool_VError(CcsErrorPool_t * self, const CcsLocation_t * token,
			 const char * format, va_list ap);
void CcsErrorPool_Fatal(CcsErrorPool_t * self, const CcsLocation_t * token,
			const char * format, ...);
void CcsErrorPool_VFatal(CcsErrorPool_t * self, const CcsLocation_t * token,
			 const char * format, va_list ap);
EXTC_END

#endif /* COCO_ERRORPOOL_H */
