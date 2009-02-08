/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_XMLSPEC_H
#define  COCO_XMLSPEC_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

#ifndef  COCO_BITARRAY_H
#include  "BitArray.h"
#endif

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

EXTC_BEGIN

struct CcXmlSpec_s {
    CcObject_t base;

    CcGlobals_t * globals;
    CcsBool_t caseSensitive;
    CcBitArray_t options;
    CcArrayList_t Tags;
    CcArrayList_t Attrs;
    CcArrayList_t PInstructions;
};

CcXmlSpec_t * CcXmlSpec(CcGlobals_t * globals);

void CcXmlSpec_SetCaseSensitive(CcXmlSpec_t * self, CcsBool_t caseSensitive);
void CcXmlSpec_SetOption(CcXmlSpec_t * self, const CcsToken_t * token);
void CcXmlSpec_AddTag(CcXmlSpec_t * self, const char * tokenName,
		      const CcsToken_t * token);
void CcXmlSpec_AddAttr(CcXmlSpec_t * self, const char * tokenName,
		       const CcsToken_t * token);
void
CcXmlSpec_AddProcessInstruction(CcXmlSpec_t * self, const char * tokenName,
				const CcsToken_t * token);

void
CcXmlSpec_MakeTerminals(const CcXmlSpec_t * self, CcGlobals_t * globals);

typedef struct {
    char * name;
    int kind0;
    int kind1;
}  CcXmlSpecData_t;

CcXmlSpecData_t *
CcXmlSpec_GetSortedTagList(const CcXmlSpec_t * self,
			   const CcGlobals_t * globals, size_t * retNum);
CcXmlSpecData_t *
CcXmlSpec_GetSortedAttrList(const CcXmlSpec_t * self,
			    const CcGlobals_t * globals, size_t * retNum);
CcXmlSpecData_t *
CcXmlSpec_GetSortedPIList(const CcXmlSpec_t * self,
			  const CcGlobals_t * globals, size_t * retNum);

void CcXmlSpecData_Destruct(CcXmlSpecData_t * self, size_t num);

EXTC_END

#endif /* COCO_XMLSPEC_H */
