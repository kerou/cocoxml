/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  <ctype.h>
#include  "Lexical.h"
#include  "ArrayList.h"
#include  "BitArray.h"
#include  "Globals.h"
#include  "lexical/Action.h"
#include  "lexical/CharSet.h"
#include  "lexical/CharClass.h"
#include  "lexical/Comment.h"
#include  "lexical/Melted.h"
#include  "lexical/Nodes.h"
#include  "lexical/State.h"
#include  "lexical/Target.h"
#include  "c/ErrorPool.h"
#include  "c/Token.h"

const char * IndentInName = "IndentIn";
const char * IndentOutName = "IndentOut";
const char * IndentErrName = "IndentErr";

/* SZ_ADDED_TERMINALS and SZ_LITERALS are a prime numbers,
 * auto-extending is not supported now */
#define  SZ_ADDED_TERMINALS 127
#define  SZ_LITERALS        127

static void
CcLexical_GetTargetStates(CcLexical_t * self, CcAction_t * a,
			  CcBitArray_t * targets, CcSymbol_t ** endOf,
			  CcsBool_t * ctx);
static CcMelted_t *
CcLexical_NewMelted(CcLexical_t * self, CcBitArray_t * set, CcState_t * state);
static CcBitArray_t * CcLexical_MeltedSet(CcLexical_t * self, int nr);
static CcMelted_t *
CcLexical_StateWithSet(CcLexical_t * self, const CcBitArray_t * s);
static void CcLexical_ClearMelted(CcLexical_t * self);

CcLexical_t *
CcLexical(CcLexical_t * self, CcGlobals_t * globals)
{
    self = (CcLexical_t *)CcEBNF(&self->base);
    self->globals = globals;

    self->ignored = CcCharSet();
    self->ignoreCase = FALSE;
    self->indentUsed = FALSE;
    self->spaceUsed = FALSE;
    self->backslashNewline = FALSE;
    CcHashTable(&self->addedTerminals, SZ_ADDED_TERMINALS);
    CcArrayList(&self->states);
    CcArrayList(&self->classes);
    CcHashTable(&self->literals, SZ_LITERALS);
    self->firstMelted = NULL;
    self->firstComment = NULL;

    self->lastSimState = 0;
    self->curSy = NULL;
    self->dirtyLexical = FALSE;
    self->hasCtxMoves = FALSE;

    CcArrayList_New(&self->states, (CcObject_t *)CcState());
    return self;
}

void
CcLexical_Destruct(CcLexical_t * self)
{
    CcComment_t * cur, * next;

    for (cur = self->firstComment; cur; cur = next) {
	next = cur->next;
	CcComment_Destruct(cur);
    }
    CcHashTable_Destruct(&self->literals);
    CcArrayList_Destruct(&self->classes);
    CcArrayList_Destruct(&self->states);
    CcHashTable_Destruct(&self->addedTerminals);
    CcCharSet_Destruct(self->ignored);
    CcEBNF_Destruct(&self->base);
}

void
CcLexical_SetOption(CcLexical_t * self, const CcsToken_t * t, CcsBool_t isIdent)
{
    const char * value;
    CcSymbolTable_t * symtab = &self->globals->symtab;

    value = isIdent ? t->val : CcUnescape(t->val);
    if (!strcasecmp(value, "ignore case")) {
	self->ignoreCase = TRUE;
    } else if (!strcasecmp(value, "indentation")) {
	self->indentUsed = TRUE;
	CcSymbolTable_NewTerminal(symtab, IndentInName, t->loc.line);
	CcSymbolTable_NewTerminal(symtab, IndentOutName, t->loc.line);
	CcSymbolTable_NewTerminal(symtab, IndentErrName, t->loc.line);
    } else if (!strcasecmp(value, "space")) {
	self->spaceUsed = TRUE;
    } else if (!strcasecmp(value, "backslash newline")) {
	self->backslashNewline = TRUE;
    } else {
	CcsErrorPool_Error(self->globals->errpool, &t->loc,
			   "Unsupported option '%s' encountered.", value);
    }
    if (!isIdent) CcFree((char *)value);
}

void CcLexical_AddTerminal(CcLexical_t * self, const CcsToken_t * t)
{
    CcSymbolTable_t * symtab = &self->globals->symtab;
    CcSymbol_t * sym = CcSymbolTable_NewTerminal(symtab, t->val, t->loc.line);
    if (!CcHashTable_Set(&self->addedTerminals, t->val, (CcObject_t *)sym))
	CcsErrorPool_Error(self->globals->errpool, &t->loc,
			   "Too many additional terminals, recompile Coco with enlarged additional terminal hash table.");
}

CcGraph_t *
CcLexical_StrToGraph(CcLexical_t * self, const char * str, const CcsToken_t * t)
{
    CcGraph_t * g; CcTransition_t trans;
    const char * cur, * slast;
    if (strlen(str) == 0)
	CcsErrorPool_Error(self->globals->errpool, &t->loc,
			   "empty token not allowed");
    g = CcGraph();
    cur = str; slast = str + strlen(str);
    while (cur < slast) {
	CcTransition(&trans, CcsUTF8GetCh(&cur, slast),
		     trans_normal, &self->classes);
	CcGraph_Append(g, CcEBNF_NewNode(&self->base, CcNodeTrans(0, &trans)));
	CcTransition_Destruct(&trans);
    }
    return g;
}

void
CcLexical_SetContextTrans(CcLexical_t * self, CcNode_t * p)
{
    while (p != NULL) {
	if (p->base.type == node_trans) {
	    CcTransition_SetCode(&((CcNodeTrans_t *)p)->trans, trans_context);
	} else if (p->base.type == node_opt || p->base.type == node_iter) {
	    CcLexical_SetContextTrans(self, p->sub);
	} else if (p->base.type == node_alt) {
	    CcLexical_SetContextTrans(self, p->sub);
	    CcLexical_SetContextTrans(self, p->down);
	}
	if (p->up) break;
	p = p->next;
    }
}

CcCharClass_t *
CcLexical_NewCharClass(CcLexical_t * self, const char * name, CcCharSet_t * s)
{
    return (CcCharClass_t *)
	CcArrayList_New(&self->classes, (CcObject_t *)CcCharClass(name, s));
}

CcCharClass_t *
CcLexical_FindCharClassN(CcLexical_t * self, const char * name)
{
    CcCharClass_t * c; CcArrayListIter_t iter;
    for (c = (CcCharClass_t *)CcArrayList_First(&self->classes, &iter);
	 c; c = (CcCharClass_t *)CcArrayList_Next(&self->classes, &iter))
	if (!strcmp(name, c->name)) return c;
    return NULL;
}

CcCharSet_t *
CcLexical_CharClassSet(CcLexical_t * self, int index)
{
    return ((CcCharClass_t *)CcArrayList_Get(&self->classes, index))->set;
}

static void
CcLexical_NewTransition(CcLexical_t * self, CcState_t * from, CcState_t * to,
			const CcTransition_t * trans)
{
    CcTarget_t * t; CcAction_t * a;

    if (to == (CcState_t *)CcArrayList_Get(&self->states, 0))
	CcsErrorPool_Error(self->globals->errpool, NULL,
			   "token must not start with an iteration");
    t = CcTarget(to);
    a = CcAction(trans); a->target = t;
    CcState_AddAction(from, a);
    if (CcTransition_Size(trans) > 1)
	CcSymbol_SetTokenKind(self->curSy, symbol_classToken);
}

static void
CcLexical_CombineShifts(CcLexical_t * self)
{
    CcState_t * state; CcArrayListIter_t iter;
    CcAction_t * a, * b, * c;
    CcCharSet_t * seta, * setb;

    for (state = (CcState_t *)CcArrayList_First(&self->states, &iter);
	 state; state = (CcState_t *)CcArrayList_Next(&self->states, &iter)) {
	for (a = state->firstAction; a != NULL; a = a->next) {
	    b = a->next;
	    while (b != NULL)
		if (a->target->state == b->target->state &&
		    a->trans.code == b->trans.code) {
		    seta = CcAction_GetShift(a);
		    setb = CcAction_GetShift(b);
		    CcCharSet_Or(seta, setb);
		    CcAction_SetShift(a, seta);
		    c = b; b = b->next; CcState_DetachAction(state, c);
		    CcAction_Destruct(c);
		    CcCharSet_Destruct(seta);
		    CcCharSet_Destruct(setb);
		} else {
		    b = b->next;
		}
	}
    }
}

static void
CcLexical_FindUsedStates(CcLexical_t * self, CcState_t * state,
			 CcBitArray_t * used)
{
    CcAction_t * a;

    if (CcBitArray_Get(used, state->base.index)) return;
    CcBitArray_Set(used, state->base.index, TRUE);
    for (a = state->firstAction; a != NULL; a = a->next)
	CcLexical_FindUsedStates(self, a->target->state, used);
}

static CcObject_t *
state_filter(CcObject_t * object, int curidx, int newidx, void * data)
{
    if (CcBitArray_Get((CcBitArray_t *)data, curidx)) return object;
    CcObject_VDestruct(object);
    return NULL;
}

static void
CcLexical_DeleteRedundantStates(CcLexical_t * self)
{
    CcArrayListIter_t iter, iter0;
    CcState_t * s1, * s2, * state;
    CcBitArray_t used;
    CcAction_t * a;
    CcState_t ** newState = (CcState_t **)
	CcMalloc(sizeof(CcState_t *) * (self->states.Count));

    CcBitArray(&used, self->states.Count);

    s1 = (CcState_t *)CcArrayList_First(&self->states, &iter);
    CcLexical_FindUsedStates(self, s1, &used);
    while (s1) {
	if (CcBitArray_Get(&used, s1->base.index) && s1->endOf != NULL &&
	    s1->firstAction == NULL && !(s1->ctx)) {
	    CcArrayListIter_Copy(&iter0, &iter);
	    for (s2 = (CcState_t *)CcArrayList_Next(&self->states, &iter0);
		 s2; s2 = (CcState_t *)CcArrayList_Next(&self->states, &iter0)) {
		if (CcBitArray_Get(&used, s2->base.index) &&
		    s1->endOf == s2->endOf &&
		    s2->firstAction == NULL && !(s2->ctx)) {
		    CcBitArray_Set(&used, s2->base.index, FALSE);
		    newState[s2->base.index] = s1;
		}
	    }
	}
	s1 = (CcState_t *)CcArrayList_Next(&self->states, &iter);
    }

    for (state = (CcState_t *)CcArrayList_First(&self->states, &iter);
	 state; state = (CcState_t *)CcArrayList_Next(&self->states, &iter))
	if (CcBitArray_Get(&used, state->base.index))
	    for (a = state->firstAction; a != NULL; a = a->next)
		if (!CcBitArray_Get(&used, a->target->state->base.index))
		    a->target->state = newState[a->target->state->base.index];

    /* delete unused states */
    CcArrayList_Filter(&self->states, state_filter, &used);
    CcFree(newState);
    CcBitArray_Destruct(&used);
}

static CcState_t *
CcLexical_TheState(CcLexical_t * self, CcNode_t * p)
{
    CcState_t * state;

    if (p != NULL)  return p->state;
    state = (CcState_t *)CcArrayList_New(&self->states,
					 (CcObject_t *)CcState());
    state->endOf = self->curSy;
    return state;
}

static void
CcLexical_Step(CcLexical_t * self, CcState_t * from, CcNode_t * p,
	       CcBitArray_t * stepped)
{
    CcBitArray_t newStepped;

    if (p == NULL) return;
    CcBitArray_Set(stepped, p->base.index, TRUE);
    if (p->base.type == node_trans) {
	CcLexical_NewTransition(self, from, CcLexical_TheState(self, p->next),
				&((CcNodeTrans_t *)p)->trans);
    } else if (p->base.type == node_alt) {
	CcLexical_Step(self, from, p->sub, stepped);
	CcLexical_Step(self, from, p->down, stepped);
    } else if (p->base.type == node_iter || p->base.type == node_opt) {
	if (p->next != NULL && !CcBitArray_Get(stepped, p->next->base.index))
	    CcLexical_Step(self, from, p->next, stepped);
	CcLexical_Step(self, from, p->sub, stepped);
	if ((p->base.type == node_iter) && (p->state != from)) {
	    CcBitArray(&newStepped, self->base.nodes.Count);
	    CcLexical_Step(self, p->state, p, &newStepped);
	    CcBitArray_Destruct(&newStepped);
	}
    }
}

static void
CcLexical_NumberNodes(CcLexical_t * self, CcNode_t * p,
		      CcState_t * state, CcsBool_t renumIter)
{
    if (p == NULL) return;
    if (p->state != NULL) return; /* already visited */
    if ((state == NULL) || (p->base.type == node_iter && renumIter))
	state = (CcState_t *)CcArrayList_New(&self->states,
					     (CcObject_t *)CcState());
    p->state = state;
    if (CcNode_DelGraph(p))
	state->endOf = self->curSy;

    if (p->base.type == node_trans) {
	CcLexical_NumberNodes(self, p->next, NULL, FALSE);
    } else if (p->base.type == node_opt) {
	CcLexical_NumberNodes(self, p->next, NULL, FALSE);
	CcLexical_NumberNodes(self, p->sub, state, TRUE);
    } else if (p->base.type == node_iter) {
	CcLexical_NumberNodes(self, p->next, state, TRUE);
	CcLexical_NumberNodes(self, p->sub, state, TRUE);
    } else if (p->base.type == node_alt) {
	CcLexical_NumberNodes(self, p->next, NULL, FALSE);
	CcLexical_NumberNodes(self, p->sub, state, TRUE);
	CcLexical_NumberNodes(self, p->down, state, renumIter);
    }
}

static void
CcLexical_FindTrans(CcLexical_t * self, CcNode_t * p, CcsBool_t start, CcBitArray_t * marked)
{
    CcBitArray_t stepped;

    if (p == NULL || CcBitArray_Get(marked, p->base.index)) return;
    CcBitArray_Set(marked, p->base.index, TRUE);
    if (start) {
	CcBitArray(&stepped, self->base.nodes.Count);
	/* start of group of equally numbered nodes */
	CcLexical_Step(self, p->state, p, &stepped);
	CcBitArray_Destruct(&stepped);
    }

    if (p->base.type == node_trans) {
	CcLexical_FindTrans(self, p->next, TRUE, marked);
    } else if (p->base.type == node_opt) {
	CcLexical_FindTrans(self, p->next, TRUE, marked);
	CcLexical_FindTrans(self, p->sub, FALSE, marked);
    } else if (p->base.type == node_iter) {
	CcLexical_FindTrans(self, p->next, FALSE, marked);
	CcLexical_FindTrans(self, p->sub, FALSE, marked);
    } else if (p->base.type == node_alt) {
	CcLexical_FindTrans(self, p->sub, FALSE, marked);
	CcLexical_FindTrans(self, p->down, FALSE, marked);
    }
}

void
CcLexical_ConvertToStates(CcLexical_t * self, CcNode_t * p, CcSymbol_t * sym)
{
    CcBitArray_t marked;
    CcState_t * firstState;

    CcsAssert(sym->base.type == symbol_t || sym->base.type == symbol_pr);
    self->curSy = sym;
    if (CcNode_DelGraph(p))
	CcsErrorPool_Error(self->globals->errpool, NULL,
			   "token '%s' might be empty", sym->name);
    firstState = (CcState_t *)CcArrayList_Get(&self->states, 0);
    CcLexical_NumberNodes(self, p, firstState, TRUE);

    CcBitArray(&marked, self->base.nodes.Count);
    CcLexical_FindTrans(self, p, TRUE, &marked);

    if (p->base.type == node_iter) {
	CcBitArray_SetAll(&marked, FALSE);
	firstState = (CcState_t *)CcArrayList_Get(&self->states, 0);
	CcLexical_Step(self, firstState, p, &marked);
    }
    CcBitArray_Destruct(&marked);
}

/* match string against current automaton; store it either as a
 * fixedToken or as a litToken */
void
CcLexical_MatchLiteral(CcLexical_t * self, const CcsToken_t * t,
		       const char * s, CcSymbol_t * sym)
{
    const char * scur, * snext, * slast;
    CcState_t * to, * state;
    CcAction_t * a;
    CcTransition_t trans;
    CcSymbol_t * matchedSym;
    CcState_t * firstState = (CcState_t *)CcArrayList_Get(&self->states, 0);

    state = firstState; a = NULL;
    CcsAssert(sym->base.type == symbol_t || sym->base.type == symbol_pr);
    /* Try to match s against existing CcLexical. */
    scur = s; slast = scur + strlen(s);
    for (scur = s; scur < slast; scur = snext) {
	snext = scur;
	a = CcState_FindAction(state, CcsUTF8GetCh(&snext, slast));
	if (a == NULL) break;
	state = a->target->state;
    }

    /* if s was not totally consumed or leads to a non-final state => make
     * new CcLexical from it */
    if (*scur || state->endOf == NULL) {
	state = firstState; scur = s; a = NULL;
	self->dirtyLexical = TRUE;
    }
    while (scur < slast) { /* make new CcLexical for s0[i..len-1] */
	to = (CcState_t *)CcArrayList_New(&self->states,
					  (CcObject_t *)CcState());
	CcTransition(&trans, CcsUTF8GetCh(&scur, slast),
		     trans_normal, &self->classes);
	CcLexical_NewTransition(self, state, to, &trans);
	CcTransition_Destruct(&trans);
	state = to;
    }

    matchedSym = state->endOf;
    if (state->endOf == NULL) {
	state->endOf = sym;
    } else if (CcSymbol_GetTokenKind(matchedSym) == symbol_fixedToken ||
	       (a != NULL && a->trans.code == trans_context)) {
	/* s matched a token with a fixed definition or a token with
	 * an appendix that will be cut off */
	CcsErrorPool_Error(self->globals->errpool, NULL,
			   "tokens '%s' and '%s' cannot be distinguished",
			   sym->name, matchedSym->name);
    } else { /* matchedSym == classToken || classLitToken */
	CcSymbol_SetTokenKind(matchedSym, symbol_classLitToken);
	CcSymbol_SetTokenKind(sym, symbol_litToken);
    }
}

static void
CcLexical_MeltStates(CcLexical_t * self, CcState_t * state)
{
    CcsBool_t ctx;
    CcBitArray_t targets;
    CcSymbol_t * endOf;
    CcAction_t * action;
    CcMelted_t * melt;
    CcState_t * s;
    CcTarget_t * targ;

    for (action = state->firstAction; action != NULL; action = action->next) {
	if (action->target->next != NULL) {
	    CcLexical_GetTargetStates(self, action, &targets, &endOf, &ctx);
	    melt = CcLexical_StateWithSet(self, &targets);
	    if (melt == NULL) {
		s = (CcState_t *)CcArrayList_New(&self->states,
						 (CcObject_t *)CcState());
		s->endOf = endOf; s->ctx = ctx;
		for (targ = action->target; targ != NULL; targ = targ->next)
		    CcState_MeltWith(s, targ->state);
		CcState_MakeUnique(s);
		melt = CcLexical_NewMelted(self, &targets, s);
	    }
	    CcTarget_ListDestruct(action->target->next);
	    action->target->next = NULL;
	    action->target->state = melt->state;
	    CcBitArray_Destruct(&targets);
	}
    }
}

static void
CcLexical_FindCtxStates(CcLexical_t * self)
{
    CcState_t * state; CcArrayListIter_t iter;
    CcAction_t * action;

    for (state = (CcState_t *)CcArrayList_First(&self->states, &iter);
	 state; state = (CcState_t *)CcArrayList_Next(&self->states, &iter))
	for (action = state->firstAction; action; action = action->next)
	    if (action->trans.code == trans_context)
		action->target->state->ctx = TRUE;
}

void
CcLexical_MakeDeterministic(CcLexical_t * self)
{
    CcState_t * state; CcArrayListIter_t iter;

    self->lastSimState = self->states.Count;
    /* heuristic for set size in CcMelted.set */
    self->maxStates = self->lastSimState + self->lastSimState;
    CcLexical_FindCtxStates(self);

    for (state = (CcState_t *)CcArrayList_First(&self->states, &iter);
	 state; state = (CcState_t *)CcArrayList_Next(&self->states, &iter))
	CcState_MakeUnique(state);
    for (state = (CcState_t *)CcArrayList_First(&self->states, &iter);
	 state; state = (CcState_t *)CcArrayList_Next(&self->states, &iter))
	CcLexical_MeltStates(self, state);
    CcLexical_DeleteRedundantStates(self);
    CcLexical_CombineShifts(self);
    CcLexical_ClearMelted(self);
}

/* ------------------------- melted states ------------------------------ */
static CcMelted_t *
CcLexical_NewMelted(CcLexical_t * self, CcBitArray_t * set, CcState_t * state)
{
    CcMelted_t * m = CcMelted(set, state);
    m->next = self->firstMelted; self->firstMelted = m;
    return m;
}

static CcBitArray_t *
CcLexical_MeltedSet(CcLexical_t * self, int nr)
{
    CcMelted_t * m = self->firstMelted;
    while (m != NULL) {
	if (m->state->base.index == nr) return &m->set;
	else m = m->next;
    }
    /*Errors::Exception("-- compiler error in CcMelted::Set");*/
    /*throw new Exception("-- compiler error in CcMelted::Set");*/
    return NULL;
}

CcMelted_t *
CcLexical_StateWithSet(CcLexical_t * self, const CcBitArray_t * s)
{
    CcMelted_t * m;
    for (m = self->firstMelted; m != NULL; m = m->next)
	if (CcBitArray_Equal(s, &m->set)) return m;
    return NULL;
}

static void
CcLexical_ClearMelted(CcLexical_t * self)
{
    CcMelted_t * cur, * next;
    for (cur = self->firstMelted; cur; cur = next) {
	next = cur->next;
	CcMelted_Destruct(cur);
    }
    self->firstMelted = NULL;
}

/* ---------------------------- actions -------------------------------- */
static void
CcLexical_GetTargetStates(CcLexical_t * self, CcAction_t * a,
			  CcBitArray_t * targets, CcSymbol_t ** endOf,
			  CcsBool_t * ctx)
{
    CcTarget_t * t; int stateNr;
    /* compute the set of target states */
    CcBitArray(targets, self->maxStates);
    *endOf = NULL; *ctx = FALSE;
    for (t = a->target; t != NULL; t = t->next) {
	stateNr = t->state->base.index;
	if (stateNr <= self->lastSimState) {
	    CcBitArray_Set(targets, stateNr, TRUE);
	} else {
	    CcBitArray_Or(targets, CcLexical_MeltedSet(self, stateNr));
	}
	if (t->state->endOf != NULL) {
	    if (*endOf == NULL || *endOf == t->state->endOf)
		*endOf = t->state->endOf;
	    else {
		CcsErrorPool_Error(self->globals->errpool, NULL,
				   "Tokens '%s' and '%s' cannot be distinguished\n",
				   (*endOf)->name,
				   t->state->endOf->name);
	    }
	}
	if (t->state->ctx) {
	    *ctx = TRUE;
	    /* The following check seems to be unnecessary. It reported an error
	     * if a symbol + context was the prefix of another symbol, e.g.
	     *   s1 = "a" "b" "c".
	     *   s2 = "a" CONTEXT("b").
	     * But this is ok.
	     * if (t.state.endOf != null) {
	     *   Console.WriteLine("Ambiguous context clause");
	     *	 Errors.count++;
	     * } */
	}
    }
}

/* ------------------------ comments -------------------------------- */
static void
CcLexical_CommentStr(CcLexical_t * self, const CcsToken_t * token,
		     int * output, CcNode_t * p)
{
    int * cur = output;
    CcTransition_t * trans;

    *cur = 0;
    while (p != NULL) {
	if (p->base.type == node_trans) {
	    trans = &((CcNodeTrans_t *)p)->trans;
	    if (CcTransition_Size(trans) != 1)
		CcsErrorPool_Error(self->globals->errpool, &token->loc,
				   "character set contains more than 1 character");
	    *cur++ = CcTransition_First(trans);
	} else {
	    CcsErrorPool_Error(self->globals->errpool, &token->loc,
			       "comment delimiters may not be structured");
	}
	if (cur - output > 2) {
	    CcsErrorPool_Error(self->globals->errpool, &token->loc,
			       "comment delimiters must be 1 or 2 characters long");
	    cur = output; *cur++ = '?';
	    break;
	}
	p = p->next;
    }
    *cur = 0;
}

void
CcLexical_NewComment(CcLexical_t * self, const CcsToken_t * token,
		     CcNode_t * from, CcNode_t * to, CcsBool_t nested)
{
    int start[3], stop[3];
    CcComment_t * c;

    CcLexical_CommentStr(self, token, start, from);
    CcLexical_CommentStr(self, token, stop, to);
    if (nested && start[0] == stop[0]) {
	CcsErrorPool_Error(self->globals->errpool, &token->loc,
			   "The first char of start and stop of nested comment is same.");
	return;
    }
    c = CcComment(start, stop, nested);
    c->next = self->firstComment; self->firstComment = c;
}

CcsBool_t
CcLexical_Finish(CcLexical_t * self)
{
    if (!self->spaceUsed) CcCharSet_Set(self->ignored, ' ');
    if (self->dirtyLexical) CcLexical_MakeDeterministic(self);
    return TRUE;
}

static int
stCmp(const void * st0, const void * st1)
{
    const CcLexical_StartTab_t * ccst0 = (const CcLexical_StartTab_t *)st0;
    const CcLexical_StartTab_t * ccst1 = (const CcLexical_StartTab_t *)st1;
    CcsAssert(ccst0->keyFrom > ccst1->keyTo || ccst0->keyTo < ccst1->keyFrom);
    return ccst0->keyFrom - ccst1->keyFrom;
}

CcLexical_StartTab_t *
CcLexical_GetStartTab(const CcLexical_t * self, int * retNumEle)
{
    CcLexical_StartTab_t * table, * cur;
    const CcAction_t * action;
    CcCharSet_t * s; CcRange_t * curRange;
    int targetStateIndex;
    const CcState_t * firstState =
	(const CcState_t *)CcArrayList_GetC(&self->states, 0);

    *retNumEle = 0;
    for (action = firstState->firstAction; action; action = action->next) {
	s = CcTransition_GetCharSet(&action->trans);
	*retNumEle += CcCharSet_NumRange(s);
	CcCharSet_Destruct(s);
    }
    table = cur = CcMalloc(sizeof(CcLexical_StartTab_t) * (*retNumEle));
    for (action = firstState->firstAction; action; action = action->next) {
	s = CcTransition_GetCharSet(&action->trans);
	targetStateIndex = action->target->state->base.index;
	for (curRange = s->head; curRange; curRange = curRange->next) {
	    cur->keyFrom = curRange->from;
	    cur->keyTo = curRange->to;
	    cur->state = targetStateIndex;
	    ++cur;
	}
	CcCharSet_Destruct(s);
    }
    qsort(table, *retNumEle, sizeof(CcLexical_StartTab_t), stCmp);
    return table;
}

CcsBool_t
CcLexical_KeywordUsed(const CcLexical_t * self)
{
    const CcSymbolT_t * sym;
    CcArrayListIter_t iter;
    const CcArrayList_t * terminals = &self->globals->symtab.terminals;
    for (sym = (const CcSymbolT_t *)CcArrayList_FirstC(terminals, &iter);
	 sym; sym = (const CcSymbolT_t *)CcArrayList_NextC(terminals, &iter))
	if (sym->tokenKind == symbol_litToken) return TRUE;
    return FALSE;
}

int
CcLexical_GetMaxKeywordLength(const CcLexical_t * self)
{
    int maxlen, symlen;
    const CcSymbolT_t * sym;
    CcArrayListIter_t iter;
    const CcArrayList_t * terminals = &self->globals->symtab.terminals;
    maxlen = 0;
    for (sym = (const CcSymbolT_t *)CcArrayList_FirstC(terminals, &iter);
	 sym; sym = (const CcSymbolT_t *)CcArrayList_NextC(terminals, &iter)) {
	if (sym->tokenKind != symbol_litToken) continue;
	symlen = strlen(sym->base.name);
	if (maxlen < symlen) maxlen = symlen;
    }
    return maxlen;
}

static int
idCmp(const void * i0, const void * i1)
{
    const CcLexical_Identifier_t * cci0 = (const CcLexical_Identifier_t *)i0;
    const CcLexical_Identifier_t * cci1 = (const CcLexical_Identifier_t *)i1;
    return strcmp(cci0->name, cci1->name);
}

CcLexical_Identifier_t *
CcLexical_GetIdentifiers(const CcLexical_t * self, int * retNumEle)
{
    const CcSymbolT_t * sym;
    CcArrayListIter_t iter;
    CcLexical_Identifier_t * list, * cur; char * curn;
    const CcArrayList_t * terminals = &self->globals->symtab.terminals;

    *retNumEle = 0;
    for (sym = (const CcSymbolT_t *)CcArrayList_FirstC(terminals, &iter);
	 sym; sym = (const CcSymbolT_t *)CcArrayList_NextC(terminals, &iter))
	if (sym->tokenKind == symbol_litToken) ++*retNumEle;
    list = cur = CcMalloc(sizeof(CcLexical_Identifier_t) * (*retNumEle));
    for (sym = (const CcSymbolT_t *)CcArrayList_FirstC(terminals, &iter);
	 sym; sym = (const CcSymbolT_t *)CcArrayList_NextC(terminals, &iter)) {
	if (sym->tokenKind != symbol_litToken) continue;
	cur->name = CcEscape(sym->base.name);
	if (self->ignoreCase) {
	    for (curn = cur->name; curn; ++curn) *curn = tolower(*curn);
	}
	cur->index = sym->base.kind;
	++cur;
    }
    qsort(list, *retNumEle, sizeof(CcLexical_Identifier_t), idCmp);
    return list;
}

void
CcLexical_Identifiers_Destruct(CcLexical_Identifier_t * self, int numEle)
{
    CcLexical_Identifier_t * cur;
    for (cur = self; cur - self < numEle; ++cur)
	CcFree(cur->name);
    CcFree(self);
}

void
CcLexical_TargetStates(const CcLexical_t * self, CcBitArray_t * mask)
{
    CcArrayListIter_t iter;
    const CcState_t * state;
    const CcAction_t * action;

    CcBitArray(mask, self->states.Count);
    state = (const CcState_t *)CcArrayList_FirstC(&self->states, &iter);
    for (state = (const CcState_t *)CcArrayList_NextC(&self->states, &iter);
	 state;
	 state = (const CcState_t *)CcArrayList_NextC(&self->states, &iter))
	for (action = state->firstAction; action != NULL; action = action->next)
	    CcBitArray_Set(mask, action->target->state->base.index, TRUE);
}

#ifndef  NDEBUG
static const char *
CharRepr(char * buf, size_t szbuf, int ch)
{
    if (ch == '\\') {
	snprintf(buf, szbuf, "'\\\\'");
    } else if (ch == '\'') {
	snprintf(buf, szbuf, "'\\\''");
    } else if (ch >= 32 && ch <= 126) {
	snprintf(buf, szbuf, "'%c'", (char)ch);
    } else if (ch == '\a') {
	snprintf(buf, szbuf, "'\\a'");
    } else if (ch == '\b') {
	snprintf(buf, szbuf, "'\\b'");
    } else if (ch == '\f') {
	snprintf(buf, szbuf, "'\\f'");
    } else if (ch == '\n') {
	snprintf(buf, szbuf, "'\\n'");
    } else if (ch == '\r') {
	snprintf(buf, szbuf, "'\\r'");
    } else if (ch == '\t') {
	snprintf(buf, szbuf, "'\\t'");
    } else if (ch == '\v') {
	snprintf(buf, szbuf, "'\\v'");
    } else {
	snprintf(buf, szbuf, "%d", ch);
    }
    return buf;
}

void
CcLexical_DumpStates(const CcLexical_t * self)
{
    CcArrayListIter_t iter;
    const CcState_t * state;
    const CcAction_t * action;
    const CcTarget_t * target;
    CcCharSet_t * s;
    const CcRange_t * curRange;
    char buf0[8], buf1[8];
    const CcArrayList_t * states = &self->states;

    fprintf(stderr, "Dump States: %d\n", states->Count);
    for (state = (const CcState_t *)CcArrayList_FirstC(states, &iter);
	 state; state = (const CcState_t *)CcArrayList_NextC(states, &iter)) {
	fprintf(stderr, "State %d: %s, ctx = %s\n", state->base.index,
		state->endOf ? state->endOf->name : "(null)",
		state->ctx ? "TRUE" : "FALSE");
	for (action = state->firstAction; action; action = action->next) {
	    fprintf(stderr, "\t");
	    s = CcAction_GetShift(action);
	    for (curRange = s->head; curRange; curRange = curRange->next) {
		if (curRange->from == curRange->to) {
		    fprintf(stderr, "%s",
			    CharRepr(buf0, sizeof(buf0), curRange->from));
		} else {
		    fprintf(stderr, "[%s, %s]",
			    CharRepr(buf0, sizeof(buf0), curRange->from),
			    CharRepr(buf1, sizeof(buf1), curRange->to));
		}
		if (curRange->next) fprintf(stderr, ",");
	    }
	    CcCharSet_Destruct(s);
	    fprintf(stderr, "\t");
	    for (target = action->target; target; target = target->next) {
		fprintf(stderr, "%d", target->state->base.index);
		if (target->next) fprintf(stderr, ",");
	    }
	    fprintf(stderr, "\n");
	}
    }
    fprintf(stderr, "\n");
}
#endif
