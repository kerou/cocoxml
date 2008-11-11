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
#include  "Object.h"

CcObject_t *
CcObject(const CcObjectType_t * type, int index, va_list ap)
{
    CcObject_t * self = CcMalloc(type->size);
    memset(self, 0, type->size);
    self->type = type;
    self->index = index;
    if (type->construct) type->construct(self, ap);
    return self;
}

void
CcObject_Destruct(CcObject_t * self)
{
    CcFree(self);
}

void
CcObject_VDestruct(CcObject_t * self)
{
    self->type->destruct(self);
}
