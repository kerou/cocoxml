/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_ARGUMENTS_H
#define  COCO_ARGUMENTS_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

#ifndef  COCO_HASHTABLE_H
#include  "HashTable.h"
#endif

EXTC_BEGIN

typedef struct {
    char         ch;
    const char * key;
    const char * promptValue; /* If it is NULL, this option do not accept any value. */
    const char * defaultValue; /* Only-used when promptValue == NULL. */
    const char * help;
}  CcArgDesc_t;

struct CcArguments_s {
    const char * selfpath;
    CcArrayList_t storage;
    CcHashTable_t map;
};

CcArguments_t *
CcArguments(CcArguments_t * self,
	    const CcArgDesc_t * desc, const CcArgDesc_t * descLast,
	    int argc, char * argv[]);
void CcArguments_Destruct(CcArguments_t * self);

typedef struct {
    const void * next;
}  CcArgumentsIter_t;

const char * CcArguments_First(CcArguments_t * self, const char * key,
			       CcArgumentsIter_t * iter);
const char * CcArguments_Next(CcArguments_t * self, CcArgumentsIter_t * iter);

void
CcArguments_ShowHelp(FILE * outfp, const CcArgDesc_t * desc,
		     const CcArgDesc_t * descLast);

EXTC_END

#endif  /* COCO_ARGUMENTS_H */
