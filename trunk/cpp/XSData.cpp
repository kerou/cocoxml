#include <stdlib.h>
#include <wchar.h>
#include "XSData.h"
#include "Tab.h"
#include "Parser.h"
#include "BitArray.h"
#include "Scanner.h"

namespace CocoXml{

    XmlLangDefinition::XmlLangDefinition:eof(-1)(Tab *tab, Errors *errors){
        this->tab = tab;
        this->errors = errors;

        /*fix*/
        useVector = new bool[Enum.GetValues(typeof(Options)).Length];
        Tags = new Dictionary<const wchar_t, *TagInfo>();
        Attrs = new Dictionary<const wchar_t*, int>();
        PInstructions = new Dictionary<wchar_t*, int>();
    }
    void XmlLangDefinition::AddOption(const wchar_t* optname, int liane){
        /*fix*/
        int optval; Symbol sym;
        try {
            optval = (int)Enum.Parse(typeof(Options), optname);
        } catch (System.ArgumentException) {
            errors.SemErr("Unsupported option '" + optname + "' encountered.");
            return;
        }
        useVector[optval] = true;
        sym = tab.FindSym(optname);
        if (sym == null) tab.NewSym(Node.t, optname, line);
    }
    void XmlLangDefinition::addUniqueToken(const wchar_t* tokenName, const wchar_t* typeName, int line){
        Symbol *sym = tab->FindSym(tokenName);
        if (sym != NULL){
            wprintf(L"typeName '%ls' declared twice\n", tokenName);
            errors->count ++;
        }
        sym = tab->NewSym(Node->t, tokenName, line);
        return sym->n;
    }
    void XmlLangDefinition::AddTag(const wchar_t* tagname, wchar_t* tokenname, int line){
        /*fix*/
        TagInfo *tinfo = new TagInfo();
        tinfo->startToken = addUniqueToken(tokenname, L"Tag", line);
        tinfo.endToken = addUniqueToken("END_" + tokenname, "Tag", line);

        if (Tags->ContainsKey(tagname)){
            wprintf(L"Tag '%ls' declared twice\n", tagname);
            errors->count ++;
        }
        Tags->Add(tagname, tinfo);
    }
    void XmlLangDefinition::AddAttr(const wchar_t* attrname, const wchar_t* tokenname, int line){
        int value = addUniqueToken(tokenname, L"Attribute", line);
        if (Attrs->ContainsKey(attrname)){
            wprintf(L"Attribute '%ls' declared twice\n", attrname);
            errors->count ++;
        }
        Attrs->Add(attrname, value);
    }
    void XmlLangDefinition::AddProcessingInstruction(const wchar_t* pinstruction, const wchar_t* tokenname, int line){
        int value = addUniqueToken(tokenname, L"Processing instruction", line);

        if (PInstructions.ContainsKey(pinstruction)){
            wprintf(L"Processing Instruction '%ls' declared twice\n", pinstruction);
            errors->count ++;
        }
        PInstructions->Add(pinstruction, value);
    }
    void XmlLangDefinition::Write(FILE* gen){
        /*fix*/
        for (int option = 0; option < useVector.Length; ++option)
            if (useVector[option])
                gen.WriteLine("\tcurXLDef.useVector[{0}] = true; // {1}", option, ((Options)option).ToString());
        foreach (KeyValuePair<string, TagInfo> entry in Tags)
            gen.WriteLine("\tcurXLDef.AddTag({0}, {1}, {2});", entry.Key, entry.Value.startToken, entry.Value.endToken);
        foreach (KeyValuePair<string, int> entry in Attrs)
            gen.WriteLine("\tcurXLDef.AddAttr({0}, {1});", entry.Key, entry.Value);
        foreach (KeyValuePair<string, int> entry in PInstructions)
            gen.WriteLine("\tcurXLDef.AddProcessInstruction({0}, {1});", entry.Key, entry.Value);
    }


    XmlScannerData::XmlScannerData(Parser *parser){
        /*fix*/
        tab = parser.tab;
        errors = parser.errors;
        XmlLangMap = new Dictionary<string, XmlLangDefinition>();
    }
    void XmlScannerData::Add(const wchar_t* NamespaceURI, XmlLangDefinition *xldef){
        /*fix*/
        if (XmlLangMap.ContainsKey(NamespaceURI))
            errors.SemErr("Namespace '" + NamespaceURI + "' declared twice");
        XmlLangMap.Add(NamespaceURI, xldef);
    }
    void XmlScannerData::OpenGen(bool backUp) { /* pdt */
        /*fix*/
        try {
            string fn = Path.Combine(tab.outDir, "XmlScanner.cs"); /* pdt */
            if (File.Exists(fn) && backUp) File.Copy(fn, fn + ".old", true);
            gen = new StreamWriter(new FileStream(fn, FileMode.Create)); /* pdt */
        } catch (IOException) {
            throw new FatalError("Cannot generate xmlscanner file");
        }
    }
    void XmlScannerData::CopyFramePart(const wchar_t * stop){
        /*fix*/
        char startCh = stop[0];
        int endOfStopString = stop.Length - 1;
        int ch = fram.ReadByte();
        while (ch != EOF)
            if (ch == startCh) {
                int i = 0;
                do {
                    if (i == endOfStopString) return; // stop[0..i] found
                    ch = fram.ReadByte(); i++;
                } while (ch == stop[i]);
                // stop[0..i-1] found; continue with last read character
                gen.Write(stop.Substring(0, i));
            } else {
                gen.Write((char)ch); ch = fram.ReadByte();
            }
        throw new FatalError("incomplete or corrupt xml scanner frame file");
    }
    void XmlScannerData::WriteOptions(){
        /*fix*/
        gen.WriteLine("    public enum Options {");
        foreach (string optname in Enum.GetNames(typeof(Options)))
            gen.WriteLine("        {0},", optname);
        gen.WriteLine("    };");
        gen.WriteLine("    public const int numOptions = {0};", Enum.GetValues(typeof(Options)).Length);
    }
    void XmlScannerData::WriteDeclarations(){
        /*fix*/
        Symbol sym;
        gen.WriteLine("    static readonly int[] useKindVector = new int[] {");
        foreach (string optname in Enum.GetNames(typeof(Options))) {
            sym = tab.FindSym(optname);
            gen.WriteLine("        {0}, // {1}",
                          sym == null ? -1 : sym.n, optname);
        }
        gen.WriteLine("    };");
    }
    void XmlScannerData::WriteInitialization(){
        /*fix*/
        foreach (KeyValuePair<string, XmlLangDefinition> entry in XmlLangMap) {
            gen.WriteLine("\tcurXLDef = new XmlLangDefinition();");
            entry.Value.Write(gen);
            gen.WriteLine("\tXmlLangMap.Add(\"{0}\", curXLDef);", entry.Key);
        }
    }
    void XmlScannerData::WriteXmlScanner(){
        /*fix*/
        string fr = Path.Combine(tab.srcDir, "XmlScanner.frame");
        if (!File.Exists(fr)) {
            if (tab.frameDir != null)
                fr = Path.Combine(tab.frameDir.Trim(), "XmlScanner.frame");
            if (!File.Exists(fr))
                throw new FatalError("Cannot find XmlScanner.frame");
        }
        try {
            fram = new FileStream(fr, FileMode.Open,
                                  FileAccess.Read, FileShare.Read);
        } catch (FileNotFoundException) {
            throw new FatalError("Cannot open XmlScanner.frame.");
        }
        OpenGen(true); /* pdt */
        CopyFramePart("/*---- Begin ----*/");
        gen.Close();
        OpenGen(false);
        gen.WriteLine("// THIS FILE IS GENERATED BY CocoXml AUTOMATICALLY, DO NOT EDIT IT MANUALLY.");
        CopyFramePart("/*---- Namespace ----*/");
        if (tab.nsName != null && tab.nsName.Length > 0)
            gen.Write("namespace {0} {", tab.nsName);

        CopyFramePart("/*---- Options ----*/"); WriteOptions();

        CopyFramePart("/*---- Declarations ----*/"); WriteDeclarations();

        CopyFramePart("/*---- Initialization ----*/"); WriteInitialization();

        CopyFramePart("/*---- $$$ ----*/");
        if (tab.nsName != null && tab.nsName.Length > 0) gen.Write("}");
        gen.Close();
    }
} //namespace
