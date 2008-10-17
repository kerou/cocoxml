#include <stdlib.h>
#include <wchar.h>
#include "XSData.h"
#include "Tab.h"
#include "Parser.h"
#include "Scanner.h"

namespace CocoXml{

struct options_s enum_options[]={
    {"UNKNOWN_NAMESPACE", UNKNOWN_NAMESPACE},
    {"END_UNKNOWN_NAMESPACE", END_UNKNOWN_NAMESPACE},
    {"UNKNOWN_TAG", UNKNOWN_TAG},
    {"END_UNKNOWN_TAG", END_UNKNOWN_TAG},
    {"UNKNOWN_ATTR_NAMESPACE", UNKNOWN_ATTR_NAMESPACE}, 
    {"UNKNOWN_ATTR", UNKNOWN_ATTR},
    {"UNKNOWN_PROCESSING_INSTRUCTION", UNKNOWN_PROCESSING_INSTRUCTION},
    // For the nodes in common status.
    {"TEXT", TEXT}, 
    {"CDATA", CDATA}, 
    {"COMMENT", COMMENT}, 
    {"WHITESPACE", WHITESPACE},
    // For the nodes in Unknown namespaces.
    {"UNS_TEXT", UNS_TEXT}, 
    {"UNS_CDATA", UNS_CDATA}, 
    {"UNS_COMMENT", UNS_COMMENT}, 
    {"UNS_WHITESPACE", UNS_WHITESPACE},
    // For the nodes in Unknown tags.
    {"UT_TEXT", UT_TEXT}, 
    {"UT_CDATA", UT_CDATA}, 
    {"UT_COMMENT", UT_COMMENT}, 
    {"UT_WHITESPACE", UT_WHITESPACE},
    {NULL, -1}
};

    XmlLangDefinition::XmlLangDefinition(Tab *tab, Errors *errors){
        int i;
        this->tab = tab;
        this->errors = errors;
        
        for (i=0; enum_options[i].value>=0; i++){
            optionsMap.insert(make_pair(coco_string_create(enum_options[i].name), enum_options[i].value));
        }
        useVector.resize(optionsMap.size());
    }

    void XmlLangDefinition::AddOption(const wchar_t* optname, int line){
#if 0
        int optval; Symbol* sym;
        
        map<const wchar_t*, int>::iterator it = optionsMap.find(optname);
        if(it != optionsMap.end()){
            /* find it */
            optval = it->second;
        }else{
            wchar_t fmt[200];
            coco_swprintf(fmt, 200, L"Unsupported option '%ls' encountered.", optname);
            /* unsupported option */
            errors->Exception(fmt);
            return;
        }
        useVector[optval] = true;

        sym = tab->FindSym(optname);
        if (sym == NULL) tab->NewSym(Node.t, optname, line);
#endif
    }
    int XmlLangDefinition::addUniqueToken(const wchar_t* tokenName, const wchar_t* typeName, int line){
#if 0
        Symbol *sym = tab->FindSym(tokenName);
        if (sym != NULL){
            wprintf(L"typeName '%ls' declared twice\n", tokenName);
            errors->count ++;
        }
        sym = tab->NewSym(Node.t, tokenName, line);
        return sym->n;
#endif
    }
    void XmlLangDefinition::AddTag(const wchar_t* tagname, wchar_t* tokenname, int line){
#if 0        
        TagInfo *tinfo = new TagInfo();
        tinfo->startToken = addUniqueToken(tokenname, L"Tag", line);
        
        tinfo->endToken = addUniqueToken("END_" + tokenname, "Tag", line);

        map<const wchar_t*, int>::iterator it = Tags->find(tagname);
        if (it!=Tags.end()){
            wprintf(L"Tag '%ls' declared twice\n", tagname);
            errors->count ++;
        }
        Tags.insert(make_pair(tagname, tinfo));
#endif
    }
    void XmlLangDefinition::AddAttr(const wchar_t* attrname, const wchar_t* tokenname, int line){
#if 0
        int value = addUniqueToken(tokenname, L"Attribute", line);
        map<const wchar_t*, int>::iterator it = Attrs.find(attrname);
        if (it!=Attrs.end()){
            wprintf(L"Attribute '%ls' declared twice\n", attrname);
            errors->count ++;
        }
        Attrs.insert(make_pair(attrname, value));
#endif
    }
    void XmlLangDefinition::AddProcessingInstruction(const wchar_t* pinstruction, const wchar_t* tokenname, int line){
#if 0
        int value = addUniqueToken(tokenname, L"Processing instruction", line);
        map<const wchar_t*, int>::iterator it = PInstructions->find(pinstruction);
        if (it!=PInstructions.end()){
            wprintf(L"Processing Instruction '%ls' declared twice\n", pinstruction);
            errors->count ++;
        }
        PInstructions.insert(make_pair(pinstruction, value));
#endif
    }
    void XmlLangDefinition::Write(FILE* gen){
#if 0
        for (int option = 0; option < useVector.Length; ++option)
            if (useVector[option])
                gen.WriteLine("\tcurXLDef.useVector[{0}] = true; // {1}", option, ((Options)option).ToString());
        foreach (KeyValuePair<string, TagInfo> entry in Tags)
            gen.WriteLine("\tcurXLDef.AddTag({0}, {1}, {2});", entry.Key, entry.Value.startToken, entry.Value.endToken);
        foreach (KeyValuePair<string, int> entry in Attrs)
            gen.WriteLine("\tcurXLDef.AddAttr({0}, {1});", entry.Key, entry.Value);
        foreach (KeyValuePair<string, int> entry in PInstructions)
            gen.WriteLine("\tcurXLDef.AddProcessInstruction({0}, {1});", entry.Key, entry.Value);
#endif
    }


    XmlScannerData::XmlScannerData(Parser *parser){
        this->tab = parser->tab;
        this->errors = parser->errors;
    }
    void XmlScannerData::Add(const wchar_t* NamespaceURI, XmlLangDefinition *xldef){
        map<const wchar_t*, XmlLangDefinition*>::iterator it=XmlLangMap.find(NamespaceURI);
        if (it != XmlLangMap.end()){
            wchar_t fmt[200];
            coco_swprintf(fmt, 200, L"Namespace '%ls' declared twice.", NamespaceURI);
            /* unsupported option */
            errors->Exception(fmt);
        }
        XmlLangMap.insert(make_pair(NamespaceURI, xldef));
    }
    void XmlScannerData::OpenGen(const wchar_t *genName, bool backUp) { /* pdt */
        wchar_t *fn = coco_string_create_append(tab->outDir, genName); /* pdt */
        char *chFn = coco_string_create_char(fn);
        FILE* tmp;
        if (backUp && ((tmp = fopen(chFn, "r")) != NULL)) {
            fclose(tmp);
            wchar_t *oldName = coco_string_create_append(fn, L".old");
            char *chOldName = coco_string_create_char(oldName);
            remove(chOldName); rename(chFn, chOldName); // copy with overwrite
            coco_string_delete(chOldName);
            coco_string_delete(oldName);
        }
        if ((gen = fopen(chFn, "w")) == NULL) {
            errors->Exception(L"-- Cannot generate scanner file");
        }
        coco_string_delete(chFn);
        coco_string_delete(fn);
    }
    void XmlScannerData::CopyFramePart(const wchar_t * stop){
        wchar_t startCh = stop[0];
        int endOfStopString = coco_string_length(stop)-1;
        wchar_t ch = 0;

        fwscanf(fram, L"%lc", &ch); //fram.ReadByte();
        while (!feof(fram)) // ch != EOF
            if (ch == startCh) {
                int i = 0;
                do {
                    if (i == endOfStopString) return; // stop[0..i] found
                    fwscanf(fram, L"%lc", &ch); i++;
                } while (ch == stop[i]);
                // stop[0..i-1] found; continue with last read character
                wchar_t *subStop = coco_string_create(stop, 0, i);
                fwprintf(gen, L"%ls", subStop);
                coco_string_delete(subStop);
            } else {
                fwprintf(gen, L"%lc", ch);
                fwscanf(fram, L"%lc", &ch);
            }
        errors->Exception(L" -- incomplete or corrupt scanner frame file");
    }
    void XmlScannerData::WriteOptions(){
#if 0
        /*fix*/
        gen.WriteLine("    public enum Options {");
        foreach (string optname in Enum.GetNames(typeof(Options)))
            gen.WriteLine("        {0},", optname);
        gen.WriteLine("    };");
        gen.WriteLine("    public const int numOptions = {0};", Enum.GetValues(typeof(Options)).Length);
#endif
    }
    void XmlScannerData::WriteDeclarations(){
#if 0
        /*fix*/
        Symbol sym;
        gen.WriteLine("    static readonly int[] useKindVector = new int[] {");
        foreach (string optname in Enum.GetNames(typeof(Options))) {
            sym = tab.FindSym(optname);
            gen.WriteLine("        {0}, // {1}",
                          sym == null ? -1 : sym.n, optname);
        }
        gen.WriteLine("    };");
#endif
    }
    void XmlScannerData::WriteInitialization(){
#if 0
        /*fix*/
        foreach (KeyValuePair<string, XmlLangDefinition> entry in XmlLangMap) {
            gen.WriteLine("\tcurXLDef = new XmlLangDefinition();");
            entry.Value.Write(gen);
            gen.WriteLine("\tXmlLangMap.Add(\"{0}\", curXLDef);", entry.Key);
        }
#endif
    }
    void XmlScannerData::WriteXmlScanner(){
        wchar_t *fr = coco_string_create_append(tab->srcDir, L"XmlScanner.frame");
        char *chFr = coco_string_create_char(fr);

        FILE* tmp;
        if ((tmp = fopen(chFr, "r")) == NULL) {
            if (coco_string_length(tab->frameDir) != 0) {
                delete [] fr;
                fr = coco_string_create(tab->frameDir);
                coco_string_merge(fr, L"/");
                coco_string_merge(fr, L"Scanner.frame");
            }
            coco_string_delete(chFr);
            chFr = coco_string_create_char(fr);
            if ((tmp = fopen(chFr, "r")) == NULL) {
                errors->Exception(L"-- Cannot find Scanner.frame\n");
            } else {
                fclose(tmp);
            }
        } else {
            fclose(tmp);
        }
        if ((fram = fopen(chFr, "r")) == NULL) {
            errors->Exception(L"-- Cannot open Scanner.frame.\n");
        }
        coco_string_delete(chFr);
        coco_string_delete(fr);

        OpenGen(L"XmlScanner.h", true);
        CopyFramePart(L"/*---- Begin ----*/");
        fclose(gen);

        OpenGen(L"XmlScanner.cpp", true);
        CopyFramePart(L"/*---- Begin ----*/");
        fwprintf(gen, L"// THIS FILE IS GENERATED BY CocoXml AUTOMATICALLY, DO NOT EDIT IT MANUALLY.");
        CopyFramePart(L"/*---- Namespace Begin ----*/");
//        nrOfNs = GenNamespaceOpen(tab->nsName);

        CopyFramePart(L"/*---- Options ----*/"); WriteOptions();

        CopyFramePart(L"/*---- Declarations ----*/"); WriteDeclarations();

        CopyFramePart(L"/*---- Initialization ----*/"); WriteInitialization();

        CopyFramePart(L"/*---- $$$ ----*/");
        CopyFramePart(L"/*---- Namespace End ----*/");
//        GenNamespaceClose(nrOfNs);
        fclose(gen);
    }
} //namespace
