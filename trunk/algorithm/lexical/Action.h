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
#ifndef  COCO_LEXICAL_ACTION_H
#define  COCO_LEXICAL_ACTION_H

#ifndef  COCO_LEXICAL_TRANSITION_H
#include "lexical/Transition.h"
#endif

EXTC_BEGIN

struct CcAction_s {
    CcAction_t     * next;
    CcTarget_t     * target;
    CcTransition_t   trans;
};

CcAction_t *
CcAction(const CcTransition_t * trans);

CcAction_t * CcAction_Clone(const CcAction_t * action);

void CcAction_Destruct(CcAction_t * self);

int CcAction_ShiftSize(CcAction_t * self);

/* The returned CcCharSet_t must be destructed */
CcCharSet_t * CcAction_GetShift(const CcAction_t * self);

void CcAction_SetShift(CcAction_t * self, const CcCharSet_t * s);

void CcAction_AddTargets(CcAction_t * self, const CcAction_t * action);

CcsBool_t CcAction_Overlap(const CcAction_t * a, const CcAction_t * b);

EXTC_END

#endif /* COCO_LEXICAL_ACTION_H */