/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#include  "Indent.h"
#include  "ScanInput.h"

#define  START_INDENT_SPACE 32
CcsBool_t CcsIndent_Init(CcsIndent_t * self, const CcsIndentInfo_t * info)
{
    self->info = info;
    self->lineStart = TRUE;
    if (!(self->indent = CcsMalloc(sizeof(int) * START_INDENT_SPACE)))
	return FALSE;
    self->indentUsed = self->indent;
    self->indentLast = self->indent + START_INDENT_SPACE;
    *self->indentUsed++ = 0;
    self->indentLimit = -1;
    return TRUE;
}
void CcsIndent_Destruct(CcsIndent_t * self)
{
    CcsFree(self->indent);
}

void CcsIndent_SetLimit(CcsIndent_t * self, const CcsToken_t * indentIn)
{
    self->indentLimit = indentIn->loc.col;
}

CcsToken_t *
CcsIndent_Generator(CcsIndent_t * self, CcsScanInput_t * input)
{
    int newLen; int * newIndent, * curIndent;
    CcsToken_t * head, * cur;

    if (!self->lineStart) return NULL;
    CcsAssert(self->indent < self->indentUsed);
    /* Skip blank lines. */
    if (input->ch == '\r' || input->ch == '\n') return NULL;
    /* Dump all required IndentOut when EoF encountered. */
    if (input->ch == EoF) {
	head = NULL;
	while (self->indent < self->indentUsed - 1) {
	    cur = CcsScanInput_NewToken(input, self->info->kIndentOut);
	    cur->next = head; head = cur;
	    --self->indentUsed;
	}
	return head;
    }
    if (self->indentLimit != -1 && input->col >= self->indentLimit) return NULL;
    self->indentLimit = -1;
    self->lineStart = FALSE;
    if (input->col > self->indentUsed[-1]) {
	if (self->indentUsed == self->indentLast) {
	    newLen = (self->indentLast - self->indent) + START_INDENT_SPACE;
	    newIndent = CcsRealloc(self->indent, sizeof(int) * newLen);
	    if (!newIndent) return NULL;
	    self->indentUsed = newIndent + (self->indentUsed - self->indent);
	    self->indentLast = newIndent + newLen;
	    self->indent = newIndent;
	}
	CcsAssert(self->indentUsed < self->indentLast);
	*self->indentUsed++ = input->col;
	return CcsScanInput_NewToken(input, self->info->kIndentIn);
    }
    for (curIndent = self->indentUsed - 1; input->col < *curIndent; --curIndent);
    if (input->col > *curIndent)
	return CcsScanInput_NewToken(input, self->info->kIndentErr);
    head = NULL;
    while (curIndent < self->indentUsed - 1) {
	cur = CcsScanInput_NewToken(input, self->info->kIndentOut);
	cur->next = head; head = cur;
	--self->indentUsed;
    }
    return head;
}
