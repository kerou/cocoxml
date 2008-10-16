

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"




void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }
		if (la->kind == 34) {
				tab->SetDDT(la->val); 
		}

		if (dummyToken != t) {
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::CocoXml() {
		Symbol *sym; Graph *g; wchar_t* gramName = NULL; XmlLangDefinition *xldef; 
		InitDeclarations(); 
		int beg = la->pos; 
		while (StartOf(1)) {
			Get();
		}
		if (la->pos != beg) {
		 pgen->usingPos = new Position(beg, t->pos - beg + coco_string_length(t->val), 0);
		}
		
		Expect(6);
		genScanner = true; 
		Expect(1);
		gramName = coco_string_create(t->val);
		beg = la->pos; 
		
		while (StartOf(2)) {
			Get();
		}
		tab->semDeclPos = new Position(beg, la->pos-beg, 0); 
		XmlLangDefinition(xldef);
		xsdata->Add("", xldef); 
		while (la->kind == 11) {
			XmlNamespaceDeclaration();
		}
		Expect(7);
		tab->DeleteNodes(); 
		while (la->kind == 1) {
			Get();
			sym = tab->FindSym(t->val);
			bool undef = (sym == NULL);
			if (undef) sym = tab->NewSym(Node::nt, t->val, t->line);
			else {
			  if (sym->typ == Node::nt) {
			    if (sym->graph != NULL) SemErr(L"name declared twice");
				 } else SemErr(L"this symbol kind not allowed on left side of production");
				 sym->line = t->line;
			}
			bool noAttrs = (sym->attrPos == NULL);
			sym->attrPos = NULL;
			
			if (la->kind == 18 || la->kind == 20) {
				AttrDecl(sym);
			}
			if (!undef)
			 if (noAttrs != (sym->attrPos == NULL))
			   SemErr(L"attribute mismatch between declaration and use of this symbol");
			
			if (la->kind == 31) {
				SemText(sym->semPos);
			}
			ExpectWeak(8, 3);
			Expression(g);
			sym->graph = g->l;
			tab->Finish(g);
			
			ExpectWeak(9, 4);
		}
		Expect(10);
		Expect(1);
		if (!coco_string_equal(gramName, t->val))
		 SemErr(L"name does not match grammar name");
		tab->gramSy = tab->FindSym(gramName);
		if (tab->gramSy == NULL)
		  SemErr(L"missing production for grammar name");
		else {
		  sym = tab->gramSy;
		  if (sym->attrPos != NULL)
		    SemErr(L"grammar symbol must not have attributes");
		}
		tab->noSym = tab->NewSym(Node::t, L"???", 0); // noSym gets highest number
		tab->SetupAnys();
		tab->RenumberPragmas();
		if (tab->ddt[2]) tab->PrintNodes();
		if (errors->count == 0) {
		  wprintf(L"checking\n");
		  tab->CompSymbolSets();
		  if (tab->ddt[7]) tab->XRef();
		  if (tab->GrammarOk()) {
		    wprintf(L"parser");
		    pgen->WriteXmlParser();
		    if (genScanner) {
		      wprintf(L" + scanner");
		      xsdata->WriteXmlScanner();
		      /* if (tab->ddt[0]) dfa->PrintStates(); */
		    }
		    wprintf(L" generated\n");
		    if (tab->ddt[8]) pgen->WriteStatistics();
		  }
		}
		if (tab->ddt[6]) tab->PrintSymbolTable();
		
		Expect(9);
}

void Parser::XmlLangDefinition(XmlLangDefinition xldef) {
		xldef = new XmlLangDefinition(tab, errors);  
		if (la->kind == 14) {
			Get();
			while (la->kind == 1) {
				OptionDecl(xldef);
			}
		}
		if (la->kind == 15) {
			Get();
			while (la->kind == 1) {
				XmlTagDecl(xldef);
			}
		}
		if (la->kind == 16) {
			Get();
			while (la->kind == 1) {
				XmlAttrDecl(xldef);
			}
		}
		if (la->kind == 17) {
			Get();
			while (la->kind == 1) {
				ProcessingInstruction(xldef);
			}
		}
}

void Parser::XmlNamespaceDeclaration() {
		string namespace_name;
		         XmlLangDefinition xldef; 
		Expect(11);
		Expect(3);
		namespace_name = t->val; 
		Expect(12);
		XmlLangDefinition(xldef);
		xsdata->Add(namespace_name, xldef); 
		Expect(13);
}

void Parser::AttrDecl(Symbol *sym) {
		if (la->kind == 18) {
			Get();
			int beg = la->pos; int col = la->col; 
			while (StartOf(5)) {
				if (StartOf(6)) {
					Get();
				} else {
					Get();
					SemErr(L"bad string in attributes"); 
				}
			}
			Expect(19);
			if (t->pos > beg)
			 sym->attrPos = new Position(beg, t->pos - beg, col); 
		} else if (la->kind == 20) {
			Get();
			int beg = la->pos; int col = la->col; 
			while (StartOf(7)) {
				if (StartOf(8)) {
					Get();
				} else {
					Get();
					SemErr(L"bad string in attributes"); 
				}
			}
			Expect(21);
			if (t->pos > beg)
			 sym->attrPos = new Position(beg, t->pos - beg, col); 
		} else SynErr(34);
}

void Parser::SemText(Position* &pos) {
		Expect(31);
		int beg = la->pos; int col = la->col; 
		while (StartOf(9)) {
			if (StartOf(10)) {
				Get();
			} else if (la->kind == 4) {
				Get();
				SemErr(L"bad string in semantic action"); 
			} else {
				Get();
				SemErr(L"missing end of previous semantic action"); 
			}
		}
		Expect(32);
		pos = new Position(beg, t->pos - beg, col); 
}

void Parser::Expression(Graph* &g) {
		Graph *g2; 
		Term(g);
		bool first = true; 
		while (WeakSeparator(22,12,11) ) {
			Term(g2);
			if (first) { tab->MakeFirstAlt(g); first = false; }
			tab->MakeAlternative(g, g2);
			
		}
}

void Parser::OptionDecl(XmlLangDefinition *xldef) {
		Expect(1);
		xldef->AddOption(t->val, t->line); 
}

void Parser::XmlTagDecl(XmlLangDefinition *xldef) {
		string tokenName; 
		Expect(1);
		tokenName = t->val; 
		Expect(8);
		Expect(3);
		xldef->AddTag(t->val, tokenName, t->line); 
}

void Parser::XmlAttrDecl(XmlLangDefinition *xldef) {
		string tokenName; 
		Expect(1);
		tokenName = t->val; 
		Expect(8);
		Expect(3);
		xldef->AddAttr(t->val, tokenName, t->line); 
}

void Parser::ProcessingInstruction(XmlLangDefinition *xldef) {
		string tokenName; 
		Expect(1);
		tokenName = t->val; 
		Expect(8);
		Expect(3);
		xldef->AddProcessingInstruction(t->val, tokenName, t->line); 
}

void Parser::Term(Graph* &g) {
		Graph *g2; Node *rslv = NULL; g = NULL; 
		if (StartOf(13)) {
			if (la->kind == 30) {
				rslv = tab->NewNode(Node::rslv, (Symbol*)NULL, la->line); 
				Resolver(rslv->pos);
				g = new Graph(rslv); 
			}
			Factor(g2);
			if (rslv != NULL) tab->MakeSequence(g, g2);
			else g = g2; 
			while (StartOf(14)) {
				Factor(g2);
				tab->MakeSequence(g, g2); 
			}
		} else if (StartOf(15)) {
			g = new Graph(tab->NewNode(Node::eps, (Symbol*)NULL, 0)); 
		} else SynErr(35);
		if (g == NULL) // invalid start of Term
		g = new Graph(tab->NewNode(Node::eps, (Symbol*)NULL, 0)); 
}

void Parser::Resolver(Position* &pos) {
		Expect(30);
		Expect(24);
		int beg = la->pos; int col = la->col; 
		Condition();
		pos = new Position(beg, t->pos - beg, col); 
}

void Parser::Factor(Graph* &g) {
		wchar_t* name = NULL; int kind; Position *pos; bool weak = false; 
		 g = NULL;
		
		switch (la->kind) {
		case 1: case 3: case 5: case 23: {
			if (la->kind == 23) {
				Get();
				weak = true; 
			}
			Sym(name, kind);
			Symbol *sym = tab->FindSym(name);
			 if (sym == NULL && kind == str)
			   sym = (Symbol*)((*(tab->literals))[name]);
			 bool undef = (sym == NULL);
			 if (undef) {
			   if (kind == id)
			     sym = tab->NewSym(Node::nt, name, 0);  // forward nt
			   else if (genScanner) { 
			     sym = tab->NewSym(Node::t, name, t->line);
			   } else {  // undefined string in production
			     SemErr(L"undefined string in production");
			     sym = tab->eofSy;  // dummy
			   }
			 }
			 int typ = sym->typ;
			 if (typ != Node::t && typ != Node::nt)
			   SemErr(L"this symbol kind is not allowed in a production");
			 if (weak)
			   if (typ == Node::t) typ = Node::wt;
			   else SemErr(L"only terminals may be weak");
			 Node *p = tab->NewNode(typ, sym, t->line);
			 g = new Graph(p);
			
			if (la->kind == 18 || la->kind == 20) {
				Attribs(p);
				if (kind != id) SemErr(L"a literal must not have attributes"); 
			}
			if (undef)
			 sym->attrPos = p->pos;  // dummy
			else if ((p->pos == NULL) != (sym->attrPos == NULL))
			  SemErr(L"attribute mismatch between declaration and use of this symbol");
			
			break;
		}
		case 24: {
			Get();
			Expression(g);
			Expect(25);
			break;
		}
		case 26: {
			Get();
			Expression(g);
			Expect(27);
			tab->MakeOption(g); 
			break;
		}
		case 12: {
			Get();
			Expression(g);
			Expect(13);
			tab->MakeIteration(g); 
			break;
		}
		case 31: {
			SemText(pos);
			Node *p = tab->NewNode(Node::sem, (Symbol*)NULL, 0);
			   p->pos = pos;
			   g = new Graph(p);
			 
			break;
		}
		case 28: {
			Get();
			Node *p = tab->NewNode(Node::any, (Symbol*)NULL, 0);  // p.set is set in tab->SetupAnys
			g = new Graph(p);
			
			break;
		}
		case 29: {
			Get();
			Node *p = tab->NewNode(Node::sync, (Symbol*)NULL, 0);
			g = new Graph(p);
			
			break;
		}
		default: SynErr(36); break;
		}
		if (g == NULL) // invalid start of Factor
		 g = new Graph(tab->NewNode(Node::eps, (Symbol*)NULL, 0));
		
}

void Parser::Sym(wchar_t* &name, int &kind) {
		name = coco_string_create(L"???"); kind = id; 
		if (la->kind == 1) {
			Get();
			kind = id; coco_string_delete(name); name = coco_string_create(t->val); 
		} else if (la->kind == 3 || la->kind == 5) {
			if (la->kind == 3) {
				Get();
				coco_string_delete(name); name = coco_string_create(t->val); 
			} else {
				Get();
				wchar_t *subName = coco_string_create(t->val, 1, coco_string_length(t->val)-2);
				coco_string_delete(name); 
				name = coco_string_create_append(L"\"", subName);
				coco_string_delete(subName);
				coco_string_merge(name, L"\""); 
				
			}
			kind = str;
			if (coco_string_indexof(name, ' ') >= 0)
			  SemErr(L"literal tokens must not contain blanks"); 
		} else SynErr(37);
}

void Parser::Attribs(Node *p) {
		if (la->kind == 18) {
			Get();
			int beg = la->pos; int col = la->col; 
			while (StartOf(5)) {
				if (StartOf(6)) {
					Get();
				} else {
					Get();
					SemErr(L"bad string in attributes"); 
				}
			}
			Expect(19);
			if (t->pos > beg) p->pos = new Position(beg, t->pos - beg, col); 
		} else if (la->kind == 20) {
			Get();
			int beg = la->pos; int col = la->col; 
			while (StartOf(7)) {
				if (StartOf(8)) {
					Get();
				} else {
					Get();
					SemErr(L"bad string in attributes"); 
				}
			}
			Expect(21);
			if (t->pos > beg) p->pos = new Position(beg, t->pos - beg, col); 
		} else SynErr(38);
}

void Parser::Condition() {
		while (StartOf(16)) {
			if (la->kind == 24) {
				Get();
				Condition();
			} else {
				Get();
			}
		}
		Expect(25);
}



void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	CocoXml();

	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 33;

	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[17][35] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,T,T,T, T,T,x,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{x,T,T,T, T,T,T,x, T,T,T,x, T,T,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{T,T,x,T, x,T,x,x, x,T,x,x, T,x,x,x, x,x,x,x, x,x,T,T, T,x,T,x, T,T,T,T, x,x,x},
		{T,T,x,x, x,x,x,x, x,x,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x},
		{x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,x, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{x,T,T,T, x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,x, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,x,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{x,T,T,T, x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,x,T,T, T,T,T,T, T,T,T,T, T,T,x},
		{x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, x,T,x},
		{x,T,T,T, x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,x, x,T,x},
		{x,x,x,x, x,x,x,x, x,T,x,x, x,T,x,x, x,x,x,x, x,x,x,x, x,T,x,T, x,x,x,x, x,x,x},
		{x,T,x,T, x,T,x,x, x,T,x,x, T,T,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, x,x,x},
		{x,T,x,T, x,T,x,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,T, T,x,T,x, T,T,T,T, x,x,x},
		{x,T,x,T, x,T,x,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,T, T,x,T,x, T,T,x,T, x,x,x},
		{x,x,x,x, x,x,x,x, x,T,x,x, x,T,x,x, x,x,x,x, x,x,T,x, x,T,x,T, x,x,x,x, x,x,x},
		{x,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,x,T,T, T,T,T,T, T,T,x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"ident expected"); break;
			case 2: s = coco_string_create(L"number expected"); break;
			case 3: s = coco_string_create(L"string expected"); break;
			case 4: s = coco_string_create(L"badString expected"); break;
			case 5: s = coco_string_create(L"char expected"); break;
			case 6: s = coco_string_create(L"\"COMPILER\" expected"); break;
			case 7: s = coco_string_create(L"\"PRODUCTIONS\" expected"); break;
			case 8: s = coco_string_create(L"\"=\" expected"); break;
			case 9: s = coco_string_create(L"\".\" expected"); break;
			case 10: s = coco_string_create(L"\"END\" expected"); break;
			case 11: s = coco_string_create(L"\"NAMESPACE\" expected"); break;
			case 12: s = coco_string_create(L"\"{\" expected"); break;
			case 13: s = coco_string_create(L"\"}\" expected"); break;
			case 14: s = coco_string_create(L"\"OPTIONS\" expected"); break;
			case 15: s = coco_string_create(L"\"TAGS\" expected"); break;
			case 16: s = coco_string_create(L"\"ATTRS\" expected"); break;
			case 17: s = coco_string_create(L"\"PROCESSING_INSTRUCTIONS\" expected"); break;
			case 18: s = coco_string_create(L"\"<\" expected"); break;
			case 19: s = coco_string_create(L"\">\" expected"); break;
			case 20: s = coco_string_create(L"\"<.\" expected"); break;
			case 21: s = coco_string_create(L"\".>\" expected"); break;
			case 22: s = coco_string_create(L"\"|\" expected"); break;
			case 23: s = coco_string_create(L"\"WEAK\" expected"); break;
			case 24: s = coco_string_create(L"\"(\" expected"); break;
			case 25: s = coco_string_create(L"\")\" expected"); break;
			case 26: s = coco_string_create(L"\"[\" expected"); break;
			case 27: s = coco_string_create(L"\"]\" expected"); break;
			case 28: s = coco_string_create(L"\"ANY\" expected"); break;
			case 29: s = coco_string_create(L"\"SYNC\" expected"); break;
			case 30: s = coco_string_create(L"\"IF\" expected"); break;
			case 31: s = coco_string_create(L"\"(.\" expected"); break;
			case 32: s = coco_string_create(L"\".)\" expected"); break;
			case 33: s = coco_string_create(L"??? expected"); break;
			case 34: s = coco_string_create(L"invalid AttrDecl"); break;
			case 35: s = coco_string_create(L"invalid Term"); break;
			case 36: s = coco_string_create(L"invalid Factor"); break;
			case 37: s = coco_string_create(L"invalid Sym"); break;
			case 38: s = coco_string_create(L"invalid Attribs"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	wprintf(L"%ls", s); 
	exit(1);
}



