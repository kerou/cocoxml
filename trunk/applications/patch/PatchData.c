/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#include  "PatchData.h"

static PatchLine_t *
PatchLine(const char * content, size_t clen)
{
    PatchLine_t * self;
    if (!(self = CcsMalloc(sizeof(PatchLine_t) + clen + 1))) return NULL;
    self->next = NULL;
    self->content = (char *)(self + 1);
    memcpy(self->content, content, clen);
    self->content[clen] = 0;
    return self;
}

static void
PatchLine_Destruct(PatchLine_t * self)
{
    CcsFree(self);
}

PatchLine_t *
PatchLineList(PatchScanner_t * scanner, int subStart, int subNum, int addStart,
	      int addNum, CcsBool_t * subLastEol, CcsBool_t * addLastEol)
{
    long start; size_t len;
    PatchLine_t * first, * last;
    const char * line; PatchLine_t * patchline;

    *subLastEol = *addLastEol = TRUE;
    first = last = NULL;
    while (subNum > 0 || addNum > 0) {
	len = 65536;
	start = PatchScanner_StringTo(scanner, &len, "\n");
	if (start < 0) {
	    PatchScanner_Fatal(scanner, "Error encountered");
	} else if (len == 0) {
	    PatchScanner_Error(scanner, "Broken patch");
	    break;
	}
	line = PatchScanner_GetString(scanner, start, len);
	switch (*line) {
	case ' ':
	    if (subNum > 0 && addNum > 0) { --subNum; --addNum; break; }
	    PatchScanner_Error(scanner, "Broken patch");
	    return first;
	case '-':
	    if (subNum > 0) { --subNum; break; }
	    PatchScanner_Error(scanner, "Broken patch");
	    return first;
	case '+':
	    if (addNum > 0) { --addNum; break; }
	    PatchScanner_Error(scanner, "Broken patch");
	    return first;
	case '\\':
	    if (subNum == 0) { *subLastEol = FALSE; break; }
	    PatchScanner_Error(scanner, "Broken patch");
	    return first;
	}
	if (!(patchline = PatchLine(line, len)))
	    PatchScanner_Fatal(scanner, "Not enough memory");
	PatchScanner_Consume(scanner, start, len);
	if (first == NULL) first = last = patchline;
	else { last->next = patchline; last = patchline; }
    }
    start = PatchScanner_StringTo(scanner, &len, "\n");
    if (start < 0) PatchScanner_Fatal(scanner, "Error encountered");
    if (len > 0 && (line = PatchScanner_GetString(scanner, start, len)) &&
	*line == '\\') {
	*addLastEol = FALSE;
	if (!(patchline = PatchLine(line, len)))
	    PatchScanner_Fatal(scanner, "Not enough memory");
	PatchScanner_Consume(scanner, start, len);
	if (first == NULL) first = last = patchline;
	else { last->next = patchline; last = patchline; }
    }
    return first;
}

void
PatchLineList_Destruct(PatchLine_t * self)
{
    PatchLine_t * next;
    while (self) {
	next = self->next;
	PatchLine_Destruct(self);
	self = next;
    }
}

PatchPiece_t *
PatchPiece(int subStart, int subNum, int addStart, int addNum,
	   PatchLine_t * lines, CcsBool_t subLastEol, CcsBool_t addLastEol)
{
    PatchPiece_t * self;
    if (!(self = CcsMalloc(sizeof(PatchPiece_t)))) return NULL;
    self->next = NULL;
    self->subStart = subStart;
    self->subNum = subNum;
    self->addStart = addStart;
    self->addNum = addNum;
    self->lines = lines;
    self->subLastEol = subLastEol;
    self->addLastEol = addLastEol;
    return self;
}

void
PatchPiece_Destruct(PatchPiece_t * self)
{
    PatchLineList_Destruct(self->lines);
    CcsFree(self);
}

PatchFile_t *
PatchFile(const char * subFname, const char * addFname)
{
    PatchFile_t * self;
    if (!(self = CcsMalloc(sizeof(PatchFile_t) + strlen(subFname) + 1
			   + strlen(addFname) + 1)))
	return NULL;
    self->next = NULL;
    self->subFname = (char *)(self + 1);
    strcpy(self->subFname, subFname);
    self->addFname = self->subFname + strlen(self->subFname) + 1;
    strcpy(self->addFname, addFname);
    self->firstPiece = self->lastPiece = NULL;
    return self;
}

void
PatchFile_Destruct(PatchFile_t * self)
{
    PatchPiece_t * cur, * next;
    for (cur = self->firstPiece; cur; cur = next) {
	next = cur->next;
	PatchPiece_Destruct(cur);
    }
    CcsFree(self);
}

void
PatchFile_Append(PatchFile_t * self, PatchPiece_t * piece)
{
    if (self->lastPiece) {
	self->lastPiece->next = piece; self->lastPiece = piece;
    } else {
	self->firstPiece = self->lastPiece = piece;
    }
}
