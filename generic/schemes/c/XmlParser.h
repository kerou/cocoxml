/*---- license ----*/
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
/*---- enable ----*/
#ifndef  COCO_XMLPARSER_H
#define  COCO_XMLPARSER_H

#ifndef  COCO_CDEFS_H
#include "c/CDefs.h"
#endif

/*---- hIncludes ----*/
#ifndef  COCO_DEFS_H
#include  "Defs.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

struct CcsXmlParser_s {
    CcsGlobals_t    * globals;
    CcsXmlScanner_t * scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    char            * schemeName;
    CcsPosition_t   * members;
    CcsPosition_t   * constructor;
    CcsPosition_t   * destructor;
    /* Shortcut pointers */
    CcSymbolTable_t * symtab;
    CcXmlSpecMap_t  * xmlspecmap;
    CcSyntax_t      * syntax;
    /*---- enable ----*/
};

CcsXmlParser_t * CcsXmlParser(CcsXmlParser_t * self, CcsGlobals_t * globals);
void CcsXmlParser_Destruct(CcsXmlParser_t * self);
void CcsXmlParser_Parse(CcsXmlParser_t * self);

EXTC_END

#endif /* COCO_XMLPARSER_H */
