/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Globals.h"
#include  "c/ErrorPool.h"

CcGlobals_t *
CcGlobals(CcGlobals_t * self, CcsErrorPool_t * errpool)
{
    self->errpool = errpool;
    self->templatePrefix = "Ccs";
    if (!CcSymbolTable(&self->symtab)) goto errquit1;
    if (!(self->lexical = CcLexical(&self->u.lexicalSpace, self)))
	goto errquit2;
    self->xmlspecmap = NULL;
    if (!CcSyntax(&self->syntax, self)) goto errquit3;
    if (!CcArrayList(&self->sections)) goto errquit4;
    return self;
 errquit4:
    CcSyntax_Destruct(&self->syntax);
 errquit3:
    CcLexical_Destruct(self->lexical);
 errquit2:
    CcSymbolTable_Destruct(&self->symtab);
 errquit1:
    return NULL;
}

CcGlobals_t *
CcGlobalsXml(CcGlobals_t * self, CcsErrorPool_t * errpool)
{
    self->errpool = errpool;
    self->templatePrefix = "Ccx";
    if (!CcSymbolTable(&self->symtab)) goto errquit1;
    self->lexical = NULL;
    if (!(self->xmlspecmap = CcXmlSpecMap(&self->u.xmlspecmapSpace, self)))
	goto errquit2;
    if (!CcSyntax(&self->syntax, self)) goto errquit3;
    if (!CcArrayList(&self->sections)) goto errquit4;
    return self;
 errquit4:
    CcSyntax_Destruct(&self->syntax);
 errquit3:
    CcXmlSpecMap_Destruct(self->xmlspecmap);
 errquit2:
    CcSymbolTable_Destruct(&self->symtab);
 errquit1:
    return NULL;
}

void
CcGlobals_Destruct(CcGlobals_t * self)
{
    CcArrayList_Destruct(&self->sections);
    CcSyntax_Destruct(&self->syntax);
    if (self->lexical) CcLexical_Destruct(self->lexical);
    if (self->xmlspecmap) CcXmlSpecMap_Destruct(self->xmlspecmap);
    CcSymbolTable_Destruct(&self->symtab);
}

CcsBool_t
CcGlobals_Finish(CcGlobals_t * self)
{
    if (self->errpool->errorCount > 0) return FALSE;
    if (!CcSymbolTable_Finish(&self->symtab)) return FALSE;
    if (self->lexical && !CcLexical_Finish(self->lexical)) return FALSE;
    if (self->xmlspecmap && !CcXmlSpecMap_Finish(self->xmlspecmap))
	return FALSE;
    if (!CcSyntax_Finish(&self->syntax)) return FALSE;
    return TRUE;
}

typedef struct {
    CcObject_t base;
    char * name;
    CcsPosition_t * pos;
}  CcSection_t;

static void
CcSection_Destruct(CcObject_t * self)
{
    CcSection_t * ccself = (CcSection_t *)self;

    CcsPosition_Destruct(ccself->pos);
    CcFree(ccself->name);
    CcObject_Destruct(self);
}

static const CcObjectType_t SectionType = {
    sizeof(CcSection_t), "Section", CcSection_Destruct
};

void
CcGlobals_NewSection(CcGlobals_t * self, const char * secname,
		     CcsPosition_t * pos)
{
    CcSection_t * section = (CcSection_t *)
	CcArrayList_New(&self->sections, CcObject(&SectionType));
    section->name = CcStrdup(secname);
    section->pos = pos;
}

const CcsPosition_t *
CcGlobals_FirstSection(const CcGlobals_t * self, const char * secname,
		       CcArrayListIter_t * iter)
{
    const CcSection_t * sec;
    for (sec = (const CcSection_t *)CcArrayList_FirstC(&self->sections, iter);
	 sec;
	 sec = (const CcSection_t *)CcArrayList_NextC(&self->sections, iter))
	if (!strcmp(sec->name, secname)) return sec->pos;
    return NULL;
}
const CcsPosition_t *
CcGlobals_NextSection(const CcGlobals_t * self, const char * secname,
		      CcArrayListIter_t * iter)
{
    const CcSection_t * sec;
    for (sec = (const CcSection_t *)CcArrayList_NextC(&self->sections, iter);
	 sec;
	 sec = (const CcSection_t *)CcArrayList_NextC(&self->sections, iter))
	if (!strcmp(sec->name, secname)) return sec->pos;
    return NULL;
}
