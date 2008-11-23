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
#include  "c/COutputScheme.h"
#include  "lexical/Action.h"
#include  "lexical/CharSet.h"
#include  "lexical/Comment.h"
#include  "lexical/State.h"
#include  "lexical/Target.h"
#include  "lexical/Transition.h"

static const CcOutputInfo_t CcCOutputScheme_OutputInfos[] = {
    { "Scanner.c" },
    { "Parser.c" },
    { "Parser.h" },
    { NULL }
};

static CcsBool_t
COS_Defines(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    if (!self->globals->lexical.ignoreCase)
	fprintf(outfp, "%s#define CASE_SENSITIVE\n", indent);
    return TRUE;
}

static CcsBool_t
COS_Declarations(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    fprintf(outfp, "%sself->eofSym = %d;\n", indent,
	    self->globals->syntax.eofSy->base.index);
    fprintf(outfp, "%sself->maxT = %d;\n", indent,
	    self->globals->symtab.terminals.Count - 1);
    fprintf(outfp, "%sself->noSym = %d;\n", indent,
	    self->globals->syntax.noSy->base.index);
    return TRUE;
}

static CcsBool_t
COS_Chars2States(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    int numEle;
    CcLexical_StartTab_t * table, * cur;
    fprintf(outfp, "%s{ EoF, EoF, -1 },\n", indent);
    table = CcLexical_GetStartTab(&self->globals->lexical, &numEle);
    for (cur = table; cur - table < numEle; ++cur)
	fprintf(outfp, "%s{ %d, %d, %d },\n", indent,
		cur->keyFrom, cur->keyTo, cur->state);
    CcFree(table);
    return TRUE;
}

static CcsBool_t
COS_Identifiers2KeywordKinds(CcOutputScheme_t * self, FILE * outfp,
			     const char * indent)
{
    int numEle;
    CcLexical_Identifier_t * list, * cur;

    list = CcLexical_GetIdentifiers(&self->globals->lexical, &numEle);
    for (cur = list; cur - list < numEle; ++cur)
	fprintf(outfp, "%s{ %s, %d },\n", indent, cur->name, cur->index);
    CcLexical_Identifiers_Destruct(list, numEle);
    return TRUE;
}

static CcsBool_t
COS_Comments(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    const CcComment_t * cur;
    for (cur = self->globals->lexical.firstComment; cur; cur = cur->next)
	fprintf(outfp, "%s    { { %d, %d }, { %d, %d }, %s },\n", indent,
		cur->start[0], cur->start[1], cur->stop[0], cur->stop[1],
		cur->nested ? "TRUE" : "FALSE");
    return TRUE;
}

static CcsBool_t
COS_Scan1(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    const CcRange_t * curRange;
    for (curRange = self->globals->lexical.ignored->head;
	 curRange; curRange = curRange->next) {
	if (curRange->from == curRange->to)
	    fprintf(outfp, "%s|| self->ch == %d\n", indent, curRange->from);
	else
	    fprintf(outfp, "%s|| (self->ch >= %d && self->ch <= %d)\n", indent,
		    curRange->from, curRange->to);
    }
    return TRUE;
}

static void
COS_WriteState(CcOutputScheme_t * self, FILE * outfp, const char * indent,
	       const CcState_t * state, const CcBitArray_t * mask)
{
    const CcAction_t * action;
    CcCharSet_t * s; const CcRange_t * curRange;
    int sIndex = state->base.index;

    if (CcBitArray_Get(mask, sIndex))
	fprintf(outfp, "%scase %d: case_%d:\n", indent, sIndex, sIndex);
    else
	fprintf(outfp, "%scase %d:\n", indent, sIndex);
    for (action = state->firstAction; action != NULL; action = action->next) {
	if (action == state->firstAction) fprintf(outfp, "%s    if (", indent);
	else fprintf(outfp, "%s    } else if (", indent);
	s = CcTransition_GetCharSet(&action->trans);
	for (curRange = s->head; curRange; curRange = curRange->next) {
	    if (curRange != s->head) fprintf(outfp, "%s        ", indent);
	    if (curRange->from == curRange->to)
		fprintf(outfp, "self->ch == %d", curRange->from);
	    else
		fprintf(outfp, "(self->ch >= %d && self->ch <= %d)",
			curRange->from, curRange->to);
	    if (curRange->next) fprintf(outfp, " ||\n");
	}
	fprintf(outfp, ") {\n");
	fprintf(outfp, "%s        CcsScanner_GetCh(self); goto case_%d;\n",
		indent, action->target->state->base.index);
	CcCharSet_Destruct(s);
    }
    if (state->firstAction == NULL) fprintf(outfp, "%s    {\n", indent);
    else fprintf(outfp, "%s    } else {\n", indent);
    if (state->endOf == NULL) {
	fprintf(outfp, "%s        kind = self->noSym; break;\n", indent);
    } else {
	fprintf(outfp, "%s        kind = %d;\n", indent,
		state->endOf->base.index);
	if (CcSymbol_GetTokenKind(state->endOf) == symbol_classLitToken)
	    fprintf(outfp,
		    "%s        kind = Identifier2KWKind(CcsBuffer_GetString(&self->buffer, pos, self->pos - pos),\n"
		    "%s                                 self->pos - pos, kind);\n",
		    indent, indent);
	fprintf(outfp, "%s        break;\n", indent);
    }
    fprintf(outfp, "%s    }\n", indent);
}

static CcsBool_t
COS_Scan3(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    CcArrayListIter_t iter;
    CcBitArray_t mask;
    const CcState_t * state;
    CcArrayList_t * stateArr = &self->globals->lexical.states;

    CcLexical_TargetStates(&self->globals->lexical, &mask);
    state = (CcState_t *)CcArrayList_First(stateArr, &iter);
    for (state = (const CcState_t *)CcArrayList_Next(stateArr, &iter);
	 state; state = (const CcState_t *)CcArrayList_Next(stateArr, &iter))
	COS_WriteState(self, outfp, indent, state, &mask);
    CcBitArray_Destruct(&mask);
    return TRUE;
}

static CcsBool_t
COS_Initialization(CcOutputScheme_t * self, FILE * outfp, const char * indent)
{
    char * setstr; int setlen, index;
    const CcBitArray_t * cur;
    CcCOutputScheme_t * ccself = (CcCOutputScheme_t *)self;

    setlen = self->globals->symtab.terminals.Count;
    setstr = CcMalloc(setlen + 1); setstr[setlen] = 0;
    for (cur = ccself->symSet.start; cur < ccself->symSet.used; ++cur) {
	CcsAssert(setlen == CcBitArray_getCount(cur));
	for (index = 0; index < setlen; ++index)
	    setstr[index] = CcBitArray_Get(cur, index) ? '*' : '.';
	fprintf(outfp, "%s\"%s\",\n", indent, setstr);
    }
    CcFree(setstr);
    return TRUE;
}

static CcsBool_t
CcCOutputScheme_write(CcOutputScheme_t * self, FILE * outfp,
		      const char * func, const char * param,
		      const char * indent)
{
    if (!strcmp(func, "defines")) {
	return COS_Defines(self, outfp, indent);
    } else if (!strcmp(func, "declarations")) {
	return COS_Declarations(self, outfp, indent);
    } else if (!strcmp(func, "chars2states")) {
	return COS_Chars2States(self, outfp, indent);
    } else if (!strcmp(func, "identifiers2keywordkinds")) {
	return COS_Identifiers2KeywordKinds(self, outfp, indent);
    } else if (!strcmp(func, "comments")) {
	return COS_Comments(self, outfp, indent);
    } else if (!strcmp(func, "scan1")) {
	return COS_Scan1(self, outfp, indent);
    } else if (!strcmp(func, "scan3")) {
	return COS_Scan3(self, outfp, indent);
    } else if (!strcmp(func, "initialization")) {
	return COS_Initialization(self, outfp, indent);
    }
    fprintf(stderr, "Unknown section '%s' encountered.\n", func);
    return FALSE;
}

static void
CcCOutputScheme_Destruct(CcObject_t * self)
{
    CcCOutputScheme_t * ccself = (CcCOutputScheme_t *)self;
    CcSyntaxSymSet_Destruct(&ccself->symSet);
    CcOutputScheme_Destruct(self);
}

static const CcOutputSchemeType_t COutputSchemeType = {
    { sizeof(CcCOutputScheme_t), "COutputScheme", CcCOutputScheme_Destruct },
    CcCOutputScheme_OutputInfos, CcCOutputScheme_write
};

CcCOutputScheme_t *
CcCOutputScheme(CcGlobals_t * globals, CcArguments_t * arguments)
{
    CcCOutputScheme_t * self = (CcCOutputScheme_t *)
	CcOutputScheme(&COutputSchemeType, globals, arguments);
    CcSyntaxSymSet(&self->symSet);
    return self;
}
