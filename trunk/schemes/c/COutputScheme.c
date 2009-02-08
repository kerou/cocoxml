/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "c/COutputScheme.h"
#include  "c/Indent.h"
#include  "lexical/Action.h"
#include  "lexical/CharSet.h"
#include  "lexical/Comment.h"
#include  "lexical/State.h"
#include  "lexical/Target.h"
#include  "lexical/Transition.h"

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

static CcsBool_t
COS_Defines(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcSymbol_t * sym;
    CcHTIterator_t iter;
    CcSymbolTable_t * symtab = &self->base.base.globals->symtab;
    CcLexical_t * lexical = self->base.base.globals->lexical;

    CcPrintfIL(output, "#define %sScanner_MAX_KEYWORD_LEN %d",
	       self->base.prefix, CcLexical_GetMaxKeywordLength(lexical));
    if (!lexical->ignoreCase)
	CcPrintfIL(output, "#define %sScanner_CASE_SENSITIVE", self->base.prefix);
    if (CcLexical_KeywordUsed(lexical))
	CcPrintfIL(output, "#define %sScanner_KEYWORD_USED", self->base.prefix);
    if (lexical->indentUsed) {
	CcPrintfIL(output, "#define %sScanner_INDENTATION", self->base.prefix);
	sym = CcSymbolTable_FindSym(symtab, IndentInName);
	CcsAssert(sym != NULL);
	CcPrintfIL(output, "#define %sScanner_INDENT_IN %d",
		   self->base.prefix, sym->kind);
	sym = CcSymbolTable_FindSym(symtab, IndentOutName);
	CcsAssert(sym != NULL);
	CcPrintfIL(output, "#define %sScanner_INDENT_OUT %d",
		   self->base.prefix, sym->kind);
	sym = CcSymbolTable_FindSym(symtab, IndentErrName);
	CcPrintfIL(output, "#define %sScanner_INDENT_ERR %d",
		   self->base.prefix, sym->kind);
    }
    CcHashTable_GetIterator(&lexical->addedTerminals, &iter);
    while (CcHTIterator_Forward(&iter)) {
	sym = (CcSymbol_t *)CcHTIterator_Value(&iter);
	CcPrintfIL(output, "#define %sScanner_%s %d",
		   self->base.prefix, CcHTIterator_Key(&iter), sym->kind);
    }
    return TRUE;
}

static CcsBool_t
COS_Declarations(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int addSpace = 0;
    if (self->base.base.globals->lexical->indentUsed)
	addSpace += sizeof(CcsIndent_t);
    CcPrintfIL(output, "%d, /* additionalSpace */", addSpace);
    CcPrintfIL(output, "%d, /* eofSym */",
	       self->base.base.globals->syntax.eofSy->kind);
    CcPrintfIL(output, "%d, /* maxT */",
	       self->base.base.globals->symtab.terminals.Count - 1);
    CcPrintfIL(output, "%d, /* noSym */",
	       self->base.base.globals->syntax.noSy->kind);
    return TRUE;
}

static CcsBool_t
COS_CheckLineStart(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcLexical_t * lexical = self->base.base.globals->lexical;

    CcPrintfIL(output, "lineStart = (si->ch == '\\n'%s);",
	       lexical->backslashNewline ?
	       " && si->chLastNonblank != '\\\\'" : "");
    return TRUE;
}

static CcsBool_t
COS_Chars2States(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int numEle;
    CcLexical_StartTab_t * table, * cur;
    char buf0[8], buf1[8];
    CcPrintfIL(output, "{ EoF, EoF, -1 },");
    table = CcLexical_GetStartTab(self->base.base.globals->lexical, &numEle);
    for (cur = table; cur - table < numEle; ++cur)
	CcPrintfIL(output, "{ %d, %d, %d },\t/* %s %s */",
		   cur->keyFrom, cur->keyTo, cur->state,
		   CharRepr(buf0, sizeof(buf0), cur->keyFrom),
		   CharRepr(buf1, sizeof(buf1), cur->keyTo));
    CcFree(table);
    return TRUE;
}

static CcsBool_t
COS_Identifiers2KeywordKinds(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int numEle;
    CcLexical_Identifier_t * list, * cur;

    list = CcLexical_GetIdentifiers(self->base.base.globals->lexical, &numEle);
    for (cur = list; cur - list < numEle; ++cur)
	CcPrintfIL(output, "{ %s, %d },", cur->name, cur->index);
    CcLexical_Identifiers_Destruct(list, numEle);
    return TRUE;
}

static CcsBool_t
COS_Comments(CcCOutputScheme_t * self, CcOutput_t * output)
{
    const CcComment_t * cur;
    char buf0[8], buf1[8], buf2[8], buf3[8];
    output->indent += 4;
    for (cur = self->base.base.globals->lexical->firstComment; cur; cur = cur->next)
	CcPrintfIL(output, "{ { %s, %s }, { %s, %s }, %s },",
		   CharRepr(buf0, sizeof(buf0), cur->start[0]),
		   CharRepr(buf1, sizeof(buf1), cur->start[1]),
		   CharRepr(buf2, sizeof(buf2), cur->stop[0]),
		   CharRepr(buf3, sizeof(buf3), cur->stop[1]),
		   cur->nested ? "TRUE" : "FALSE");
    output->indent -= 4;
    return TRUE;
}

static CcsBool_t
COS_Scan1(CcCOutputScheme_t * self, CcOutput_t * output)
{
    const CcRange_t * curRange;
    char buf0[8], buf1[8];
    const char * oper = "";
    CcCharSet_t * ignored = self->base.base.globals->lexical->ignored;

    if (ignored->head == NULL) {
	CcPrintfIL(output, "FALSE");
    } else {
	for (curRange = ignored->head; curRange; curRange = curRange->next) {
	    if (curRange->from == curRange->to)
		CcPrintfIL(output, "%sinput->ch == %s", oper,
			   CharRepr(buf0 ,sizeof(buf0), curRange->from));
	    else
		CcPrintfIL(output, "%s(input->ch >= %s && input->ch <= %s)", oper,
			   CharRepr(buf0 ,sizeof(buf0), curRange->from),
			   CharRepr(buf1 ,sizeof(buf1), curRange->to));
	    oper = "|| ";
	}
    }
    return TRUE;
}

static void
COS_WriteState(CcCOutputScheme_t * self, CcOutput_t * output,
	       const CcState_t * state, const CcBitArray_t * mask)
{
    const CcAction_t * action;
    CcCharSet_t * s; const CcRange_t * curRange;
    char buf0[8], buf1[8];
    int sIndex = state->base.index;

    if (CcBitArray_Get(mask, sIndex))
	CcPrintfIL(output, "case %d: case_%d:", sIndex, sIndex);
    else
	CcPrintfIL(output, "case %d:", sIndex);
    output->indent += 4;
    for (action = state->firstAction; action != NULL; action = action->next) {
	if (action == state->firstAction) CcPrintfI(output, "if (");
	else CcPrintfI(output, "} else if (");
	s = CcTransition_GetCharSet(&action->trans);
	for (curRange = s->head; curRange; curRange = curRange->next) {
	    if (curRange != s->head) CcPrintfI(output, "    ");
	    if (curRange->from == curRange->to)
		CcPrintf(output,"input->ch == %s",
			CharRepr(buf0, sizeof(buf0), curRange->from));
	    else
		CcPrintf(output, "(input->ch >= %s && input->ch <= %s)",
			 CharRepr(buf0, sizeof(buf0), curRange->from),
			 CharRepr(buf1, sizeof(buf1), curRange->to));
	    if (curRange->next) CcPrintfL(output, " ||");
	}
	CcPrintfL(output, ") {");
	output->indent += 4;
	CcPrintfIL(output, "CcsGetCh(input); goto case_%d;",
		   action->target->state->base.index);
	output->indent -= 4;
	CcCharSet_Destruct(s);
    }

    if (state->firstAction == NULL) CcPrintfI(output, "{ ");
    else CcPrintfI(output, "} else { ");
    if (state->endOf == NULL) {
	CcPrintf(output, "kind = Scanner_Info.noSym;");
    } else if (CcSymbol_GetTokenKind(state->endOf) != symbol_classLitToken) {
	CcPrintf(output, "kind = %d;", state->endOf->kind);
    } else {
	CcPrintf(output, "kind = GetKWKind(input, pos, input->pos, %d);",
		 state->endOf->kind);
    }
    CcPrintfL(output, " break; }");
    output->indent -= 4;
}

static CcsBool_t
COS_Scan3(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcArrayListIter_t iter;
    CcBitArray_t mask;
    const CcState_t * state;
    CcArrayList_t * stateArr = &self->base.base.globals->lexical->states;

    CcLexical_TargetStates(self->base.base.globals->lexical, &mask);
    state = (CcState_t *)CcArrayList_First(stateArr, &iter);
    for (state = (const CcState_t *)CcArrayList_Next(stateArr, &iter);
	 state; state = (const CcState_t *)CcArrayList_Next(stateArr, &iter))
	COS_WriteState(self, output, state, &mask);
    CcBitArray_Destruct(&mask);
    return TRUE;
}

static CcsBool_t
CcCOutputScheme_write(CcOutputScheme_t * self, CcOutput_t * output,
		      const char * func, const char * param)
{
    CcCOutputScheme_t * ccself = (CcCOutputScheme_t *)self;

    if (!strcmp(func, "defines")) {
	return COS_Defines(ccself, output);
    } else if (!strcmp(func, "declarations")) {
	return COS_Declarations(ccself, output);
    } else if (!strcmp(func, "checkLineStart")) {
	return COS_CheckLineStart(ccself, output);
    } else if (!strcmp(func, "chars2states")) {
	return COS_Chars2States(ccself, output);
    } else if (!strcmp(func, "identifiers2keywordkinds")) {
	return COS_Identifiers2KeywordKinds(ccself, output);
    } else if (!strcmp(func, "comments")) {
	return COS_Comments(ccself, output);
    } else if (!strcmp(func, "scan1")) {
	return COS_Scan1(ccself, output);
    } else if (!strcmp(func, "scan3")) {
	return COS_Scan3(ccself, output);
    } else if (!strcmp(func, "SubScanners")) {
	/* SubScanners section is not presented, omit it. */
	return TRUE;
    }
    return CcCBaseOutputScheme_write(self, output, func, param);
}

static const CcOutputSchemeType_t COutputSchemeType = {
    { sizeof(CcCOutputScheme_t), "COutputScheme",
      CcCBaseOutputScheme_Destruct },
    /* If the following lists are modified, modify install.py too. */
    "Parser.c\0Scanner.c\0Parser.h\0Scanner.h\0\0",
    CcCOutputScheme_write
};

CcCOutputScheme_t *
CcCOutputScheme(CcsParser_t * parser, CcArguments_t * arguments)
{
    CcCOutputScheme_t * self = (CcCOutputScheme_t *)
	CcCBaseOutputScheme(&COutputSchemeType, &parser->globals, arguments);
    self->parser = parser;
    return self;
}
