/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#ifndef  COCO_CFDATA_H
#define  COCO_CFDATA_H

#ifndef  COCO_CDEFS_H
#include  "c/CDefs.h"
#endif

EXTC_BEGIN

#define  CfNo     0
#define  CfModule 1
#define  CfYes    2

typedef struct CfValue_s CfValue_t;

#ifndef  CFSIZE_VALTAB
#define  CFSIZE_VALTAB  509
#endif

typedef struct {
    CfValue_t ** first;
    CfValue_t ** last;
    CfValue_t  * valueSpace[CFSIZE_VALTAB];
}  CfValueMap_t;

CfValueMap_t * CfValueMap(CfValueMap_t * self);
void CfValueMap_Destruct(CfValueMap_t * self);

const char *
CfValueMap_SetState(CfValueMap_t * self, const char * name, int _state_);
const char *
CfValueMap_SetString(CfValueMap_t * self, const char * name,
		     const char * _string_);
const char *
CfValueMap_SetInt(CfValueMap_t * self, const char * name, const char * value);
const char *
CfValueMap_SetHex(CfValueMap_t * self, const char * name, const char * value);

const char *
CfValueMap_GetState(CfValueMap_t * self, const char * name, int * retState);
const char *
CfValueMap_GetString(CfValueMap_t * self, const char * name,
		     const char ** retString);
const char *
CfValueMap_GetInt(CfValueMap_t * self, const char * name, int * retInt);

EXTC_END

#endif /* COCO_CFDATA_H */
