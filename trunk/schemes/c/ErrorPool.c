/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#include  <stdarg.h>
#include  "ErrorPool.h"

CcsErrorPool_t *
CcsErrorPool(CcsErrorPool_t * self, FILE * fp)
{
    self->fp = fp;
    self->warningCount = 0;
    self->errorCount = 0;
    return self;
}

void
CcsErrorPool_Destruct(CcsErrorPool_t * self)
{
}

void
CcsErrorPool_Info(CcsErrorPool_t * self, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    va_end(ap);
}

void
CcsErrorPool_Warning(CcsErrorPool_t * self, const CcsLocation_t * loc,
		     const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VWarning(self, loc, format, ap);
    va_end(ap);

}
void
CcsErrorPool_VWarning(CcsErrorPool_t * self, const CcsLocation_t * loc,
		      const char * format, va_list ap)
{
    if (!loc)  fprintf(self->fp, "Warning: ");
    else fprintf(self->fp, "%s(%d,%d) warning: ",
		 loc->fname, loc->line, loc->col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    ++self->warningCount;
}

void
CcsErrorPool_Error(CcsErrorPool_t * self, const CcsLocation_t * loc,
		   const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(self, loc, format, ap);
    va_end(ap);
}

void
CcsErrorPool_VError(CcsErrorPool_t * self, const CcsLocation_t * loc,
		    const char * format, va_list ap)
{
    if (!loc) fprintf(self->fp, "Error: ");
    else fprintf(self->fp, "%s(%d,%d) error: ",
		 loc->fname, loc->line, loc->col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    ++self->errorCount;
}

void
CcsErrorPool_Fatal(CcsErrorPool_t * self, const CcsLocation_t * loc,
		   const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VFatal(self, loc, format, ap);
    va_end(ap);
}

void
CcsErrorPool_VFatal(CcsErrorPool_t * self, const CcsLocation_t * loc,
		    const char * format, va_list ap)
{
    if (!loc) fprintf(self->fp, "Fatal: ");
    else fprintf(self->fp, "%s(%d,%d) fatal: ",
		 loc->fname, loc->line, loc->col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    exit(-1);
}
