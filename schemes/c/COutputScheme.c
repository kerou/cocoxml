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
#include  "XmlSpec.h"
#include  "syntax/Nodes.h"

/* When the number of possible terminals is greater than maxTerm,
   symSet is used. */
#define  maxTerm  3

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
    CcPrintfIL(output, "#define %sScanner_MAX_KEYWORD_LEN %d", self->prefix,
	       CcLexical_GetMaxKeywordLength(self->base.globals->lexical));
    if (!self->base.globals->lexical->ignoreCase)
	CcPrintfIL(output, "#define %sScanner_CASE_SENSITIVE", self->prefix);
    if (CcLexical_KeywordUsed(self->base.globals->lexical))
	CcPrintfIL(output, "#define %sScanner_KEYWORD_USED", self->prefix);
    if (self->base.globals->lexical->indentUsed) {
	CcPrintfIL(output, "#define %sScanner_INDENTATION", self->prefix);
	CcPrintfIL(output, "#define %sScanner_INDENT_START %d",
		   self->prefix, 32);
	sym = CcSymbolTable_FindSym(&self->base.globals->symtab, IndentInName);
	CcsAssert(sym != NULL);
	CcPrintfIL(output, "#define %sScanner_INDENT_IN %d", self->prefix,
		   sym->kind);
	sym = CcSymbolTable_FindSym(&self->base.globals->symtab, IndentOutName);
	CcsAssert(sym != NULL);
	CcPrintfIL(output, "#define %sScanner_INDENT_OUT %d", self->prefix,
		   sym->kind);
	sym = CcSymbolTable_FindSym(&self->base.globals->symtab, IndentErrName);
	CcPrintfIL(output, "#define %sScanner_INDENT_ERR %d", self->prefix,
		   sym->kind);
    }
    return TRUE;
}

static CcsBool_t
COS_Declarations(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcPrintfIL(output, "self->eofSym = %d;",
	       self->base.globals->syntax.eofSy->kind);
    CcPrintfIL(output, "self->maxT = %d;",
	       self->base.globals->symtab.terminals.Count - 1);
    CcPrintfIL(output, "self->noSym = %d;",
	       self->base.globals->syntax.noSy->kind);
    return TRUE;
}

static CcsBool_t
COS_Chars2States(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int numEle;
    CcLexical_StartTab_t * table, * cur;
    char buf0[8], buf1[8];
    CcPrintfIL(output, "{ EoF, EoF, -1 },");
    table = CcLexical_GetStartTab(self->base.globals->lexical, &numEle);
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

    list = CcLexical_GetIdentifiers(self->base.globals->lexical, &numEle);
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
    for (cur = self->base.globals->lexical->firstComment; cur; cur = cur->next)
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
    for (curRange = self->base.globals->lexical->ignored->head;
	 curRange; curRange = curRange->next) {
	if (curRange->from == curRange->to)
	    CcPrintfIL(output, "|| self->ch == %s",
		       CharRepr(buf0 ,sizeof(buf0), curRange->from));
	else
	    CcPrintfIL(output, "|| (self->ch >= %s && self->ch <= %s)",
		       CharRepr(buf0 ,sizeof(buf0), curRange->from),
		       CharRepr(buf1 ,sizeof(buf1), curRange->to));
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
		CcPrintf(output,"self->ch == %s",
			CharRepr(buf0, sizeof(buf0), curRange->from));
	    else
		CcPrintf(output, "(self->ch >= %s && self->ch <= %s)",
			 CharRepr(buf0, sizeof(buf0), curRange->from),
			 CharRepr(buf1, sizeof(buf1), curRange->to));
	    if (curRange->next) CcPrintfL(output, " ||");
	}
	CcPrintfL(output, ") {");
	output->indent += 4;
	CcPrintfIL(output, "%sScanner_GetCh(self); goto case_%d;",
		   self->prefix, action->target->state->base.index);
	output->indent -= 4;
	CcCharSet_Destruct(s);
    }

    if (state->firstAction == NULL) CcPrintfI(output, "{ ");
    else CcPrintfI(output, "} else { ");
    if (state->endOf == NULL) {
	CcPrintf(output, "kind = self->noSym;");
    } else if (CcSymbol_GetTokenKind(state->endOf) != symbol_classLitToken) {
	CcPrintf(output, "kind = %d;", state->endOf->kind);
    } else {
	CcPrintf(output,
		 "kind = %sScanner_GetKWKind(self, pos, self->pos, %d);",
		 self->prefix, state->endOf->kind);
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
    CcArrayList_t * stateArr = &self->base.globals->lexical->states;

    CcLexical_TargetStates(self->base.globals->lexical, &mask);
    state = (CcState_t *)CcArrayList_First(stateArr, &iter);
    for (state = (const CcState_t *)CcArrayList_Next(stateArr, &iter);
	 state; state = (const CcState_t *)CcArrayList_Next(stateArr, &iter))
	COS_WriteState(self, output, state, &mask);
    CcBitArray_Destruct(&mask);
    return TRUE;
}

static CcsBool_t
COS_KindUnknownNS(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcsAssert(self->base.globals->xmlspecmap);
    CcPrintfIL(output, "self->base.kindUnknownNS = %d;",
	       self->base.globals->xmlspecmap->kindUnknownNS);
    return TRUE;
}

static int
cmpSpecKey(const void * cs0, const void * cs1)
{
    return strcmp(*(const char **)cs0, *(const char **)cs1);
}

static CcsBool_t
COS_XmlSpecSubLists(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int count; CcHTIterator_t iter;
    const char ** keylist, ** curkey;
    const CcXmlSpec_t * spec;
    CcXmlSpecData_t * datalist, * datacur; size_t datanum; char * tmp;
    CcXmlSpecMap_t * map = self->base.globals->xmlspecmap;

    CcsAssert(map != NULL);
    count = CcHashTable_Num(&map->map);
    keylist = curkey = CcMalloc(sizeof(char *) * count);
    CcHashTable_GetIterator(&map->map, &iter);
    while (CcHTIterator_Forward(&iter)) *curkey++ = CcHTIterator_Key(&iter);
    CcsAssert(curkey - keylist == count);
    qsort(keylist, count, sizeof(const char *), cmpSpecKey);

    CcPrintfIL(output, "static const CcxTag_t XmlTags[] = {");
    for (curkey = keylist; curkey - keylist < count; ++curkey) {
	spec = (const CcXmlSpec_t *)CcHashTable_Get(&map->map, *curkey);
	CcsAssert(spec != NULL);
	datalist = CcXmlSpec_GetSortedTagList(spec, self->base.globals,
					      &datanum);
	if (datanum == 0) continue;
	output->indent += 4;
	for (datacur = datalist; datacur - datalist < datanum; ++datacur) {
	    tmp = CcEscape(datacur->name);
	    CcPrintfIL(output, "{ %s, %d, %d },",
		       tmp, datacur->kind0, datacur->kind1);
	    CcFree(tmp);
	}
	output->indent -= 4;
	CcXmlSpecData_Destruct(datalist, datanum);
    }
    CcPrintfL(output, "};");

    CcPrintfIL(output, "static const CcxAttr_t XmlAttrs[] = {");
    for (curkey = keylist; curkey - keylist < count; ++curkey) {
	spec = (const CcXmlSpec_t *)CcHashTable_Get(&map->map, *curkey);
	CcsAssert(spec != NULL);
	datalist = CcXmlSpec_GetSortedAttrList(spec, self->base.globals,
					       &datanum);
	if (datanum == 0) continue;
	output->indent += 4;
	for (datacur = datalist; datacur - datalist < datanum; ++datacur) {
	    tmp = CcEscape(datacur->name);
	    CcPrintfIL(output, "{ %s, %d },", tmp, datacur->kind0);
	    CcFree(tmp);
	}
	output->indent -= 4;
	CcXmlSpecData_Destruct(datalist, datanum);
    }
    CcPrintfL(output, "};");

    CcPrintfIL(output, "static const CcxPInstruction_t XmlPIs[] = {");
    for (curkey = keylist; curkey - keylist < count; ++curkey) {
	spec = (const CcXmlSpec_t *)CcHashTable_Get(&map->map, *curkey);
	CcsAssert(spec != NULL);
	datalist = CcXmlSpec_GetSortedPIList(spec, self->base.globals,
					     &datanum);
	if (datanum == 0) continue;
	output->indent += 4;
	for (datacur = datalist; datacur - datalist < datanum; ++datacur) {
	    tmp = CcEscape(datacur->name);
	    CcPrintfIL(output, "{ %s, %d },", tmp, datacur->kind0);
	    CcFree(tmp);
	}
	output->indent -= 4;
	CcXmlSpecData_Destruct(datalist, datanum);
    }
    CcPrintfL(output, "};");

    CcFree(keylist);
    return TRUE;
}

static CcsBool_t
COS_XmlSpecList(CcCOutputScheme_t * self, CcOutput_t * output)
{
    int kinds[XSO_SIZE];
    int count; CcHTIterator_t iter;
    const char ** keylist, ** curkey;
    int cntTagList, cntAttrList, cntPIList;
    char * tmp; CcsXmlSpecOption_t option;
    const CcXmlSpec_t * spec;
    CcXmlSpecData_t * datalist; size_t datanum;
    CcXmlSpecMap_t * map = self->base.globals->xmlspecmap;

    CcsAssert(map != NULL);
    CcXmlSpecMap_GetOptionKinds(map, kinds, self->base.globals);
    count = CcHashTable_Num(&map->map);
    keylist = curkey = CcMalloc(sizeof(char *) * count);
    CcHashTable_GetIterator(&map->map, &iter);
    while (CcHTIterator_Forward(&iter)) *curkey++ = CcHTIterator_Key(&iter);
    CcsAssert(curkey - keylist == count);
    qsort(keylist, count, sizeof(const char *), cmpSpecKey);

    cntTagList = cntAttrList = cntPIList = 0;
    for (curkey = keylist; curkey - keylist < count; ++curkey) {
	spec = (const CcXmlSpec_t *)CcHashTable_Get(&map->map, *curkey);
	CcsAssert(spec != NULL);
	tmp = CcEscape(*curkey);
	CcPrintfIL(output, "{ %s, %s,", tmp,
		   spec->caseSensitive ? "TRUE" : "FALSE");
	CcFree(tmp);
	output->indent += 4;
	CcPrintfI(output, "{");
	for (option = XSO_UnknownTag; option < XSO_SIZE; ++option)
	    CcPrintf(output, " %d,",
		     CcBitArray_Get(&spec->options, option) ? kinds[option] : -1);
	CcPrintfL(output, " },");

	datalist = CcXmlSpec_GetSortedTagList(spec, self->base.globals,
					      &datanum);
	if (datanum == 0) CcPrintfIL(output, "NULL, 0, /* Tags */");
	else {
	    CcPrintfIL(output, "XmlTags + %d, %d,", cntTagList, datanum);
	    cntTagList += datanum;
	}
	CcXmlSpecData_Destruct(datalist, datanum);

	datalist = CcXmlSpec_GetSortedAttrList(spec, self->base.globals,
					       &datanum);
	if (datanum == 0) CcPrintfIL(output, "NULL, 0, /* Attrs */");
	else {
	    CcPrintfIL(output, "XmlAttrs + %d, %d,", cntAttrList, datanum);
	    cntAttrList += datanum;
	}
	CcXmlSpecData_Destruct(datalist, datanum);

	datalist = CcXmlSpec_GetSortedPIList(spec, self->base.globals,
					     &datanum);
	if (datanum == 0) {
	    CcPrintfIL(output, "NULL, 0, /* Processing Instructions */");
	} else {
	    CcPrintfIL(output, "XmlPIs + %d, %d,", cntPIList, datanum);
	    cntPIList += datanum;
	}
	CcXmlSpecData_Destruct(datalist, datanum);

	output->indent -= 4;
	CcPrintfIL(output, "},");
    }
    CcFree(keylist);
    return TRUE;
}

static CcsBool_t
COS_SynDefines(CcCOutputScheme_t * self, CcOutput_t * output)
{
    if (self->base.globals->syntax.weakUsed)
	CcPrintfIL(output, "#define %sParser_WEAK_USED", self->prefix);
    return TRUE;
}

static CcsBool_t
COS_Members(CcCOutputScheme_t * self, CcOutput_t * output)
{
    if (self->parser &&	self->parser->members)
	CcSource(output, self->parser->members);
    if (self->xmlparser && self->xmlparser->members)
	CcSource(output, self->xmlparser->members);
    return TRUE;
}

static CcsBool_t
COS_Constructor(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcPrintfIL(output, "self->maxT = %d;",
	      self->base.globals->symtab.terminals.Count - 1);
    if (self->parser && self->parser->constructor)
	CcSource(output, self->parser->constructor);
    if (self->xmlparser && self->xmlparser->constructor)
	CcSource(output, self->xmlparser->constructor);
    return TRUE;
}

static CcsBool_t
COS_Destructor(CcCOutputScheme_t * self, CcOutput_t * output)
{
    if (self->parser && self->parser->destructor)
	CcSource(output, self->parser->destructor);
    if (self->xmlparser && self->xmlparser->destructor)
	CcSource(output, self->xmlparser->destructor);
    return TRUE;
}

static CcsBool_t
COS_Pragmas(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcArrayListIter_t iter;
    const CcSymbolPR_t * sym, * sym1;
    const CcArrayList_t * pragmas = &self->base.globals->symtab.pragmas;

    for (sym = sym1 = (const CcSymbolPR_t *)CcArrayList_FirstC(pragmas, &iter);
	 sym; sym = (const CcSymbolPR_t *)CcArrayList_NextC(pragmas, &iter)) {
	CcPrintfIL(output, "%sif (self->la->kind == %d) {",
		   (sym == sym1) ? "" : "} else ", sym->base.kind);
	if (sym->semPos) {
	    output->indent += 4;
	    CcSource(output, sym->semPos);
	    output->indent -= 4;
	}
    }
    if (sym1) CcPrintfIL(output, "}");
    return TRUE;
}

static CcsBool_t
COS_ProductionsHeader(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcArrayListIter_t iter;
    const CcSymbolNT_t * sym;
    const CcArrayList_t * nonterminals =
	&self->base.globals->symtab.nonterminals;

    for (sym = (const CcSymbolNT_t *)CcArrayList_FirstC(nonterminals, &iter);
	 sym;
	 sym = (const CcSymbolNT_t *)CcArrayList_NextC(nonterminals, &iter))
	if (sym->attrPos)
	    CcPrintfIL(output,
		       "static void %sParser_%s(%sParser_t * self, %s);",
		       self->prefix, sym->base.name,
		       self->prefix, sym->attrPos->text);
	else
	    CcPrintfIL(output, "static void %sParser_%s(%sParser_t * self);",
		       self->prefix, sym->base.name, self->prefix);
    return TRUE;
}

static CcsBool_t
COS_ParseRoot(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcPrintfIL(output, "%sParser_%s(self);", self->prefix,
	       self->base.globals->syntax.gramSy->name);
    return TRUE;
}

static void
SCOS_GenCond(CcCOutputScheme_t * self, CcOutput_t * output,
	     const char * prefix, const char * suffix,
	     const CcBitArray_t * s, const CcNode_t * p)
{
    const CcNodeRSLV_t * prslv; int n;
    CcArrayListIter_t iter; const CcSymbol_t * sym;
    const CcArrayList_t * terminals;

    if (p->base.type == node_rslv) {
	prslv = (CcNodeRSLV_t *)p;
	CcPrintfIL(output, "%s%s%s", prefix, prslv->pos->text, suffix);
    } else if ((n = CcBitArray_Elements(s)) == 0) {
	CcPrintfIL(output, "%s%s%s", prefix, "FALSE", suffix);
    } else if (n <= maxTerm) {
	CcPrintfI(output, "%s", prefix);
	terminals = &self->base.globals->symtab.terminals;
	for (sym = (const CcSymbol_t *)CcArrayList_FirstC(terminals, &iter);
	     sym; sym = (const CcSymbol_t *)CcArrayList_NextC(terminals, &iter))
	    if (CcBitArray_Get(s, sym->kind)) {
		CcPrintf(output, "self->la->kind == %d", sym->kind);
		if (--n > 0) CcPrintf(output, " || ");
	    }
	CcPrintfL(output, "%s", suffix);
    } else {
	CcPrintfIL(output, "%s%sParser_StartOf(self, %d)%s",
		   prefix, self->prefix,
		   CcSyntaxSymSet_New(&self->symSet, s), suffix);
    }
}

static CcsBool_t
SCOS_UseSwitch(CcCOutputScheme_t * self, CcNode_t * p)
{
    CcBitArray_t s1, s2; int nAlts;
    CcSyntax_t * syntax = &self->base.globals->syntax;
    CcArrayList_t * terminals = &self->base.globals->symtab.terminals;

    if (p->base.type != node_alt) return FALSE;
    nAlts = 0;
    CcBitArray(&s1, terminals->Count);
    while (p != NULL) {
	CcSyntax_Expected0(syntax, &s2, p->sub, self->curSy);
	if (CcBitArray_Intersect(&s1, &s2)) goto falsequit2;
	CcBitArray_Or(&s1, &s2);
	CcBitArray_Destruct(&s2);
	++nAlts;
	if (p->sub->base.type == node_rslv) goto falsequit1;
	p = p->down;
    }
    CcBitArray_Destruct(&s1);
    return nAlts > 5;
 falsequit2:
    CcBitArray_Destruct(&s2);
 falsequit1:
    CcBitArray_Destruct(&s1);
    return FALSE;
}

static void
SCOS_GenCode(CcCOutputScheme_t * self, CcOutput_t * output,
	     CcNode_t * p, const CcBitArray_t * IsChecked)
{
    int err; CcsBool_t equal, useSwitch; int index;
    CcNode_t * p2; CcBitArray_t s1, s2, isChecked;
    CcNodeNT_t * pnt; CcNodeT_t * pt; CcNodeWT_t * pwt;
    CcNodeSEM_t * psem; CcNodeSYNC_t * psync;
    CcSyntax_t * syntax = &self->base.globals->syntax;
    CcArrayList_t * terminals = &self->base.globals->symtab.terminals;

    CcBitArray_Clone(&isChecked, IsChecked);
    while (p != NULL) {
	if (p->base.type == node_nt) {
	    pnt = (CcNodeNT_t *)p;
	    if (pnt->pos) {
		CcPrintfIL(output, "%sParser_%s(self, %s);",
			   self->prefix, pnt->sym->name, pnt->pos->text);
	    } else {
		CcPrintfIL(output, "%sParser_%s(self);",
			   self->prefix, pnt->sym->name);
	    }
	} else if (p->base.type == node_t) {
	    pt = (CcNodeT_t *)p;
	    if (CcBitArray_Get(&isChecked, pt->sym->kind))
		CcPrintfIL(output, "%sParser_Get(self);", self->prefix);
	    else
		CcPrintfIL(output, "%sParser_Expect(self, %d);",
			   self->prefix, pt->sym->kind);
	} else if (p->base.type == node_wt) {
	    pwt = (CcNodeWT_t *)p;
	    CcSyntax_Expected(syntax, &s1, p->next, self->curSy);
	    CcBitArray_Or(&s1, syntax->allSyncSets);
	    CcPrintfIL(output, "%sParser_ExpectWeak(self, %d, %d);",
		       self->prefix, pwt->sym->kind,
		       CcSyntaxSymSet_New(&self->symSet, &s1));
	    CcBitArray_Destruct(&s1);
	} else if (p->base.type == node_any) {
	    CcPrintfIL(output, "%sParser_Get(self);", self->prefix);
	} else if (p->base.type == node_eps) {
	} else if (p->base.type == node_rslv) {
	} else if (p->base.type == node_sem) {
	    psem = (CcNodeSEM_t *)p;
	    CcSource(output, psem->pos);
	} else if (p->base.type == node_sync) {
	    psync = (CcNodeSYNC_t *)p;
	    err = CcSyntax_SyncError(syntax, self->curSy);
	    CcBitArray_Clone(&s1, psync->set);
	    SCOS_GenCond(self, output, "while (!(", ")) {", &s1, p);
	    output->indent += 4;
	    CcPrintfIL(output, "%sParser_SynErr(self, %d); %sParser_Get(self);",
		       self->prefix, err, self->prefix);
	    output->indent -= 4;
	    CcPrintfIL(output, "}");
	    CcBitArray_Destruct(&s1);
	} else if (p->base.type == node_alt) {
	    CcSyntax_First(syntax, &s1, p);
	    equal = CcBitArray_Equal(&s1, &isChecked);
	    CcBitArray_Destruct(&s1);
	    useSwitch = SCOS_UseSwitch(self, p);
	    if (useSwitch)
		CcPrintfIL(output, "switch (self->la->kind) {");
	    p2 = p;
	    while (p2 != NULL) {
		CcSyntax_Expected(syntax, &s1, p2->sub, self->curSy);
		if (useSwitch) {
		    CcPrintfI(output, "");
		    for (index = 0; index < terminals->Count; ++index)
			if (CcBitArray_Get(&s1, index))
			    CcPrintf(output, "case %d: ", index);
		    CcPrintfL(output,"{");
		} else if (p2 == p) {
		    SCOS_GenCond(self, output, "if (", ") {", &s1, p2->sub);
		} else if (p2->down == NULL && equal) {
		    CcPrintfIL(output, "} else {");
		} else {
		    SCOS_GenCond(self, output,
				 "} else if (", ") {", &s1, p2->sub);
		}
		CcBitArray_Or(&s1, &isChecked);
		output->indent += 4;
		SCOS_GenCode(self, output, p2->sub, &s1);
		if (useSwitch) CcPrintfIL(output, "break;");
		output->indent -= 4;
		if (useSwitch) CcPrintfIL(output, "}");
		p2 = p2->down;
		CcBitArray_Destruct(&s1);
	    }
	    if (equal) {
		CcPrintfIL(output, "}");
	    } else {
		err = CcSyntax_AltError(syntax, self->curSy);
		if (useSwitch) {
		    CcPrintfIL(output,
			       "default: %sParser_SynErr(self, %d); break;",
			       self->prefix, err);
		    CcPrintfIL(output, "}");
		} else {
		    CcPrintfIL(output, "} else %sParser_SynErr(self, %d);",
			       self->prefix, err);
		}
	    }
	} else if (p->base.type == node_iter) {
	    p2 = p->sub;
	    if (p2->base.type == node_wt) {
		CcSyntax_Expected(syntax, &s1, p2->next, self->curSy);
		CcSyntax_Expected(syntax, &s2, p->next, self->curSy);
		CcPrintfIL(output,
			   "while (%sParser_WeakSeparator(self, %d, %d, %d)) {",
			   self->prefix, ((CcNodeWT_t *)p2)->sym->kind,
			   CcSyntaxSymSet_New(&self->symSet, &s1),
			   CcSyntaxSymSet_New(&self->symSet, &s2));
		CcBitArray_Destruct(&s1); CcBitArray_Destruct(&s2);
		CcBitArray(&s1, terminals->Count);
		if (p2->up || p2->next == NULL) p2 = NULL; else p2 = p2->next;
	    } else {
		CcSyntax_First(syntax, &s1, p2);
		SCOS_GenCond(self, output, "while (", ") {", &s1, p2);
	    }
	    output->indent += 4;
	    SCOS_GenCode(self, output, p2, &s1);
	    output->indent -= 4;
	    CcPrintfIL(output, "}");
	    CcBitArray_Destruct(&s1);
	} else if (p->base.type == node_opt) {
	    CcSyntax_First(syntax, &s1, p->sub);
	    SCOS_GenCond(self, output, "if (", ") {", &s1, p->sub);
	    output->indent += 4;
	    SCOS_GenCode(self, output, p->sub, &s1);
	    output->indent -= 4;
	    CcPrintfIL(output, "}");
	    CcBitArray_Destruct(&s1);
	}
	if (p->base.type != node_eps && p->base.type != node_sem &&
	    p->base.type != node_sync)
	    CcBitArray_SetAll(&isChecked, FALSE);
	if (p->up) break;
	p = p->next;
    }
    CcBitArray_Destruct(&isChecked);
}

static CcsBool_t
COS_ProductionsBody(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcBitArray_t isChecked;
    CcArrayListIter_t iter;
    const CcSymbolNT_t * sym;
    const CcArrayList_t * nonterminals =
	&self->base.globals->symtab.nonterminals;

    CcBitArray(&isChecked, self->base.globals->symtab.terminals.Count);
    for (sym = (const CcSymbolNT_t *)CcArrayList_FirstC(nonterminals, &iter);
	 sym;
	 sym = (const CcSymbolNT_t *)CcArrayList_NextC(nonterminals, &iter)) {
	self->curSy = (const CcSymbol_t *)sym;
	if (sym->attrPos == NULL) {
	    CcPrintfIL(output, "static void");
	    CcPrintfIL(output, "%sParser_%s(%sParser_t * self)",
		       self->prefix, sym->base.name,
		       self->prefix);
	} else {
	    CcPrintfIL(output, "static void");
	    CcPrintfIL(output, "%sParser_%s(%sParser_t * self, %s)",
		       self->prefix, sym->base.name,
		       self->prefix, sym->attrPos->text);
	}
	CcPrintfIL(output, "{");
	output->indent += 4;
	if (sym->semPos) CcSource(output, sym->semPos);
	SCOS_GenCode(self, output, sym->graph, &isChecked);
	output->indent -= 4;
	CcPrintfIL(output, "}");
	CcPrintfL(output, "");
    }
    CcBitArray_Destruct(&isChecked);
    return TRUE;
}

static CcsBool_t
COS_SynErrors(CcCOutputScheme_t * self, CcOutput_t * output)
{
    CcArrayListIter_t iter; char * str;
    const CcSyntaxError_t * synerr;
    const CcArrayList_t * errors = &self->base.globals->syntax.errors;
    for (synerr = (const CcSyntaxError_t *)CcArrayList_FirstC(errors, &iter);
	 synerr;
	 synerr = (const CcSyntaxError_t *)CcArrayList_NextC(errors, &iter)) {
	CcPrintfI(output, "case %d: s = \"", synerr->base.index);
	str = CcEscape(synerr->sym->name);
	switch (synerr->type) {
	case cet_t:
	    CcPrintf(output, "\\\"\" %s \"\\\" expected", str);
	    break;
	case cet_alt:
	    CcPrintf(output,
		     "this symbol not expected in \\\"\" %s \"\\\"", str);
	    break;
	case cet_sync:
	    CcPrintf(output, "invalid \\\"\" %s \"\\\"", str);
	    break;
	}
	CcFree(str);
	CcPrintfL(output, "\"; break;");
    }
    return TRUE;
}

static CcsBool_t
COS_InitSet(CcCOutputScheme_t * self, CcOutput_t * output)
{
    char * setstr; int setlen, index;
    const CcBitArray_t * cur;

    setlen = self->base.globals->symtab.terminals.Count;
    setstr = CcMalloc(setlen + 1); setstr[setlen] = 0;
    if (setlen > 4) {
	for (index = 0; index < setlen; ++index)
	    if (index == 0) setstr[index] = '*';
	    else setstr[index] = index % 5 == 0 ? '0' + index % 10 : ' ';
	CcPrintfIL(output, "/%s */", setstr);
    }
    for (cur = self->symSet.start; cur < self->symSet.used; ++cur) {
	CcsAssert(setlen == CcBitArray_getCount(cur));
	for (index = 0; index < setlen; ++index)
	    setstr[index] = CcBitArray_Get(cur, index) ? '*' : '.';
	CcPrintfIL(output, "\"%s.\"%c /* %d */", setstr,
		   cur < self->symSet.used - 1 ? ',' : ' ',
		   cur - self->symSet.start);
    }
    CcFree(setstr);
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
    } else if (!strcmp(func, "kindUnknownNS")) {
	return COS_KindUnknownNS(ccself, output);
    } else if (!strcmp(func, "XmlSpecSubLists")) {
	return COS_XmlSpecSubLists(ccself, output);
    } else if (!strcmp(func, "XmlSpecList")) {
	return COS_XmlSpecList(ccself, output);
    } else if (!strcmp(func, "SynDefines")) {
	return COS_SynDefines(ccself, output);
    } else if (!strcmp(func, "members")) {
	return COS_Members(ccself, output);
    } else if (!strcmp(func, "constructor")) {
	return COS_Constructor(ccself, output);
    } else if (!strcmp(func, "destructor")) {
	return COS_Destructor(ccself, output);
    } else if (!strcmp(func, "Pragmas")) {
	return COS_Pragmas(ccself, output);
    } else if (!strcmp(func, "ProductionsHeader")) {
	return COS_ProductionsHeader(ccself, output);
    } else if (!strcmp(func, "ParseRoot")) {
	return COS_ParseRoot(ccself, output);
    } else if (!strcmp(func, "ProductionsBody")) {
	return COS_ProductionsBody(ccself, output);
    } else if (!strcmp(func, "SynErrors")) {
	return COS_SynErrors(ccself, output);
    } else if (!strcmp(func, "InitSet")) {
	return COS_InitSet(ccself, output);
    }
    fprintf(stderr, "Unknown section '%s' encountered.\n", func);
    return TRUE;
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
    /* If the following lists are modified, modify install.py too. */
    "Scanner.h\0Scanner.c\0Parser.h\0Parser.c\0\0",
    "Scanner4Xml.h\0Scanner4Xml.c\0Parser4Xml.h\0Parser4Xml.c\0\0",
    CcCOutputScheme_write
};

CcCOutputScheme_t *
CcCOutputScheme(CcsParser_t * parser, CcsXmlParser_t * xmlparser,
		CcArguments_t * arguments)
{
    CcCOutputScheme_t * self = (CcCOutputScheme_t *)
	CcOutputScheme(&COutputSchemeType,
		       parser ? &parser->globals :
		       xmlparser ? &xmlparser->globals : NULL, arguments);
    self->parser = parser;
    self->xmlparser = xmlparser;
    if (parser) self->prefix = parser->syntax->grammarPrefix;
    else if (xmlparser) self->prefix = xmlparser->syntax->grammarPrefix;
    else { CcsAssert(0); }
    if (self->prefix == NULL) self->prefix = "";
    CcSyntaxSymSet(&self->symSet);
    CcSyntaxSymSet_New(&self->symSet, self->base.globals->syntax.allSyncSets);
    return self;
}
