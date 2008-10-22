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
#include  "DFA.h"

State_t *
State(State_t * self)
{
    self->nr = 0;
    self->firstAction = NULL;
    self->endOf = NULL;
    self->ctx = 0;
    self->next = NULL;
}

void
State_Destruct(State_t * self)
{
}

void
State_AddAction(State_t * self, Action_t * act)
{
    Action_t * lasta = NULL, * a = self->firstAction;
    while (a != NULL && act->type >= a->typ) { lasta = a; a = a->next; }
    act->next = a;
    if (a == self->firstAction) self->firstAction = act;
    else  lasta->next = act;
}

void
State_DetachAction(State_t * self, Action_t * act)
{
    Action_t * lasta = NULL, * a = self->firstAction;
    while (a != NULL && a != act) { lasta = a; a = a->next; }
    if (a != NULL) {
	if (a == self->firstAction) self->firstAction = a->next;
	else lasta->next = a->next;
    }
}

int
State_MeltWith(State_t * self, State_t * s)
{
    Action_t * action, * a;
    for (action = s->firstAction; action != NULL; action = action->next) {
	if (!(a = malloc(sizeof(Action_t)))) goto errquit0;
	if (!(Action(a, action->typ, action->sym, action->tc))) goto errquit1;
	Action_AddTargets(a, action);
	Action_AddAction(self, a);
    }
    return 0;
 errquit1:
    free(a);
 errquit0:
    return -1;
}

Action_t *
Action(Action_t * self, int typ, int sym, int tc)
{
    self->typ = type;
    self->sym = sym;
    self->tc = tc;
    self->target = NULL;
    self->next = NULL;
    return self;
}

void
Action_Destruct(Action_t * self)
{
}

void
Action_AddTarget(Action_t * self, Target_t * t)
{
    Target_t * last = NULL;
    Target_t * p = target;
    while (p != NULL && t->state->nr >= p->state->nr) {
	if (t->state == p->state) return;
	last = p; p = p->next;
    }
    t->next = p;
    if (p == target)  target=t;
    else  last->next = t;
}

int
Action_AddTargets(Action_t * self, Action_t * a)
{
    Target_t * p, * t;
    for (Target_t * p = a->target; p != NULL; p = p->next) {
	if (!(t = malloc(sizeof(Target_t)))) goto errquit0;
	if (!Target(t, p->state)) goto errquit1;
	Action_AddTarget(t);
    }
    if (a->tc == Node->contextTrans) tc = Node->contextTrans;
}

CharSet_t * Action_Symbols(Action_t * self, Tab_t * tab)
{
    CharSet_t * s;
    if (typ == Node.clas) {
	s = CharSet_Clone(tab->CharClassSet(sym));
    } else {
	s = CharSet();
	CharSet_Set(s, sym);
    }
    return s;
}

void
Action_ShiftWith(Action_t * self, CharSet_t * s, Tab_t * tab)
{
    CharClass_t * c;
    if (CharSet_Elements() == 1) {
	self->typ = Node->chr; sym = CharSet_First(s);
    } else {
	c = Tab_FindCharClass(tab, s);
	if (c == NULL) c = Tab_NewCharClass("#", s);
	self->typ = Node->clas; sym = c->n;
    }
}

Target_t *
Target(Target_t * self, State_t * s)
{
    self->state = s;
    self->next = NULL;
    return self;
}

void
Target_Destruct(Target_t * self)
{
}
