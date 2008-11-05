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

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
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
CcsErrorPool_Warning(CcsErrorPool_t * self, int line, int col,
		     const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VWarning(self, line, col, format, ap);
    va_end(ap);

}
void
CcsErrorPool_VWarning(CcsErrorPool_t * self, int line, int col,
		      const char * format, va_list ap)
{
    fprintf(self->fp, "Warning(%d,%d): ", line, col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    ++self->warningCount;
}

void
CcsErrorPool_Error(CcsErrorPool_t * self, int line, int col,
		   const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VError(self, line, col, format, ap);
    va_end(ap);
}

void
CcsErrorPool_VError(CcsErrorPool_t * self, int line, int col,
		    const char * format, va_list ap)
{
    fprintf(self->fp, "Error(%d,%d): ", line, col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    ++self->errorCount;
}

void
CcsErrorPool_Fatal(CcsErrorPool_t * self, int line, int col,
		   const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    CcsErrorPool_VFatal(self, line, col, format, ap);
    va_end(ap);
}

void
CcsErrorPool_VFatal(CcsErrorPool_t * self, int line, int col,
		    const char * format, va_list ap)
{
    fprintf(self->fp, "Fatal Error(%d,%d): ", line, col);
    vfprintf(self->fp, format, ap);
    fprintf(self->fp, "\n");
    exit(-1);
}
