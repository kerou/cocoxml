/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_XMLSPECMAP_H
#define  COCO_XMLSPECMAP_H

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

#ifndef  COCO_HASHTABLE_H
#include  "HashTable.h"
#endif

EXTC_BEGIN

struct CcXmlSpecMap_s {
    CcGlobals_t   * globals;
    int             kindUnknownNS;
    CcArrayList_t   storage;
    CcHashTable_t   map;
};

CcXmlSpecMap_t * CcXmlSpecMap(CcXmlSpecMap_t * self, CcGlobals_t * globals);
void CcXmlSpecMap_Destruct(CcXmlSpecMap_t * self);

CcsBool_t
CcXmlSpecMap_Add(CcXmlSpecMap_t * self, const char * nsURI,
		 CcXmlSpec_t * xmlspec);

void CcXmlSpecMap_MakeTerminals(const CcXmlSpecMap_t * self,
				CcGlobals_t * symtab);
CcsBool_t CcXmlSpecMap_Finish(CcXmlSpecMap_t * self);

void
CcXmlSpecMap_GetOptionKinds(const CcXmlSpecMap_t * self, int * kinds,
			    const CcGlobals_t * globals);

EXTC_END

#endif /* COCO_XMLSPECMAP_H */
