

#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

#include "Tab.h"
#include "XSData.h"
#include "ParserGen.h"


#include "Scanner.h"



class Errors {
public:
	int count;			// number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
private:
	enum {
		_EOF=0,
		_ident=1,
		_number=2,
		_string=3,
		_badString=4,
		_char=5,
		_ddtSym=34,
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s);
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

int id;
	int str;

	FILE* trace;		// other Coco objects referenced in this ATG
	Tab *tab;
    XmlScannerData *xsdata;
	ParserGen *pgen;

	bool genScanner;

	void InitDeclarations() {
		id  = 0;
		str = 1;
	}

/*-------------------------------------------------------------------------*/



	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void CocoXml();
	void XmlLangDefinition(XmlLangDefinition xldef);
	void XmlNamespaceDeclaration();
	void AttrDecl(Symbol *sym);
	void SemText(Position* &pos);
	void Expression(Graph* &g);
	void OptionDecl(XmlLangDefinition *xldef);
	void XmlTagDecl(XmlLangDefinition *xldef);
	void XmlAttrDecl(XmlLangDefinition *xldef);
	void ProcessingInstruction(XmlLangDefinition *xldef);
	void Term(Graph* &g);
	void Resolver(Position* &pos);
	void Factor(Graph* &g);
	void Sym(wchar_t* &name, int &kind);
	void Attribs(Node *p);
	void Condition();

	void Parse();

}; // end Parser



#endif // !defined(COCO_PARSER_H__)

