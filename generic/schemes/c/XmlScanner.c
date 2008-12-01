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
#include  "c/XmlScanner.h"
#include  "c/Token.h"
#include  "c/CXmlGlobals.h"
#include  "c/ErrorPool.h"

static const char nsSep = '@';

static CcsToken_t *
CXS_GetLastToken(CcsXmlScanner_t * self)
{
    CcsToken_t * cur;
    if (self->tokens == NULL) return NULL;
    for (cur = self->tokens; cur->next; cur = cur->next);
    return cur;
}

static CcsToken_t *
CXS_Append(CcsXmlScanner_t * self, CcsToken_t * last,
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
    const CcsXmlSpec_t * ccspec = (const CcsXmlSpec_t *)spec;
    return localname == NULL ? strcmp("", ccspec->nsURI) :
	strncmp(name, ccspec->nsURI, localname - (const char *)name);
}

static int
cmpXmlTag(const void * localname, const void * tag)
{
    const CcsXmlTag_t * cctag = (const CcsXmlTag_t *)tag;
    return strcmp(localname, cctag->name);
}
static int
casecmpXmlTag(const void * localname, const void * tag)
{
    const CcsXmlTag_t * cctag = (const CcsXmlTag_t *)tag;
    return strcasecmp(localname, cctag->name);
}

static int
cmpXmlAttr(const void * localname, const void * attr)
{
    const CcsXmlAttr_t * ccattr = (const CcsXmlAttr_t *)attr;
    return strcmp(localname, ccattr->name);
}
static int
casecmpXmlAttr(const void * localname, const void * attr)
{
    const CcsXmlAttr_t * ccattr = (const CcsXmlAttr_t *)attr;
    return strcasecmp(localname, ccattr->name);
}

static int
cmpXmlPI(const void * localname, const void * pi)
{
    const CcsXmlPInstruction_t * ccpi = (const CcsXmlPInstruction_t *)pi;
    return strcmp(localname, ccpi->name);
}
static int
casecmpXmlPI(const void * localname, const void * pi)
{
    const CcsXmlPInstruction_t * ccpi = (const CcsXmlPInstruction_t *)pi;
    return strcasecmp(localname, ccpi->name);
}

static void
CXS_StartElement(void * self, const XML_Char * name, const XML_Char ** attrs)
{
    const XML_Char ** curattr;
    const CcsXmlSpec_t * tagSpec, * attrSpec;
    const CcsXmlTag_t * tag; const CcsXmlAttr_t * attr;
    CcsXmlScanner_t * ccself = (CcsXmlScanner_t *)self;
    CcsToken_t * last = CXS_GetLastToken(ccself);
    const char * localname = strchr(name, nsSep);

    localname = localname ? localname + 1 : name;
    if (!(tagSpec = (const CcsXmlSpec_t *)
	  bsearch(name, ccself->firstspec, ccself->numspecs,
		  sizeof(CcsXmlSpec_t), cmpXmlSpec))) {
	last = CXS_Append(ccself, last, ccself->kindUnknownNS, NULL, 0);
    } else if (!(tag = (const CcsXmlTag_t *)
		 bsearch(localname, tagSpec->firstTag, tagSpec->numTags, sizeof(CcsXmlTag_t),
			 tagSpec->caseSensitive ? cmpXmlTag : casecmpXmlTag))) {
	last = CXS_Append(ccself, last,
			     tagSpec->kinds[XSO_UnknownTag], NULL, 0);
    } else {
	last = CXS_Append(ccself, last, tag->kind, NULL, 0);
    }
    if (ccself->curSpecStack - ccself->specStack < SZ_SPECSTACK) {
	*ccself->curSpecStack = tagSpec;
    } else {
	CcsErrorPool_Error(&ccself->globals->error,
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
	    attrSpec = (const CcsXmlSpec_t *)
		bsearch(curattr[0], ccself->firstspec, ccself->numspecs,
			sizeof(CcsXmlSpec_t), cmpXmlSpec);
	    ++localname;
	}
	if (!attrSpec) {
	    last = CXS_Append(ccself, last,
				 ccself->kindUnknownNS, NULL, 0);
	} else if (!(attr = (const CcsXmlAttr_t *)
		     bsearch(localname, attrSpec->firstAttr, attrSpec->numAttrs, sizeof(CcsXmlAttr_t),
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
    const CcsXmlSpec_t * tagSpec;
    const CcsXmlTag_t * tag;
    CcsXmlScanner_t * ccself = (CcsXmlScanner_t *)self;
    CcsToken_t * last = CXS_GetLastToken(ccself);
    const char * localname = strchr(name, nsSep);

    localname = localname ? localname + 1 : name;
    if (!(tagSpec = (const CcsXmlSpec_t *)
	  bsearch(name, ccself->firstspec, ccself->numspecs,
		  sizeof(CcsXmlSpec_t), cmpXmlSpec))) {
	CXS_Append(ccself, last, ccself->kindUnknownNS, NULL, 0);
    } else if (!(tag = (const CcsXmlTag_t *)
		 bsearch(localname, tagSpec->firstTag, tagSpec->numTags, sizeof(CcsXmlTag_t),
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
CcsXmlScanner_t *
CcsXmlScanner(CcsXmlScanner_t * self, CcsXmlGlobals_t * globals,
	      const char * filename, int kindUnknownNS,
	      const CcsXmlSpec_t * firstspec, size_t numspecs)
{
    self->globals = globals;
    self->kindUnknownNS = kindUnknownNS;
    self->firstspec = firstspec;
    self->numspecs = numspecs;
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
    CcsXmlScanner_IncRef(self, self->dummy);
    self->curSpecStack = self->specStack;
    return self;
 errquit1:
    fclose(self->fp);
 errquit0:
    XML_ParserFree(self->parser);
    return NULL;
}

void
CcsXmlScanner_Destruct(CcsXmlScanner_t * self)
{
    CcsToken_t * cur, * next;

    for (cur = self->tokens; cur; cur = next) {
	next = cur->next;
	CcsXmlScanner_DecRef(self, cur);
    }
    CcsXmlScanner_DecRef(self, self->dummy);
    fclose(self->fp);
    XML_ParserFree(self->parser);
}

CcsToken_t *
CcsXmlScanner_GetDummy(CcsXmlScanner_t * self)
{
    CcsXmlScanner_IncRef(self, self->dummy);
    return self->dummy;
}

static CcsBool_t
CcsXmlScanner_Parse(CcsXmlScanner_t * self)
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
CcsXmlScanner_Scan(CcsXmlScanner_t * self)
{
    CcsToken_t * token;
    while (self->tokens == NULL)
	if (!CcsXmlScanner_Parse(self)) break;
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
CcsXmlScanner_Peek(CcsXmlScanner_t * self)
{
    CcsToken_t * token;
    while (self->peek == NULL)
	if (!CcsXmlScanner_Parse(self)) return NULL;
    token = self->peek;
    if (token) {
	self->peek = self->peek->next;
	CcsXmlScanner_IncRef(self, token);
    }
    return token;
}

void
CcsXmlScanner_ResetPeek(CcsXmlScanner_t * self)
{
    self->peek = self->tokens;
}

void
CcsXmlScanner_IncRef(CcsXmlScanner_t * self, CcsToken_t * token)
{
    ++token->refcnt;
}

void
CcsXmlScanner_DecRef(CcsXmlScanner_t * self, CcsToken_t * token)
{
    if (--token->refcnt > 0) return;
    CcsToken_Destruct(token);
}
