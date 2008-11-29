/*---- license ----*/
/*---- enable ----*/
#include  <ctype.h>
#include  "Parser.h"
#include  "Token.h"
#include  "CGlobals.h"

/*---- cIncludes ----*/
#include  "Globals.h"
#include  "lexical/CharSet.h"
#include  "lexical/CharClass.h"
#include  "lexical/Nodes.h"
#include  "syntax/Nodes.h"
static const int CcsParser_id = 0;
static const int CcsParser_str = 1;
static const char * noString = "~none~";
/*---- enable ----*/

static void CcsParser_SynErr(CcsParser_t * self, int n);
static const char * set[];

static void
CcsParser_Get(CcsParser_t * self)
{
    for (;;) {
	self->t = self->la;
	self->la = CcsScanner_Scan(self->scanner);
	if (self->la->kind <= self->maxT) { /*++self->errDist;*/ break; }
	/*---- Pragmas ----*/
	if (self->la->kind == 45) {
	    
	}
	/*---- enable ----*/
	self->la = self->t;
    }
}

static CcsBool_t
CcsParser_StartOf(CcsParser_t * self, int s)
{
    return set[s][self->la->kind] == '*';
}

static void
CcsParser_Expect(CcsParser_t * self, int n)
{
    if (self->la->kind == n) CcsParser_Get(self);
    else CcsParser_SynErr(self, n);
}

static void
CcsParser_ExpectWeak(CcsParser_t * self, int n, int follow)
{
    if (self->la->kind == n) CcsParser_Get(self);
    else {
	CcsParser_SynErr(self, n);
	while (!CcsParser_StartOf(self, follow)) CcsParser_Get(self);
    }
}

static CcsBool_t
CcsParser_WeakSeparator(CcsParser_t * self, int n, int syFol, int repFol)
{
    if (self->la->kind == n) { CcsParser_Get(self); return TRUE; }
    else if (CcsParser_StartOf(self, repFol)) { return FALSE; }
    CcsParser_SynErr(self, n);
    while (!(CcsParser_StartOf(self, syFol) ||
	     CcsParser_StartOf(self, repFol) ||
	     CcsParser_StartOf(self, 0)))
	CcsParser_Get(self);
    return CcsParser_StartOf(self, syFol);
}

/*---- ProductionsHeader ----*/
static void CcsParser_Coco(CcsParser_t * self);
static void CcsParser_SetDecl(CcsParser_t * self);
static void CcsParser_TokenDecl(CcsParser_t * self, const CcObjectType_t * typ);
static void CcsParser_TokenExpr(CcsParser_t * self, CcGraph_t ** g);
static void CcsParser_Set(CcsParser_t * self, CcCharSet_t ** s);
static void CcsParser_AttrDecl(CcsParser_t * self, CcSymbolNT_t * sym);
static void CcsParser_SemText(CcsParser_t * self, CcsPosition_t ** pos);
static void CcsParser_Expression(CcsParser_t * self, CcGraph_t ** g);
static void CcsParser_SimSet(CcsParser_t * self, CcCharSet_t ** s);
static void CcsParser_Char(CcsParser_t * self, int * n);
static void CcsParser_Sym(CcsParser_t * self, char ** name, int * kind);
static void CcsParser_Term(CcsParser_t * self, CcGraph_t ** g);
static void CcsParser_Resolver(CcsParser_t * self, CcsPosition_t ** pos);
static void CcsParser_Factor(CcsParser_t * self, CcGraph_t ** g);
static void CcsParser_Attribs(CcsParser_t * self, CcNode_t * p);
static void CcsParser_Condition(CcsParser_t * self);
static void CcsParser_TokenTerm(CcsParser_t * self, CcGraph_t ** g);
static void CcsParser_TokenFactor(CcsParser_t * self, CcGraph_t ** g);
/*---- enable ----*/

void
CcsParser_Parse(CcsParser_t * self)
{
    self->t = NULL;
    self->la = CcsScanner_GetDummy(self->scanner);
    CcsParser_Get(self);
    /*---- ParseRoot ----*/
    CcsParser_Coco(self);
    /*---- enable ----*/
    CcsParser_Expect(self, 0);
}

CcsParser_t *
CcsParser(CcsParser_t * self, CcsGlobals_t * globals)
{
    self->globals = globals;
    self->scanner = &globals->scanner;
    self->t = self->la = NULL;
    /*---- constructor ----*/
    self->maxT = 44;
    self->tokenString = NULL;
    self->genScanner = FALSE;
    self->hIncludes = NULL;
    self->cIncludes = NULL;
    self->members = NULL;
    self->constructor = NULL;
    self->destructor = NULL;
    self->symtab = &((CcGlobals_t *)globals)->symtab;
    self->lexical = &((CcGlobals_t *)globals)->lexical;
    self->syntax = &((CcGlobals_t *)globals)->syntax;
    /*---- enable ----*/
    return self;
}

void
CcsParser_Destruct(CcsParser_t * self)
{
    /*---- destructor ----*/
    if (self->destructor) CcsPosition_Destruct(self->destructor);
    if (self->constructor) CcsPosition_Destruct(self->constructor);
    if (self->members) CcsPosition_Destruct(self->members);
    if (self->cIncludes) CcsPosition_Destruct(self->cIncludes);
    if (self->hIncludes) CcsPosition_Destruct(self->hIncludes);
    if (self->tokenString && self->tokenString != noString)
	CcsFree(self->tokenString);
    /*---- enable ----*/
}

/*---- ProductionsBody ----*/
static void
CcsParser_Coco(CcsParser_t * self)
{
    CcSymbol_t  * sym;
    CcGraph_t   * g, * g1, * g2;
    char        * gramName = NULL;
    CcCharSet_t * s; 
    self->tokenString = NULL; 
    CcsToken_t * beg;
    CcsScanner_IncRef(self->scanner, beg = self->la); 
    while (CcsParser_StartOf(self, 1)) {
	CcsParser_Get(self);
    }
    if (self->la->pos != beg->pos) {
    } 
    CcsParser_Expect(self, 6);
    self->genScanner = TRUE; 
    CcsParser_Expect(self, 1);
    gramName = CcStrdup(self->t->val);
    CcsScanner_DecRef(self->scanner, beg); 
    if (self->la->kind == 7) {
	CcsParser_Get(self);
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 2)) {
	    CcsParser_Get(self);
	}
	self->members = CcsScanner_GetPosition(self->scanner, beg, self->la);
	CcsScanner_DecRef(self->scanner, beg); 
    }
    if (self->la->kind == 8) {
	CcsParser_Get(self);
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 3)) {
	    CcsParser_Get(self);
	}
	self->constructor = CcsScanner_GetPosition(self->scanner, beg, self->la);
			  CcsScanner_DecRef(self->scanner, beg); 
    }
    if (self->la->kind == 9) {
	CcsParser_Get(self);
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 4)) {
	    CcsParser_Get(self);
	}
	self->destructor = CcsScanner_GetPosition(self->scanner, beg, self->la);
			  CcsScanner_DecRef(self->scanner, beg); 
    }
    if (self->la->kind == 10) {
	CcsParser_Get(self);
	self->lexical->ignoreCase = TRUE; 
    }
    if (self->la->kind == 11) {
	CcsParser_Get(self);
	while (self->la->kind == 1) {
	    CcsParser_SetDecl(self);
	}
    }
    if (self->la->kind == 12) {
	CcsParser_Get(self);
	while (self->la->kind == 1 || self->la->kind == 3 || self->la->kind == 5) {
	    CcsParser_TokenDecl(self, symbol_t);
	}
    }
    if (self->la->kind == 13) {
	CcsParser_Get(self);
	while (self->la->kind == 1 || self->la->kind == 3 || self->la->kind == 5) {
	    CcsParser_TokenDecl(self, symbol_pr);
	}
    }
    while (self->la->kind == 14) {
	CcsParser_Get(self);
	CcsBool_t nested = FALSE; 
	CcsParser_Expect(self, 15);
	CcsParser_TokenExpr(self, &g1);
	CcsParser_Expect(self, 16);
	CcsParser_TokenExpr(self, &g2);
	if (self->la->kind == 17) {
	    CcsParser_Get(self);
	    nested = TRUE; 
	}
	CcLexical_NewComment(self->lexical, self->t,
			     g1->head, g2->head, nested);
	CcGraph_Destruct(g1); CcGraph_Destruct(g2); 
    }
    while (self->la->kind == 18) {
	CcsParser_Get(self);
	CcsParser_Set(self, &s);
	CcCharSet_Or(self->lexical->ignored, s);
	CcCharSet_Destruct(s); 
    }
    while (!(self->la->kind == 0 || self->la->kind == 19)) {
	CcsParser_SynErr(self, 45); CcsParser_Get(self);
    }
    CcsParser_Expect(self, 19);
    if (self->genScanner) CcLexical_MakeDeterministic(self->lexical);
    CcEBNF_Clear(&self->lexical->base);
    CcEBNF_Clear(&self->syntax->base); 
    while (self->la->kind == 1) {
	CcsParser_Get(self);
	sym = CcSymbolTable_FindSym(self->symtab, self->t->val);
	CcsBool_t undef = (sym == NULL);
	if (undef) {
	    sym = CcSymbolTable_NewNonTerminal(self->symtab,
					       self->t->val, self->t->line);
	} else {
	    if (sym->base.type == symbol_nt) {
		if (((CcSymbolNT_t *)sym)->graph != NULL)
		    CcsGlobals_SemErr(self->globals, self->t, "name declared twice");
	    } else {
		CcsGlobals_SemErr(self->globals, self->t,
				  "this symbol kind not allowed on left side of production");
	    }
	    sym->line = self->t->line;
	}
	CcsAssert(sym->base.type == symbol_nt);
	CcsBool_t noAttrs = (((CcSymbolNT_t *)sym)->attrPos == NULL);
	((CcSymbolNT_t *)sym)->attrPos = NULL; 
	if (self->la->kind == 27 || self->la->kind == 29) {
	    CcsParser_AttrDecl(self, (CcSymbolNT_t *)sym);
	}
	if (!undef && noAttrs != (((CcSymbolNT_t *)sym)->attrPos == NULL))
	    CcsGlobals_SemErr(self->globals, self->t,
			      "attribute mismatch between declaration and use of this symbol"); 
	if (self->la->kind == 42) {
	    CcsParser_SemText(self, &((CcSymbolNT_t *)sym)->semPos);
	}
	CcsParser_ExpectWeak(self, 20, 5);
	CcsParser_Expression(self, &g);
	((CcSymbolNT_t *)sym)->graph = g->head;
	CcGraph_Finish(g);
	CcGraph_Destruct(g); 
	CcsParser_ExpectWeak(self, 21, 6);
    }
    CcsParser_Expect(self, 22);
    CcsParser_Expect(self, 1);
    if (strcmp(gramName, self->t->val))
	CcsGlobals_SemErr(self->globals, self->t,
			  "name does not match grammar name");
    self->syntax->gramSy = CcSymbolTable_FindSym(self->symtab, gramName);
    CcFree(gramName);
    if (self->syntax->gramSy == NULL) {
	CcsGlobals_SemErr(self->globals, self->t,
			  "missing production for grammar name");
    } else {
	sym = self->syntax->gramSy;
	if (((CcSymbolNT_t *)sym)->attrPos != NULL)
	    CcsGlobals_SemErr(self->globals, self->t,
			      "grammar symbol must not have attributes");
    }
    /* noSym gets highest number */
    self->syntax->noSy = CcSymbolTable_NewTerminal(self->symtab, "???", 0);
    CcSyntax_SetupAnys(self->syntax); 
    CcsParser_Expect(self, 21);
}

static void
CcsParser_SetDecl(CcsParser_t * self)
{
    CcCharSet_t * s; 
    CcsParser_Expect(self, 1);
    const char * name = self->t->val;
    CcCharClass_t * c = CcLexical_FindCharClassN(self->lexical, name);
    if (c != NULL)
	CcsGlobals_SemErr(self->globals, self->t,
			  "name '%s' declared twice", name); 
    CcsParser_Expect(self, 20);
    CcsParser_Set(self, &s);
    if (CcCharSet_Elements(s) == 0)
	CcsGlobals_SemErr(self->globals, self->t,
			  "character set must not be empty");
    CcLexical_NewCharClass(self->lexical, name, s); 
    CcsParser_Expect(self, 21);
}

static void
CcsParser_TokenDecl(CcsParser_t * self, const CcObjectType_t * typ)
{
    char * name = NULL; int kind; CcSymbol_t * sym; CcGraph_t * g; 
    CcsParser_Sym(self, &name, &kind);
    sym = CcSymbolTable_FindSym(self->symtab, name);
    if (sym != NULL) {
	CcsGlobals_SemErr(self->globals, self->t, "name '%s' declared twice", name);
    } else if (typ == symbol_t) {
	sym = CcSymbolTable_NewTerminal(self->symtab, name, self->t->line);
	((CcSymbolT_t *)sym)->tokenKind = symbol_fixedToken;
    } else if (typ == symbol_pr) {
	sym = CcSymbolTable_NewPragma(self->symtab, name, self->t->line);
	((CcSymbolPR_t *)sym)->tokenKind = symbol_fixedToken;
	((CcSymbolPR_t *)sym)->semPos = NULL;
    }
    if (self->tokenString && self->tokenString != noString)
	CcFree(self->tokenString);
    self->tokenString = NULL;
    CcFree(name); 
    while (!(CcsParser_StartOf(self, 0))) {
	CcsParser_SynErr(self, 46); CcsParser_Get(self);
    }
    if (self->la->kind == 20) {
	CcsParser_Get(self);
	CcsParser_TokenExpr(self, &g);
	CcsParser_Expect(self, 21);
	if (kind == CcsParser_str)
	    CcsGlobals_SemErr(self->globals, self->t,
			      "a literal must not be declared with a structure");
	CcGraph_Finish(g);
	if (self->tokenString == NULL || self->tokenString == noString) {
	    CcLexical_ConvertToStates(self->lexical, g->head, sym);
	} else { /* CcsParser_TokenExpr is a single string */
	    if (CcHashTable_Get(&self->lexical->literals,
				self->tokenString) != NULL)
		CcsGlobals_SemErr(self->globals, self->t,
				  "token string '%s' declared twice", self->tokenString);
	    CcHashTable_Set(&self->lexical->literals,
			    self->tokenString, (CcObject_t *)sym);
	    CcLexical_MatchLiteral(self->lexical, self->t,
				   self->tokenString, sym);
	    CcFree(self->tokenString);
	}
	self->tokenString = NULL;
	CcGraph_Destruct(g); 
    } else if (CcsParser_StartOf(self, 7)) {
	if (kind == CcsParser_id) self->genScanner = FALSE;
	else CcLexical_MatchLiteral(self->lexical, self->t, sym->name, sym); 
    } else CcsParser_SynErr(self, 47);
    if (self->la->kind == 42) {
	CcsParser_SemText(self, &((CcSymbolPR_t *)sym)->semPos);
	if (typ != symbol_pr)
	    CcsGlobals_SemErr(self->globals, self->t,
			      "semantic action not allowed here"); 
    }
}

static void
CcsParser_TokenExpr(CcsParser_t * self, CcGraph_t ** g)
{
    CcGraph_t * g2; 
    CcsParser_TokenTerm(self, g);
    CcsBool_t first = TRUE; 
    while (CcsParser_WeakSeparator(self, 31, 9, 8)) {
	CcsParser_TokenTerm(self, &g2);
	if (first) { CcEBNF_MakeFirstAlt(&self->lexical->base, *g); first = FALSE; }
	CcEBNF_MakeAlternative(&self->lexical->base, *g, g2);
	CcGraph_Destruct(g2); 
    }
}

static void
CcsParser_Set(CcsParser_t * self, CcCharSet_t ** s)
{
    CcCharSet_t * s2; 
    CcsParser_SimSet(self, s);
    while (self->la->kind == 23 || self->la->kind == 24) {
	if (self->la->kind == 23) {
	    CcsParser_Get(self);
	    CcsParser_SimSet(self, &s2);
	    CcCharSet_Or(*s, s2);
	    CcCharSet_Destruct(s2); 
	} else {
	    CcsParser_Get(self);
	    CcsParser_SimSet(self, &s2);
	    CcCharSet_Subtract(*s, s2);
	    CcCharSet_Destruct(s2); 
	}
    }
}

static void
CcsParser_AttrDecl(CcsParser_t * self, CcSymbolNT_t * sym)
{
    if (self->la->kind == 27) {
	CcsParser_Get(self);
	CcsToken_t * beg;
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 10)) {
	    if (CcsParser_StartOf(self, 11)) {
		CcsParser_Get(self);
	    } else {
		CcsParser_Get(self);
		CcsGlobals_SemErr(self->globals, self->t,
				  "bad string in attributes"); 
	    }
	}
	CcsParser_Expect(self, 28);
	sym->attrPos = CcsScanner_GetPosition(self->scanner, beg, self->t);
	CcsScanner_DecRef(self->scanner, beg); 
    } else if (self->la->kind == 29) {
	CcsParser_Get(self);
	CcsToken_t * beg;
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 12)) {
	    if (CcsParser_StartOf(self, 13)) {
		CcsParser_Get(self);
	    } else {
		CcsParser_Get(self);
		CcsGlobals_SemErr(self->globals, self->t,
				  "bad string in attributes"); 
	    }
	}
	CcsParser_Expect(self, 30);
	sym->attrPos = CcsScanner_GetPosition(self->scanner, beg, self->t);
	CcsScanner_DecRef(self->scanner, beg); 
    } else CcsParser_SynErr(self, 48);
}

static void
CcsParser_SemText(CcsParser_t * self, CcsPosition_t ** pos)
{
    CcsParser_Expect(self, 42);
    CcsToken_t * beg;
    CcsScanner_IncRef(self->scanner, beg = self->la); 
    while (CcsParser_StartOf(self, 14)) {
	if (CcsParser_StartOf(self, 15)) {
	    CcsParser_Get(self);
	} else if (self->la->kind == 4) {
	    CcsParser_Get(self);
	    CcsGlobals_SemErr(self->globals, self->t,
			      "bad string in semantic action"); 
	} else {
	    CcsParser_Get(self);
	    CcsGlobals_SemErr(self->globals, self->t,
			      "missing end of previous semantic action"); 
	}
    }
    CcsParser_Expect(self, 43);
    *pos = CcsScanner_GetPosition(self->scanner, beg, self->t);
    CcsScanner_DecRef(self->scanner, beg); 
}

static void
CcsParser_Expression(CcsParser_t * self, CcGraph_t ** g)
{
    CcGraph_t * g2; 
    CcsParser_Term(self, g);
    CcsBool_t first = TRUE; 
    while (CcsParser_WeakSeparator(self, 31, 17, 16)) {
	CcsParser_Term(self, &g2);
	if (first) { CcEBNF_MakeFirstAlt(&self->syntax->base, *g); first = FALSE; }
	CcEBNF_MakeAlternative(&self->syntax->base, *g, g2);
	CcGraph_Destruct(g2); 
    }
}

static void
CcsParser_SimSet(CcsParser_t * self, CcCharSet_t ** s)
{
    int n1, n2; 
    *s = CcCharSet(); 
    if (self->la->kind == 1) {
	CcsParser_Get(self);
	CcCharClass_t * c = CcLexical_FindCharClassN(self->lexical, self->t->val);
	if (c != NULL) CcCharSet_Or(*s, c->set);
	else CcsGlobals_SemErr(self->globals, self->t, "undefined name"); 
    } else if (self->la->kind == 3) {
	CcsParser_Get(self);
	const char * cur0; int ch;
	char * cur, * name = CcUnescape(self->t->val);
	if (self->lexical->ignoreCase) {
	    for (cur = name; *cur; ++cur) *cur = tolower(*cur);
	}
	cur0 = name;
	while (*cur0) {
	    ch = CcsUTF8GetCh(&cur0, name + strlen(name));
	    CcsAssert(ch >= 0);
	    CcCharSet_Set(*s, ch);
	}
	CcFree(name); 
    } else if (self->la->kind == 5) {
	CcsParser_Char(self, &n1);
	CcCharSet_Set(*s, n1); 
	if (self->la->kind == 25) {
	    CcsParser_Get(self);
	    CcsParser_Char(self, &n2);
	    int idx;
	    for (idx = n1; idx <= n2; ++idx) CcCharSet_Set(*s, idx); 
	}
    } else if (self->la->kind == 26) {
	CcsParser_Get(self);
	CcCharSet_Fill(*s, COCO_WCHAR_MAX); 
    } else CcsParser_SynErr(self, 49);
}

static void
CcsParser_Char(CcsParser_t * self, int * n)
{
    char * name; const char * cur; 
    CcsParser_Expect(self, 5);
    *n = 0;
    cur = name = CcUnescape(self->t->val);
    *n = CcsUTF8GetCh(&cur, name + strlen(name));
    if (*cur != 0)
	CcsGlobals_SemErr(self->globals, self->t,
	    "unacceptable character value: '%s'", self->t->val);
    CcFree(name);
    if (self->lexical->ignoreCase) *n = tolower(*n); 
}

static void
CcsParser_Sym(CcsParser_t * self, char ** name, int * kind)
{
    *name = CcStrdup("???"); *kind = CcsParser_id; 
    if (self->la->kind == 1) {
	CcsParser_Get(self);
	*kind = CcsParser_id; CcFree(*name); *name = CcStrdup(self->t->val); 
    } else if (self->la->kind == 3 || self->la->kind == 5) {
	if (self->la->kind == 3) {
	    CcsParser_Get(self);
	    CcFree(*name); *name = CcStrdup(self->t->val); 
	} else {
	    CcsParser_Get(self);
	    CcFree(*name); *name = CcStrdup(self->t->val); 
	}
	*kind = CcsParser_str;
	if (self->lexical->ignoreCase) {
	    char * cur;
	    for (cur = *name; *cur; ++cur) *cur = tolower(*cur);
	}
	if (strchr(*name, ' '))
	    CcsGlobals_SemErr(self->globals, self->t,
			      "literal tokens \"%s\" can not contain blanks", *name); 
    } else CcsParser_SynErr(self, 50);
}

static void
CcsParser_Term(CcsParser_t * self, CcGraph_t ** g)
{
    CcGraph_t * g2; CcsPosition_t * pos; CcNode_t * rslv = NULL;
    *g = NULL; 
    if (CcsParser_StartOf(self, 18)) {
	if (self->la->kind == 40) {
	    CcsParser_Resolver(self, &pos);
	    rslv = CcEBNF_NewNode(&self->syntax->base,
				  CcNodeRslvP(self->la->line, pos));
	    *g = CcGraphP(rslv); 
	}
	CcsParser_Factor(self, &g2);
	if (rslv == NULL) *g = g2;
	else {
	    CcEBNF_MakeSequence(&self->syntax->base, *g, g2);
	    CcGraph_Destruct(g2);
	} 
	while (CcsParser_StartOf(self, 19)) {
	    CcsParser_Factor(self, &g2);
	    CcEBNF_MakeSequence(&self->syntax->base, *g, g2);
	    CcGraph_Destruct(g2); 
	}
    } else if (CcsParser_StartOf(self, 20)) {
	*g = CcGraphP(CcEBNF_NewNode(&self->syntax->base, CcNodeEps(0))); 
    } else CcsParser_SynErr(self, 51);
    if (*g == NULL) /* invalid start of Term */
	*g = CcGraphP(CcEBNF_NewNode(&self->syntax->base, CcNodeEps(0))); 
}

static void
CcsParser_Resolver(CcsParser_t * self, CcsPosition_t ** pos)
{
    CcsParser_Expect(self, 40);
    CcsParser_Expect(self, 33);
    CcsToken_t * beg;
    CcsScanner_IncRef(self->scanner, beg = self->la); 
    CcsParser_Condition(self);
    *pos = CcsScanner_GetPosition(self->scanner, beg, self->t);
    CcsScanner_DecRef(self->scanner, beg); 
}

static void
CcsParser_Factor(CcsParser_t * self, CcGraph_t ** g)
{
    char * name = NULL; int kind; CcsPosition_t * pos; CcsBool_t weak = FALSE; 
    *g = NULL; 
    switch (self->la->kind) {
    case 1: case 3: case 5: case 32: {
	if (self->la->kind == 32) {
	    CcsParser_Get(self);
	    weak = TRUE; 
	}
	CcsParser_Sym(self, &name, &kind);
	CcSymbol_t * sym = CcSymbolTable_FindSym(self->symtab, name);
	if (sym == NULL && kind == CcsParser_str)
	    sym = (CcSymbol_t *)CcHashTable_Get(&self->lexical->literals, name);
	CcsBool_t undef = (sym == NULL);
	if (undef) {
	    if (kind == CcsParser_id) {
		/* forward nt */
		sym = CcSymbolTable_NewNonTerminal(self->symtab, name, 0);
	    } else if (self->genScanner) {
		sym = CcSymbolTable_NewTerminal(self->symtab, name, self->t->line);
		CcLexical_MatchLiteral(self->lexical, self->t, sym->name, sym);
	    } else {  /* undefined string in production */
		CcsGlobals_SemErr(self->globals, self->t,
				  "undefined string in production");
		sym = self->syntax->eofSy;  /* dummy */
	    }
	}
	CcFree(name);
	if (sym->base.type != symbol_t && sym->base.type != symbol_nt)
	    CcsGlobals_SemErr(self->globals, self->t,
			      "this symbol kind is not allowed in a production");
	if (weak) {
	    if (sym->base.type != symbol_t)
		CcsGlobals_SemErr(self->globals, self->t,
				  "only terminals may be weak");
	}
	CcNode_t * p = CcSyntax_NodeFromSymbol(self->syntax, sym, self->t->line, weak);
	*g = CcGraphP(p); 
	if (self->la->kind == 27 || self->la->kind == 29) {
	    CcsParser_Attribs(self, p);
	    if (kind != CcsParser_id)
		CcsGlobals_SemErr(self->globals, self->t,
				  "a literal must not have attributes"); 
	}
	if (undef) {
	    if (sym->base.type == symbol_nt)
		((CcSymbolNT_t *)sym)->attrPos = ((CcNodeNT_t *)p)->pos; /* dummy */
	} else if (sym->base.type == symbol_nt &&
		   (((CcNodeNT_t *)p)->pos == NULL) !=
		   (((CcSymbolNT_t *)sym)->attrPos == NULL))
	    CcsGlobals_SemErr(self->globals, self->t,
			      "attribute mismatch between declaration and use of this symbol"); 
	break;
    }
    case 33: {
	CcsParser_Get(self);
	CcsParser_Expression(self, g);
	CcsParser_Expect(self, 34);
	break;
    }
    case 35: {
	CcsParser_Get(self);
	CcsParser_Expression(self, g);
	CcsParser_Expect(self, 36);
	CcEBNF_MakeOption(&self->syntax->base, *g); 
	break;
    }
    case 37: {
	CcsParser_Get(self);
	CcsParser_Expression(self, g);
	CcsParser_Expect(self, 38);
	CcEBNF_MakeIteration(&self->syntax->base, *g); 
	break;
    }
    case 42: {
	CcsParser_SemText(self, &pos);
	CcNode_t * p = CcEBNF_NewNode(&self->syntax->base, CcNodeSem(0));
	((CcNodeSEM_t *)p)->pos = pos;
	*g = CcGraphP(p); 
	break;
    }
    case 26: {
	CcsParser_Get(self);
	CcNode_t * p = CcEBNF_NewNode(&self->syntax->base, CcNodeAny(0));
	*g = CcGraphP(p); 
	break;
    }
    case 39: {
	CcsParser_Get(self);
	CcNode_t * p = CcEBNF_NewNode(&self->syntax->base, CcNodeSync(0));
	*g = CcGraphP(p);
	break;
    }
    default: CcsParser_SynErr(self, 52); break;
    }
    if (*g == NULL) /* invalid start of Factor */
	*g = CcGraphP(CcEBNF_NewNode(&self->syntax->base, CcNodeEps(0))); 
}

static void
CcsParser_Attribs(CcsParser_t * self, CcNode_t * p)
{
    if (self->la->kind == 27) {
	CcsParser_Get(self);
	CcsToken_t * beg;
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 10)) {
	    if (CcsParser_StartOf(self, 11)) {
		CcsParser_Get(self);
	    } else {
		CcsParser_Get(self);
		CcsGlobals_SemErr(self->globals, self->t,
				  "bad string in attributes"); 
	    }
	}
	CcsParser_Expect(self, 28);
	CcNode_SetPosition(p, CcsScanner_GetPosition(self->scanner,
						     beg, self->t));
	CcsScanner_DecRef(self->scanner, beg); 
    } else if (self->la->kind == 29) {
	CcsParser_Get(self);
	CcsToken_t * beg;
	CcsScanner_IncRef(self->scanner, beg = self->la); 
	while (CcsParser_StartOf(self, 12)) {
	    if (CcsParser_StartOf(self, 13)) {
		CcsParser_Get(self);
	    } else {
		CcsParser_Get(self);
		CcsGlobals_SemErr(self->globals, self->t,
				  "bad string in attributes"); 
	    }
	}
	CcsParser_Expect(self, 30);
	CcNode_SetPosition(p, CcsScanner_GetPosition(self->scanner,
						     beg, self->t));
	CcsScanner_DecRef(self->scanner, beg); 
    } else CcsParser_SynErr(self, 53);
}

static void
CcsParser_Condition(CcsParser_t * self)
{
    while (CcsParser_StartOf(self, 21)) {
	if (self->la->kind == 33) {
	    CcsParser_Get(self);
	    CcsParser_Condition(self);
	} else {
	    CcsParser_Get(self);
	}
    }
    CcsParser_Expect(self, 34);
}

static void
CcsParser_TokenTerm(CcsParser_t * self, CcGraph_t ** g)
{
    CcGraph_t * g2; 
    CcsParser_TokenFactor(self, g);
    while (CcsParser_StartOf(self, 9)) {
	CcsParser_TokenFactor(self, &g2);
	CcEBNF_MakeSequence(&self->lexical->base, *g, g2);
	CcGraph_Destruct(g2); 
    }
    if (self->la->kind == 41) {
	CcsParser_Get(self);
	CcsParser_Expect(self, 33);
	CcsParser_TokenExpr(self, &g2);
	CcLexical_SetContextTrans(self->lexical, g2->head);
	self->lexical->hasCtxMoves = TRUE;
	CcEBNF_MakeSequence(&self->lexical->base, *g, g2); 
	CcsParser_Expect(self, 34);
    }
}

static void
CcsParser_TokenFactor(CcsParser_t * self, CcGraph_t ** g)
{
    char * name = NULL; int kind; CcTransition_t trans; 
    *g = NULL; 
    if (self->la->kind == 1 || self->la->kind == 3 || self->la->kind == 5) {
	CcsParser_Sym(self, &name, &kind);
	if (kind == CcsParser_id) {
	    CcCharClass_t * c = CcLexical_FindCharClassN(self->lexical, name);
	    if (c == NULL) {
		CcsGlobals_SemErr(self->globals, self->t, "undefined name");
		c = CcLexical_NewCharClass(self->lexical, name, CcCharSet());
	    }
	    CcTransition_FromCharSet(&trans, c->set, trans_normal,
				     &self->lexical->classes);
	    *g = CcGraphP(CcEBNF_NewNode(&self->lexical->base,
					 CcNodeTrans(0, &trans)));
	    CcTransition_Destruct(&trans);
	    if (self->tokenString && self->tokenString != noString)
		CcFree(self->tokenString);
	    self->tokenString = (char *)noString;
	} else { /* CcsParser_str */
	    *g = CcLexical_StrToGraph(self->lexical, name, self->t);
	    if (self->tokenString == NULL) self->tokenString = CcStrdup(name);
	    else {
		if (self->tokenString != noString) CcFree(self->tokenString);
		self->tokenString = (char *)noString;
	    }
	}
	CcFree(name); 
    } else if (self->la->kind == 33) {
	CcsParser_Get(self);
	CcsParser_TokenExpr(self, g);
	CcsParser_Expect(self, 34);
    } else if (self->la->kind == 35) {
	CcsParser_Get(self);
	CcsParser_TokenExpr(self, g);
	CcsParser_Expect(self, 36);
	CcEBNF_MakeOption(&self->lexical->base, *g); 
    } else if (self->la->kind == 37) {
	CcsParser_Get(self);
	CcsParser_TokenExpr(self, g);
	CcsParser_Expect(self, 38);
	CcEBNF_MakeIteration(&self->lexical->base, *g); 
    } else CcsParser_SynErr(self, 54);
    if (*g == NULL) /* invalid start of TokenFactor */
      *g = CcGraphP(CcEBNF_NewNode(&self->lexical->base, CcNodeEps(0))); 
}

/*---- enable ----*/

static void
CcsParser_SynErr(CcsParser_t * self, int n)
{
    const char * s; char format[20];
    switch (n) {
    /*---- SynErrors ----*/
    case 0: s = "\"" "EOF" "\" expected"; break;
    case 1: s = "\"" "ident" "\" expected"; break;
    case 2: s = "\"" "number" "\" expected"; break;
    case 3: s = "\"" "string" "\" expected"; break;
    case 4: s = "\"" "badString" "\" expected"; break;
    case 5: s = "\"" "char" "\" expected"; break;
    case 6: s = "\"" "COMPILER" "\" expected"; break;
    case 7: s = "\"" "MEMBERS" "\" expected"; break;
    case 8: s = "\"" "CONSTRUCTOR" "\" expected"; break;
    case 9: s = "\"" "DESTRUCTOR" "\" expected"; break;
    case 10: s = "\"" "IGNORECASE" "\" expected"; break;
    case 11: s = "\"" "CHARACTERS" "\" expected"; break;
    case 12: s = "\"" "TOKENS" "\" expected"; break;
    case 13: s = "\"" "PRAGMAS" "\" expected"; break;
    case 14: s = "\"" "COMMENTS" "\" expected"; break;
    case 15: s = "\"" "FROM" "\" expected"; break;
    case 16: s = "\"" "TO" "\" expected"; break;
    case 17: s = "\"" "NESTED" "\" expected"; break;
    case 18: s = "\"" "IGNORE" "\" expected"; break;
    case 19: s = "\"" "PRODUCTIONS" "\" expected"; break;
    case 20: s = "\"" "=" "\" expected"; break;
    case 21: s = "\"" "." "\" expected"; break;
    case 22: s = "\"" "END" "\" expected"; break;
    case 23: s = "\"" "+" "\" expected"; break;
    case 24: s = "\"" "-" "\" expected"; break;
    case 25: s = "\"" ".." "\" expected"; break;
    case 26: s = "\"" "ANY" "\" expected"; break;
    case 27: s = "\"" "<" "\" expected"; break;
    case 28: s = "\"" ">" "\" expected"; break;
    case 29: s = "\"" "<." "\" expected"; break;
    case 30: s = "\"" ".>" "\" expected"; break;
    case 31: s = "\"" "|" "\" expected"; break;
    case 32: s = "\"" "WEAK" "\" expected"; break;
    case 33: s = "\"" "(" "\" expected"; break;
    case 34: s = "\"" ")" "\" expected"; break;
    case 35: s = "\"" "[" "\" expected"; break;
    case 36: s = "\"" "]" "\" expected"; break;
    case 37: s = "\"" "{" "\" expected"; break;
    case 38: s = "\"" "}" "\" expected"; break;
    case 39: s = "\"" "SYNC" "\" expected"; break;
    case 40: s = "\"" "IF" "\" expected"; break;
    case 41: s = "\"" "CONTEXT" "\" expected"; break;
    case 42: s = "\"" "(." "\" expected"; break;
    case 43: s = "\"" ".)" "\" expected"; break;
    case 44: s = "\"" "???" "\" expected"; break;
    case 45: s = "invalid \"" "Coco" "\""; break;
    case 46: s = "invalid \"" "TokenDecl" "\""; break;
    case 47: s = "this symbol not expected in \"" "TokenDecl" "\""; break;
    case 48: s = "this symbol not expected in \"" "AttrDecl" "\""; break;
    case 49: s = "this symbol not expected in \"" "SimSet" "\""; break;
    case 50: s = "this symbol not expected in \"" "Sym" "\""; break;
    case 51: s = "this symbol not expected in \"" "Term" "\""; break;
    case 52: s = "this symbol not expected in \"" "Factor" "\""; break;
    case 53: s = "this symbol not expected in \"" "Attribs" "\""; break;
    case 54: s = "this symbol not expected in \"" "TokenFactor" "\""; break;
    /*---- enable ----*/
    default:
	snprintf(format, sizeof(format), "error %d", n);
	s = format;
	break;
    }
    CcsGlobals_SemErr(self->globals, self->la, "%s", s);
}

static const char * set[] = {
    /*---- InitSet ----*/
    /*    5    0    5    0    5    0    5    0     */
    "**.*.*.......**...***.....................*...", /* 0 */
    ".*****.**************************************.", /* 1 */
    ".*******.......***..*************************.", /* 2 */
    ".********......***..*************************.", /* 3 */
    ".*********.....***..*************************.", /* 4 */
    "**.*.*.......**...****....*....***.*.*.**.*...", /* 5 */
    "**.*.*.......**...***.*...................*...", /* 6 */
    ".*.*.*.......**...**......................*...", /* 7 */
    "..............*.****.*............*.*.*.......", /* 8 */
    ".*.*.*...........................*.*.*........", /* 9 */
    ".***************************.****************.", /* 10 */
    ".***.***********************.****************.", /* 11 */
    ".*****************************.**************.", /* 12 */
    ".***.*************************.**************.", /* 13 */
    ".******************************************.*.", /* 14 */
    ".***.*************************************..*.", /* 15 */
    ".....................*............*.*.*.......", /* 16 */
    ".*.*.*...............*....*....**********.*...", /* 17 */
    ".*.*.*....................*.....**.*.*.**.*...", /* 18 */
    ".*.*.*....................*.....**.*.*.*..*...", /* 19 */
    ".....................*.........*..*.*.*.......", /* 20 */
    ".*********************************.**********."  /* 21 */
    /*---- enable ----*/
};
