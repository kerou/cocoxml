#if !defined(COCOXML_XSDATA_H__)
#define COCOXML_XSDATA_H__

#include <stddef.h>
#include "Action.h"
#include "Comment.h"
#include "State.h"
#include "Symbol.h"
#include "Melted.h"
#include "Node.h"
#include "Target.h"

namespace CocoXml {

enum Options {
    UNKNOWN_NAMESPACE, END_UNKNOWN_NAMESPACE,
    UNKNOWN_TAG, END_UNKNOWN_TAG,
    UNKNOWN_ATTR_NAMESPACE, UNKNOWN_ATTR,
    UNKNOWN_PROCESSING_INSTRUCTION,
    // For the nodes in common status.
    TEXT, CDATA, COMMENT, WHITESPACE,
    // For the nodes in Unknown namespaces.
    UNS_TEXT, UNS_CDATA, UNS_COMMENT, UNS_WHITESPACE,
    // For the nodes in Unknown tags.
    UT_TEXT, UT_CDATA, UT_COMMENT, UT_WHITESPACE
}

class TagInfo{
    int startToken;
    int endToken;
}

class XmlLangDefinition{
    Tab                         *tab;
    Errors                      *errors;
    bool[]                      useVector;
    map<string, TagInfo>        Tags;
    map<string, int>            Attrs;
    map<string, int>            PInstructions;

    XmlLangDefinition(Tab *tab, Errors *errors);
    
    void AddOption(const wchar_t* optname, int liane);
    void AddTag(const wchar_t* tagname, wchar_t* tokenname, int line);
    void AddAttr(const wchar_t* attrname, const wchar_t* tokenname, int line);
    void AddProcessingInstruction(const wchar_t* pinstruction, const wchar_t* tokenname, int line);
    void Write(FILE* gen);
}

class XmlScannerData{
    const int   eof;
    
    Tab         *tab;
    Errors      *errors;
    FILE        *fram;
    FILE        *gen;

    map<const wchar_t*, XmlLangDefinition>  XmlLangMap;

    XmlScannerData(Parser *parser);
    void Add(const wchar_t* NamespaceURI, XmlLangDefinition *xldef);
    void OpenGen(bool backUp);
    void CopyFramePart(const wchar_t * stop);
    void WriteOptions();
    void WriteDeclarations();
    void WriteInitialization();
    void WriteXmlScanner();
}

} //namespace
