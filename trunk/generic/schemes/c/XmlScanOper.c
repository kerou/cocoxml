/*-------------------------------------------------------------------------
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
#include  "c/XmlScanOper.h"
#include  "c/Token.h"
#include  "c/ErrorPool.h"
#include  <ctype.h>

static const char nsSep = '@';

static const CcxSpec_t BottomSpec = {
    "BOTTOM", TRUE,
    { -1, -1, -1, -1,
      -1, -1, -1,
      -1, -1, -1,
      -1, -1, -1 },
    NULL, 0,
    NULL, 0,
    NULL, 0
};

static CcsToken_t * CXS_GetLastToken(CcxScanOper_t * self);
static CcsToken_t * CXS_Append(CcxScanOper_t * self, CcsToken_t * last,
			       int kind, const char * val, size_t vallen);

static void CXS_UpdateKinds(CcxScanOper_t * self);
static void CXS_PushSpec(CcxScanOper_t * self, const CcxSpec_t * spec,
			 const CcxTag_t * tag);
static void CXS_PopSpec(CcxScanOper_t * self, const CcxSpec_t * spec,
			const CcxTag_t * tag);

static CcsBool_t CXS_EnsureTextSpace(CcxScanOper_t * self,
				     size_t sz0, size_t sz1);
static void CXS_AppendText(CcxScanOper_t * self, const char * text, size_t len);
static void CXS_Text2Tokens(CcxScanOper_t * self);

static int cmpXmlSpec(const void * name, const void * spec);
static int cmpXmlTag(const void * name, const void * tag);
static int casecmpXmlTag(const void * name, const void * tag);
static int cmpXmlAttr(const void * name, const void * attr);
static int casecmpXmlAttr(const void * name, const void * attr);
static int cmpXmlPI(const void * name, const void * pi);
static int casecmpXmlPI(const void * name, const void * pi);

static void
CXS_StartElement(void * self, const XML_Char * name, const XML_Char ** attrs)
{
    const XML_Char ** curattr;
    const CcxSpec_t * tagSpec, * attrSpec;
    const CcxTag_t * tag; const CcxAttr_t * attr;
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    CcsToken_t * last = CXS_GetLastToken(ccself);
    const char * localname = strchr(name, nsSep);

    CXS_Text2Tokens(ccself);
    localname = localname ? localname + 1 : name;
    if (!(tagSpec = (const CcxSpec_t *)
	  bsearch(name, ccself->firstXmlSpec, ccself->numXmlSpecs,
		  sizeof(CcxSpec_t), cmpXmlSpec))) {
	last = CXS_Append(ccself, last, ccself->kindUnknownNS, NULL, 0);
    } else if (!(tag = (const CcxTag_t *)
		 bsearch(localname, tagSpec->firstTag, tagSpec->numTags, sizeof(CcxTag_t),
			 tagSpec->caseSensitive ? cmpXmlTag : casecmpXmlTag))) {
	last = CXS_Append(ccself, last,
			  tagSpec->kinds[XSO_UnknownTag], NULL, 0);
    } else {
	last = CXS_Append(ccself, last, tag->kind, NULL, 0);
    }
    CXS_PushSpec(ccself, tagSpec, tag);
    for (curattr = attrs; curattr[0] != NULL; curattr += 2) {
	CcsAssert(curattr[1] != NULL);
	localname = strchr(curattr[0], nsSep);
	if (localname == NULL) { attrSpec = tagSpec; localname = curattr[0]; }
	else {
	    attrSpec = (const CcxSpec_t *)
		bsearch(curattr[0], ccself->firstXmlSpec, ccself->numXmlSpecs,
			sizeof(CcxSpec_t), cmpXmlSpec);
	    ++localname;
	}
	if (!attrSpec) {
	    last = CXS_Append(ccself, last, ccself->kindUnknownNS, NULL, 0);
	} else if (!(attr = (const CcxAttr_t *)
		     bsearch(localname, attrSpec->firstAttr, attrSpec->numAttrs, sizeof(CcxAttr_t),
			     attrSpec->caseSensitive ? cmpXmlAttr : casecmpXmlAttr))) {
	    last = CXS_Append(ccself, last,
				 attrSpec->kinds[XSO_UnknownAttr],
				 curattr[1], strlen(curattr[1]));
	} else {
	    last = CXS_Append(ccself, last,
				 attr->kind, curattr[1], strlen(curattr[1]));
	}
    }
}

static void
CXS_EndElement(void * self, const XML_Char * name)
{
    const CcxSpec_t * tagSpec;
    const CcxTag_t * tag;
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    CcsToken_t * last = CXS_GetLastToken(ccself);
    const char * localname = strchr(name, nsSep);

    CXS_Text2Tokens(ccself);
    localname = localname ? localname + 1 : name;
    if (!(tagSpec = (const CcxSpec_t *)
	  bsearch(name, ccself->firstXmlSpec, ccself->numXmlSpecs,
		  sizeof(CcxSpec_t), cmpXmlSpec))) {
	CXS_Append(ccself, last, ccself->kindUnknownNS, NULL, 0);
    } else if (!(tag = (const CcxTag_t *)
		 bsearch(localname, tagSpec->firstTag, tagSpec->numTags, sizeof(CcxTag_t),
			 tagSpec->caseSensitive ? cmpXmlTag : casecmpXmlTag))) {
	CXS_Append(ccself, last, tagSpec->kinds[XSO_UnknownTagEnd], NULL, 0);
    } else {
	CXS_Append(ccself, last, tag->kindEnd, NULL, 0);
    }
    CXS_PopSpec(ccself, tagSpec, tag);
}

static void
CXS_CharacterData(void * self, const XML_Char * s, int len)
{
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    CXS_AppendText(ccself, s, len);
}

static void
CXS_PInstruction(void * self, const XML_Char * target, const XML_Char * data)
{
    const CcxPInstruction_t * pi;
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    const CcxSpec_t * spec = ccself->effect->spec;

    CXS_Text2Tokens(ccself);
    pi = (const CcxPInstruction_t *)
	bsearch(target, spec->firstPInstruction, spec->numPInstructions,
		sizeof(CcxPInstruction_t),
		spec->caseSensitive ? cmpXmlPI : casecmpXmlPI);
    if (pi) {
	CXS_Append(ccself, CXS_GetLastToken(ccself),
		   pi->kind, data, strlen(data));
    } else {
	CXS_Append(ccself, CXS_GetLastToken(ccself),
		   spec->kinds[XSO_UnknownProcessingInstruction],
		   data, strlen(data));
    }
}

static void
CXS_Comment(void * self, const XML_Char * data)
{
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    CXS_Text2Tokens(ccself);
    CXS_Append(ccself, CXS_GetLastToken(ccself),
	       ccself->kindComment, data, strlen(data));
}

static const char * dummyval = "dummy";
CcxScanOper_t *
CcxScanOper(CcxScanOper_t * self, CcsErrorPool_t * errpool,
	    const char * filename)
{
    self->errpool = errpool;
    self->parser = XML_ParserCreateNS(NULL, nsSep);
    CcsAssert(self->parser);

    XML_SetUserData(self->parser, self);
    XML_SetElementHandler(self->parser, CXS_StartElement, CXS_EndElement);
    XML_SetCharacterDataHandler(self->parser, CXS_CharacterData);
    XML_SetProcessingInstructionHandler(self->parser, CXS_PInstruction);
    XML_SetCommentHandler(self->parser, CXS_Comment);

    if (!(self->fp = fopen(filename, "r"))) {
	fprintf(stderr, "Can not open '%s'.\n", filename);
	goto errquit0;
    }
    self->EOFGenerated = FALSE;
    if (!(self->dummy = CcsToken(0, 0, 0, 0, dummyval, strlen(dummyval))))
	goto errquit1;
    CcxScanOper_IncRef(self, self->dummy);
    self->tokens = self->peek = NULL;
    if (!(self->textStart = CcsMalloc(SZ_TEXTBUF))) goto errquit2;
    self->textUsed = self->textStart;
    self->textSpace = SZ_TEXTBUF;
    self->stack[0].spec = &BottomSpec;
    self->stack[0].tag = NULL;
    self->cur = self->effect = self->stack;
    CXS_UpdateKinds(self);
    return self;
 errquit2:
    CcsToken_Destruct(self->dummy);
 errquit1:
    fclose(self->fp);
 errquit0:
    XML_ParserFree(self->parser);
    return NULL;
}

void
CcxScanOper_Destruct(CcxScanOper_t * self)
{
    CcsToken_t * cur, * next;

    CcsFree(self->textStart);
    for (cur = self->tokens; cur; cur = next) {
	next = cur->next;
	CcxScanOper_DecRef(self, cur);
    }
    CcxScanOper_DecRef(self, self->dummy);
    fclose(self->fp);
    XML_ParserFree(self->parser);
}

CcsToken_t *
CcxScanOper_GetDummy(CcxScanOper_t * self)
{
    CcxScanOper_IncRef(self, self->dummy);
    return self->dummy;
}

static CcsBool_t
CcxScanOper_Parse(CcxScanOper_t * self)
{
    char buf[4096]; int rc;
    enum XML_Status status;
    enum XML_Error error;

    if (self->EOFGenerated) goto AppendEOF;
    while (self->tokens == NULL) {
	rc = fread(buf, 1, sizeof(buf), self->fp);
	if (ferror(self->fp)) {
	    fprintf(stderr, "Error in reading.\n");
	    goto AppendEOF;
	}
	status = XML_Parse(self->parser, buf, rc, rc < sizeof(buf));
	if (status != XML_STATUS_OK &&
	    (error = XML_GetErrorCode(self->parser)) != XML_ERROR_FINISHED) {
	    fprintf(stderr, "XML Parse error: %s.\n", XML_ErrorString(error));
	    goto AppendEOF;
	}
	if (feof(self->fp)) goto AppendEOF;
    }
    return TRUE;
 AppendEOF:
    CXS_Append(self, CXS_GetLastToken(self), 0, NULL, 0);
    self->EOFGenerated = TRUE;
    return FALSE;
}

CcsToken_t *
CcxScanOper_Scan(CcxScanOper_t * self)
{
    CcsToken_t * token;
    while (self->tokens == NULL)
	if (!CcxScanOper_Parse(self)) break;
    token = self->tokens;
    if (token) {
	if (self->peek == self->tokens)
	    self->tokens = self->peek = self->tokens->next;
	else
	    self->tokens = self->tokens->next;
    }
    return token;
}

CcsToken_t *
CcxScanOper_Peek(CcxScanOper_t * self)
{
    CcsToken_t * token;
    while (self->peek == NULL)
	if (!CcxScanOper_Parse(self)) return NULL;
    token = self->peek;
    if (token) {
	self->peek = self->peek->next;
	CcxScanOper_IncRef(self, token);
    }
    return token;
}

void
CcxScanOper_ResetPeek(CcxScanOper_t * self)
{
    self->peek = self->tokens;
}

void
CcxScanOper_IncRef(CcxScanOper_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
CcxScanOper_DecRef(CcxScanOper_t * self, CcsToken_t * token)
{
    if (--token->refcnt > 0) return;
    CcsToken_Destruct(token);
}

/* Support functions */
static CcsToken_t *
CXS_GetLastToken(CcxScanOper_t * self)
{
    CcsToken_t * cur;
    if (self->tokens == NULL) return NULL;
    for (cur = self->tokens; cur->next; cur = cur->next);
    return cur;
}

static CcsToken_t *
CXS_Append(CcxScanOper_t * self, CcsToken_t * last,
	   int kind, const char * val, size_t vallen)
{
    CcsToken_t * token;
    if (kind < 0) return last;
    token = CcsToken(kind,
		     XML_GetCurrentByteIndex(self->parser),
		     XML_GetCurrentColumnNumber(self->parser),
		     XML_GetCurrentLineNumber(self->parser),
		     val, vallen);
    if (token == NULL) {
	fprintf(stderr, "Not enough memory!");
	return last;
    }
    if (last == NULL) self->tokens = self->peek = token;
    else last->next = token;
    return token;
}

static void
CXS_UpdateKinds(CcxScanOper_t * self)
{
    const int * kinds = self->effect->spec->kinds;
    if (self->effect < self->cur) {
	self->kindText = kinds[XSO_UNS_Text];
	self->kindWhitespace = kinds[XSO_UNS_Whitespace];
	self->kindComment = kinds[XSO_UNS_Comment];
    } else if (self->effect->tag) {
	self->kindText = kinds[XSO_Text];
	self->kindWhitespace = kinds[XSO_Whitespace];
	self->kindComment = kinds[XSO_Comment];
    } else {
	self->kindText = kinds[XSO_UT_Text];
	self->kindWhitespace = kinds[XSO_UT_Whitespace];
	self->kindComment = kinds[XSO_UT_Comment];
    }
}

static void
CXS_PushSpec(CcxScanOper_t * self, const CcxSpec_t * spec, const CcxTag_t * tag)
{
    const CcxScanStack_t * last = self->stack + SZ_STACK;
    ++self->cur;
    if (self->cur < last) {
	self->cur->spec = spec;
	self->cur->tag = tag;
	if (spec) self->effect = self->cur;
	CXS_UpdateKinds(self);
    } else {
	self->kindText = self->kindWhitespace = self->kindComment = -1;
	CcsErrorPool_Error(self->errpool,
			   XML_GetCurrentLineNumber(self->parser),
			   XML_GetCurrentColumnNumber(self->parser),
			   "XML Tag too deep(limit = %d)", SZ_STACK);
    }
}

static void
CXS_PopSpec(CcxScanOper_t * self, const CcxSpec_t * spec, const CcxTag_t * tag)
{
    const CcxScanStack_t * last = self->stack + SZ_STACK;
    if (self->cur < last) {
	CcsAssert(self->cur->spec == spec);
	CcsAssert(self->cur->tag == tag);
	if (self->cur == self->effect)
	    for (--self->effect; !self->effect->spec; --self->effect);
	CXS_UpdateKinds(self);
    } else {
	CcsAssert(self->kindText == -1);
	CcsAssert(self->kindWhitespace == -1);
	CcsAssert(self->kindComment == -1);
    }
    --self->cur;
}

static CcsBool_t
CXS_EnsureTextSpace(CcxScanOper_t * self, size_t sz0, size_t sz1)
{
    size_t totalsz, cursz;
    char * newStart;
    totalsz = self->textSpace;
    cursz = (self->textUsed - self->textStart) + (sz0 > sz1 ? sz0 : sz1);
    while (cursz > totalsz) totalsz += totalsz;
    if (totalsz == self->textSpace) return TRUE;
    if (!(newStart = CcsRealloc(self->textStart, totalsz))) return FALSE;
    self->textSpace = totalsz;
    self->textUsed = newStart + (self->textUsed - self->textStart);
    self->textStart = newStart;
    return TRUE;
}

static void
CXS_AppendText(CcxScanOper_t * self, const char * text, size_t len)
{
    CcsToken_t * lastT;
    const char * cur, * last = text + len;

    if (self->kindText < 0 && self->kindWhitespace < 0) return;
    lastT = CXS_GetLastToken(self);
    if (self->textStart == self->textUsed || isspace(*self->textStart)) {
	/* All characters in self->textStart ~ self->textUsed is space.
	 * If any non-space character is encountered, cut them off from
	 * text ~ last. And append kindWhitespace if required. */
	for (cur = text; cur < last; ++cur)
	    if (!isspace(*cur)) break;
	if (cur < last) { /* Whitespace can be generated. */
	    if (self->textStart == self->textUsed) {
		if (text == cur) {
		    /* Do nothing. */
		} else { /* text < cur */
		    lastT = CXS_Append(self, lastT, self->kindWhitespace,
				       text, cur - text);
		    text = cur;
		}
	    } else {
		if (text == cur) {
		    lastT = CXS_Append(self, lastT, self->kindWhitespace,
				       self->textStart, self->textUsed - self->textStart);
		} else {
		    if (self->kindWhitespace >= 0) {
			if (!CXS_EnsureTextSpace(self,
						 (self->textUsed - self->textStart) + (cur - text),
						 last - cur))
			    goto nomem;
			memcpy(self->textUsed, text, cur - text);
			self->textUsed += cur - text;
			lastT = CXS_Append(self, lastT, self->kindWhitespace,
					   self->textStart, self->textUsed - self->textStart);
		    }
		    text = cur;
		}
		self->textUsed = self->textStart;
	    }
	}
    }
    if (text == last) return;
    if (self->kindWhitespace < 0 && isspace(*text)) return;
    if (!CXS_EnsureTextSpace(self, last - text, 0)) goto nomem;
    memcpy(self->textUsed, text, last - text);
    self->textUsed += last - text;
    return;
 nomem:
    CcsErrorPool_Error(self->errpool,
		       XML_GetCurrentLineNumber(self->parser),
		       XML_GetCurrentColumnNumber(self->parser),
		       "Not enough memory, some text lost");
}

static void
CXS_Text2Tokens(CcxScanOper_t * self)
{
    char * cur; CcsToken_t * lastT;
    if (self->textStart == self->textUsed) return;
    if (isspace(*self->textStart)) { /* All whitespace. */
	if (self->kindWhitespace >= 0)
	    CXS_Append(self, CXS_GetLastToken(self), self->kindWhitespace,
		       self->textStart, self->textUsed - self->textStart);
    } else {
	for (cur = self->textUsed - 1; isspace(*cur); --cur);
	++cur;
	lastT = CXS_Append(self, CXS_GetLastToken(self), self->kindText,
			   self->textStart, cur - self->textStart);
	if (cur < self->textUsed)
	    CXS_Append(self, lastT, self->kindWhitespace,
		       cur, self->textUsed - cur);
    }
    self->textUsed = self->textStart;
}

static int
cmpXmlSpec(const void * name, const void * spec)
{
    const char * localname = strchr(name, nsSep);
    const CcxSpec_t * ccspec = (const CcxSpec_t *)spec;
    return localname == NULL ? strcmp("", ccspec->nsURI) :
	strncmp(name, ccspec->nsURI, localname - (const char *)name);
}

static int
cmpXmlTag(const void * localname, const void * tag)
{
    const CcxTag_t * cctag = (const CcxTag_t *)tag;
    return strcmp(localname, cctag->name);
}
static int
casecmpXmlTag(const void * localname, const void * tag)
{
    const CcxTag_t * cctag = (const CcxTag_t *)tag;
    return strcasecmp(localname, cctag->name);
}

static int
cmpXmlAttr(const void * localname, const void * attr)
{
    const CcxAttr_t * ccattr = (const CcxAttr_t *)attr;
    return strcmp(localname, ccattr->name);
}
static int
casecmpXmlAttr(const void * localname, const void * attr)
{
    const CcxAttr_t * ccattr = (const CcxAttr_t *)attr;
    return strcasecmp(localname, ccattr->name);
}

static int
cmpXmlPI(const void * localname, const void * pi)
{
    const CcxPInstruction_t * ccpi = (const CcxPInstruction_t *)pi;
    return strcmp(localname, ccpi->name);
}
static int
casecmpXmlPI(const void * localname, const void * pi)
{
    const CcxPInstruction_t * ccpi = (const CcxPInstruction_t *)pi;
    return strcasecmp(localname, ccpi->name);
}
