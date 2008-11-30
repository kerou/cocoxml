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

static void
CXS_StartElement(void * self, const XML_Char * name, const XML_Char ** attrs)
{
}

static void
CXS_EndElement(void * self, const XML_Char * name)
{
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
CcsXmlScanner(CcsXmlScanner_t * self,
	      CcsXmlGlobals_t * globals,
	      const char * filename,
	      const CcsXmlSpec_t * firstspec,
	      const CcsXmlSpec_t * lastspec)
{
    self->globals = globals;
    self->firstspec = firstspec;
    self->lastspec = lastspec;
    self->parser = XML_ParserCreateNS(NULL, ':');
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
    if (!(self->peek = self->tokens = self->dummy =
	  CcsToken(0, 0, 0, 0, dummyval, strlen(dummyval))))
	goto errquit1;
    CcsXmlScanner_IncRef(self, self->dummy);
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

    while (self->tokens == NULL) {
	rc = fread(buf, 1, sizeof(buf), self->fp);
	if (ferror(self->fp)) {
	    fprintf(stderr, "Error in reading.\n");
	    return FALSE;
	}
	status = XML_Parse(self->parser, buf, rc, rc < sizeof(buf));
	if (status != XML_STATUS_OK) {
	    fprintf(stderr, "XML Parse error: %s.\n",
		    XML_ErrorString(XML_GetErrorCode(self->parser)));
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
	if (!CcsXmlScanner_Parse(self)) return NULL;
    token = self->tokens;
    if (self->peek == self->tokens)
	self->tokens = self->peek = self->tokens->next;
    else
	self->tokens = self->tokens->next;
    return token;
}

CcsToken_t *
CcsXmlScanner_Peek(CcsXmlScanner_t * self)
{
    CcsToken_t * token;
    while (self->peek == NULL)
	if (!CcsXmlScanner_Parse(self)) return NULL;
    token = self->peek;
    self->peek = self->peek->next;
    CcsXmlScanner_IncRef(self, token);
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
