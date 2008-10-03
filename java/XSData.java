/*-------------------------------------------------------------------------
XSData.java -- XML Scanner Data
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
------------------------------------------------------------------------*/

package CocoXml;

import java.io.PrintWriter;

class TagInfo {
    public int startToken;
    public int endToken;
}

enum Options {
    UNKNOWN_NAMESPACE, END_UNKNOWN_NAMESPACE,
    UNKNOWN_TAG, END_UNKNOWN_TAG,
    UNKNOWN_ATTR_NAMESPACE, UNKNOWN_ATTR,
    // For the nodes in common status.
    TEXT, CDATA, COMMENT, WHITESPACE, PROCESSING_INSTRUCTION,
    // For the nodes in Unknown namespaces.
    UNS_TEXT, UNS_CDATA, UNS_COMMENT, UNS_WHITESPACE, UNS_PROCESSING_INSTRUCTION,
    // For the nodes in Unknown tags.
    UT_TEXT, UT_CDATA, UT_COMMENT, UT_WHITESPACE, UT_PROCESSING_INSTRUCTION
}

class XmlLangDefinition {
    Tab    tab;
    Errors errors;

    public XmlLangDefinition(Tab tab, Errors errors) {
	this.tab = tab;
	this.errors = errors;
    }

    public void AddOption(String optName, int line) {
    }

    public void AddTag(String tagName, String tokenName, int line) {
    }

    public void AddAttr(String attrName, String tokenName, int line) {
    }

    public void Write(PrintWriter gen) {
    }
}

class XmlScannerData {
    public static final int EOF = -1;

    Tab     tab;
    Errors  errors;

    public XmlScannerData(Parser parser) {
	tab = parser.tab;
	errors = parser.errors;
    }

    public void Add(String NamespaceURI, XmlLangDefinition xldef) {
    }

    public void WriteXmlScanner() {
    }
}
