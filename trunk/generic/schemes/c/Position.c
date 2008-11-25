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
#include  "Position.h"

CcsPosition_t *
CcsPosition(int beg, int len, int col, const char * text)
{
    CcsPosition_t * self;
    if (!(self = CcsMalloc(sizeof(CcsPosition_t) + len + 1))) return NULL;
    self->beg = beg;
    self->len = len;
    self->col = col;
    self->text = (char *)(self + 1);
    memcpy(self->text, text, len);
    self->text[len] = 0;
    return self;
}

void
CcsPosition_Destruct(CcsPosition_t * self)
{
    CcsFree(self);
}
/*
const char *
CcsPosition_Dump(CcsPosition_t * self, char * buf, size_t szbuf)
{
    if (self == NULL) snprintf(buf, szbuf, "     ");
    else snprintf(buf, szbuf, "%5d", self->beg);
    return buf;
}
*/
