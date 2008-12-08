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

static void
CXS_StartElement(void * self, const XML_Char * name, const XML_Char ** attrs)
{
    const XML_Char ** curattr;
    const CcxSpec_t * tagSpec, * attrSpec;
    const CcxTag_t * tag; const CcxAttr_t * attr;
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;
    CcsToken_t * last = CXS_GetLastToken(ccself);
    const char * localname = strchr(name, nsSep);

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
    if (ccself->curSpecStack - ccself->specStack < SZ_SPECSTACK) {
	*ccself->curSpecStack = tagSpec;
    } else {
	CcsErrorPool_Error(ccself->errpool,
			   XML_GetCurrentLineNumber(ccself->parser),
			   XML_GetCurrentColumnNumber(ccself->parser),
			   "XML Tag too deep(limit = %d)", SZ_SPECSTACK);
    }
    ++ccself->curSpecStack;
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
    CcsAssert(ccself->specStack < ccself->curSpecStack);
    --ccself->curSpecStack;
#ifndef NDEBUG
    if (ccself->curSpecStack - ccself->specStack < SZ_SPECSTACK)
	CcsAssert(tagSpec == *ccself->curSpecStack);
#endif
}

static void
CXS_CharacterData(void * self, const XML_Char * s, int len)
{
    CcsBool_t SpecFound, InUnknownNS;
    const CcxSpec_t ** spec;
    int kindText, kindWhitespace;
    CcsToken_t * last;
    const char * s0, * s1, * s2;
    CcxScanOper_t * ccself = (CcxScanOper_t *)self;

    SpecFound = FALSE; InUnknownNS = FALSE;
    spec = ccself->curSpecStack;
    while (spec > ccself->specStack) {
	--spec;
	if (*spec) {
	    SpecFound = TRUE;
	    if (InUnknownNS) {
		kindText = (*spec)->kinds[XSO_UNS_Text];
		kindWhitespace = (*spec)->kinds[XSO_UNS_Whitespace];
	    } else {
		kindText = (*spec)->kinds[XSO_Text];
		kindWhitespace = (*spec)->kinds[XSO_Whitespace];
	    }
	    break;
	}
	InUnknownNS = TRUE;
    }
    if (!SpecFound) return;
    last = CXS_GetLastToken(ccself);
    s0 = s;
    /* For leading space. */
    for (s2 = s0; s2 - s < len; ++s2)
	if (!isspace(*s2)) break;
    if (s0 < s2) {
	last = CXS_Append(ccself, last, kindWhitespace, s0, s2 - s0);
	s0 = s2;
    }
    s1 = s0;
    for (s2 = s0; s2 - s < len; ++s2)
	if (!isspace(*s2)) s1 = s2 + 1;
    /* For text. */
    if (s0 < s1) last = CXS_Append(ccself, last, kindText, s0, s1 - s0);
    /* For tailing space. */
    if (s1 < s2) last = CXS_Append(ccself, last, kindWhitespace, s1, s2 - s1);
}

static void
CXS_PInstruction(void * self, const XML_Char * target, const XML_Char * data)
{
}

static void
CXS_Comment(void * self, const XML_Char * data)
{
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
    if (!(self->dummy = CcsToken(0, 0, 0, 0, dummyval, strlen(dummyval))))
	goto errquit1;
    self->tokens = self->peek = NULL;
    CcxScanOper_IncRef(self, self->dummy);
    self->curSpecStack = self->specStack;
    return self;
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

    while (self->tokens == NULL) {
	rc = fread(buf, 1, sizeof(buf), self->fp);
	if (ferror(self->fp)) {
	    fprintf(stderr, "Error in reading.\n");
	    return FALSE;
	}
	status = XML_Parse(self->parser, buf, rc, rc < sizeof(buf));
	if (status != XML_STATUS_OK &&
	    (error = XML_GetErrorCode(self->parser)) != XML_ERROR_FINISHED) {
	    fprintf(stderr, "XML Parse error: %s.\n", XML_ErrorString(error));
	    return FALSE;
	}
	if (feof(self->fp)) return FALSE;
    }
    return TRUE;
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
