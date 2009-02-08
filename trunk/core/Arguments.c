/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: GPLv2 (see LICENSE-GPL)
-------------------------------------------------------------------------*/
#include  "Arguments.h"

typedef struct CcArg_s CcArg_t;
struct CcArg_s {
    CcObject_t base;
    CcArg_t * next;
    const char * value;
};

static const CcObjectType_t CcArgType = {
    sizeof(CcArg_t), "Arg", CcObject_Destruct
};

static void
CcArguments_Append(CcArguments_t * self, const char * key, const char * value)
{
    CcArg_t * origArg = (CcArg_t *)CcHashTable_Get(&self->map, key);
    CcArg_t * newArg = (CcArg_t *)
	CcArrayList_New(&self->storage, CcObject(&CcArgType));
    newArg->value = value;
    if (origArg == NULL) {
	CcHashTable_Set(&self->map, key, (CcObject_t *)newArg);
    } else {
	while (origArg->next != NULL) origArg = origArg->next;
	origArg->next = newArg;
    }
}

CcArguments_t *
CcArguments(CcArguments_t * self,
	    const CcArgDesc_t * desc, const CcArgDesc_t * descLast,
	    int argc, char * argv[])
{
    int index;
    const CcArgDesc_t * cur;

    self->selfpath = argv[0];
    CcArrayList(&self->storage);
    CcHashTable(&self->map, 127);
    /* Scan argv. */
    for (index = 1; index < argc; ++index) {
	if (!strcmp(argv[index], "--")) break;
	if (*argv[index] != '-') {
	    CcArguments_Append(self, "", argv[index]);
	    continue;
	}
	if (strlen(argv[index]) != 2) {
	    fprintf(stderr, "Unrecognized option '%s' encountered.\n", argv[index]);
	    continue;
	}
	for (cur = desc; cur < descLast; ++cur)
	    if (cur->ch == argv[index][1]) break;
	if (cur >= descLast) {
	    fprintf(stderr, "Unrecognized option '%s' encountered.\n", argv[index]);
	    continue;
	}
	if (cur->promptValue == NULL) {
	    CcArguments_Append(self, cur->key, cur->defaultValue);
	} else if (index + 1 < argc) {
	    CcArguments_Append(self, cur->key, argv[index + 1]);
	    ++index;
	} else {
	    fprintf(stderr, "The required value of option '%s' is lost.\n", argv[index]);
	    continue;
	}
    }
    for (;index < argc; ++index) CcArguments_Append(self, "", argv[index]);
    return self;
}

void
CcArguments_Destruct(CcArguments_t * self)
{
    CcHashTable_Destruct(&self->map);
    CcArrayList_Destruct(&self->storage);
}

const char *
CcArguments_First(CcArguments_t * self, const char * key,
		  CcArgumentsIter_t * iter)
{
    iter->next = (CcArg_t *)CcHashTable_Get(&self->map, key);
    return CcArguments_Next(self, iter);
}

const char *
CcArguments_Next(CcArguments_t * self, CcArgumentsIter_t * iter)
{
    CcArg_t * arg = (CcArg_t *)iter->next;
    if (arg == NULL) return NULL;
    iter->next = arg->next;
    return arg->value;
}
