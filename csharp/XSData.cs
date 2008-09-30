/*-------------------------------------------------------------------------
XSData.cs -- XML Scanner Data
Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>

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
-------------------------------------------------------------------------*/
using System;
using System.IO;
using System.Collections.Generic;

namespace at.jku.ssw.CocoXml {

public class TagInfo {
    public int startToken;
    public int endToken;
}

public class XmlLangDefinition {
    // The following list must be same with OptionDecl in CocoXml.atg.
    public static readonly string[] OptStrings = new string[] {
	"UNKNOWN_NAMESPACE", "UNKNOWN_TAG", "UNKNOWN_ATTR",
	"TEXT", "CDATA", "COMMENT",
	"WHITESPACE", "PROCESSING_INSTRUCTION",
	// For the nodes in Unknown namespaces.
	"UNS_TEXT", "UNS_CDATA", "UNS_COMMENT",
	"UNS_WHITESPACE", "UNS_PROCESSING_INSTRUCTION",
	// For the nodes in Unknown tags.
	"UT_TEXT", "UT_CDATA", "UT_COMMENT",
	"UT_WHITESPACE", "UT_PROCESSING_INSTRUCTION"
    };
    public const int numOptions = 18;

    Tab                         tab;
    Errors                      errors;
    bool[]                      useVector;
    Dictionary<string, TagInfo> Tags;
    Dictionary<string, int>     Attrs;

    public XmlLangDefinition(Tab tab, Errors errors) {
	if (numOptions != OptStrings.Length)
	    Console.WriteLine("numOptions:{0} != OptString.Length:{1}",
			      numOptions, OptStrings.Length);
	this.tab = tab;
	this.errors = errors;
	useVector = new bool[numOptions];
	Tags = new Dictionary<string, TagInfo>();
	Attrs = new Dictionary<string, int>();
    }

    public void AddOption(string optname, int line) {
	Symbol sym;
	for (int option = 0; option < numOptions; ++option)
	    if (optname == OptStrings[option]) {
		useVector[option] = true;
		sym = tab.FindSym(optname);
		if (sym == null) tab.NewSym(Node.t, optname, line);
		return;
	    }
	errors.SemErr("Unsupported option '" + optname + "' encountered.");
    }

    int addUniqueToken(string tokenName, string typeName, int line) {
	Symbol sym = tab.FindSym(tokenName);
	if (sym != null)
	    errors.SemErr(typeName + " '" + tokenName + "' declared twice");
	sym = tab.NewSym(Node.t, tokenName, line);
	return sym.n;
    }

    public void AddTag(string tagname, string tokenname, int line) {
	TagInfo tinfo = new TagInfo();
	tinfo.startToken = addUniqueToken(tokenname, "Tag", line);
	tinfo.endToken = addUniqueToken("END_" + tokenname, "Tag", line);

	if (Tags.ContainsKey(tagname))
	    errors.SemErr("Tag '" + tagname + "' declared twice");
	Tags.Add(tagname, tinfo);
    }

    public void AddAttr(string attrname, string tokenname, int line) {
	int value = addUniqueToken(tokenname, "Attribute", line);
	if (Attrs.ContainsKey(attrname))
	    errors.SemErr("Attribute '" + attrname + "' declared twice");
	Attrs.Add(attrname, value);
    }

    public void Write(StreamWriter gen) {
	for (int option = 0; option < useVector.Length; ++option)
	    if (useVector[option])
		gen.WriteLine("\tcurXLDef.useVector[{0}] = true;", option);
	foreach (KeyValuePair<string, TagInfo> entry in Tags)
	    gen.WriteLine("\tcurXLDef.AddTag({0}, {1}, {2});", entry.Key,
			  entry.Value.startToken, entry.Value.endToken);
	foreach (KeyValuePair<string, int> entry in Attrs)
	    gen.WriteLine("\tcurXLDef.AddAttr({0}, {1});", entry.Key,
			  entry.Value);
    }
}

public class XmlScannerData {
    public const int EOF = -1;

    Tab           tab;
    Errors        errors;
    FileStream    fram;
    StreamWriter  gen;

    Dictionary<string, XmlLangDefinition>  XmlLangMap;

    public XmlScannerData(Parser parser) {
	tab = parser.tab;
	errors = parser.errors;
	XmlLangMap = new Dictionary<string, XmlLangDefinition>();
    }

    public void Add(string NamespaceURI, XmlLangDefinition xldef) {
	if (XmlLangMap.ContainsKey(NamespaceURI))
	    errors.SemErr("Namespace '" + NamespaceURI + "' declared twice");
	XmlLangMap.Add(NamespaceURI, xldef);
    }

    void OpenGen(bool backUp) { /* pdt */
	try {
	    string fn = Path.Combine(tab.outDir, "XmlScanner.cs"); /* pdt */
	    if (File.Exists(fn) && backUp) File.Copy(fn, fn + ".old", true);
	    gen = new StreamWriter(new FileStream(fn, FileMode.Create)); /* pdt */
	} catch (IOException) {
	    throw new FatalError("Cannot generate xmlscanner file");
	}
    }

    void CopyFramePart(string stop) {
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

    void WriteOptions() {
	gen.WriteLine("    public enum Options {");
	gen.Write("        " + XmlLangDefinition.OptStrings[0]);
	for (int option = 1; option < XmlLangDefinition.numOptions; ++option)
	    gen.Write(", " + XmlLangDefinition.OptStrings[option]);
	gen.WriteLine("");
	gen.WriteLine("    };");
	gen.WriteLine("    public const int numOptions = {0};",
		      XmlLangDefinition.numOptions);
    }

    int getOptionKind(int option) {
	Symbol sym = tab.FindSym(XmlLangDefinition.OptStrings[option]);
	if (sym == null) return -1;
	return sym.n;
    }

    void WriteDeclarations() {
	gen.Write("    static readonly int[] useKindVector = new int[] { " +
		  getOptionKind(0));
	for (int option = 1; option < XmlLangDefinition.numOptions; ++option)
	    gen.Write(", " + getOptionKind(option));
	gen.WriteLine(" };");
    }

    void WriteInitialization() {
	foreach (KeyValuePair<string, XmlLangDefinition> entry in XmlLangMap) {
	    gen.WriteLine("\tcurXLDef = new XmlLangDefinition();");
	    entry.Value.Write(gen);
	    gen.WriteLine("\tXmlLangMap.Add(\"{0}\", curXLDef);", entry.Key);
	}
    }

    public void WriteXmlScanner() {
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
	OpenGen(true);
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
}

} // namespace.
