/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang
  Author: Charles Wang <charlesw123456@gmail.com>
  License: LGPLv2 (see LICENSE-LGPL)
-------------------------------------------------------------------------*/
#ifndef  COCO_CDEFS_H
#define  COCO_CDEFS_H

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

#ifdef  __cplusplus
#define EXTC_BEGIN extern "C" {
#define EXTC_END   }
#else
#define EXTC_BEGIN
#define EXTC_END
#endif

EXTC_BEGIN

typedef char CcsBool_t;
#define FALSE 0
#define TRUE  1

#ifdef  NDEBUG
#define CcsAssert(e)  ((void)0)
#else /* NDEBUG */
#define CcsAssert(e)  ((e) ? (void)0 : _CcsAssertFailed_(#e, __FILE__, __LINE__))
#endif /* NDEBUG */
void _CcsAssertFailed_(const char * vstr, const char * fname, int line);

/* C Scheme types */
typedef struct CcsLocation_s CcsLocation_t;
typedef struct CcsErrorPool_s CcsErrorPool_t;
typedef struct CcsToken_s CcsToken_t;
typedef struct CcsPosition_s CcsPosition_t;
typedef struct CcsIncPathList_s CcsIncPathList_t;
typedef struct CcsScanInput_s CcsScanInput_t;

typedef enum {
    XSO_UnknownTag, XSO_UnknownTagEnd,
    XSO_UnknownAttr, XSO_UnknownProcessingInstruction,
    XSO_Text, XSO_Whitespace, XSO_Comment,
    XSO_UT_Text, XSO_UT_Whitespace, XSO_UT_Comment,
    XSO_UNS_Text, XSO_UNS_Whitespace, XSO_UNS_Comment,
    XSO_SIZE
}  CcsXmlSpecOption_t;
extern const char * CcsXmlSpecOptionNames[];

#define CcsMalloc(size)  _CcsMalloc_(size, __FILE__, __LINE__)
void * _CcsMalloc_(size_t size, const char * fname, int line);

#define CcsRealloc(ptr, size)  _CcsRealloc_(ptr, size, __FILE__, __LINE__)
void * _CcsRealloc_(void * ptr, size_t size, const char * fname, int line);

#define CcsFree(ptr)  _CcsFree_(ptr, __FILE__, __LINE__)
void _CcsFree_(void * ptr, const char * fname, int line);

#define CcsStrdup(str)  _CcsStrdup_(str, __FILE__, __LINE__)
char * _CcsStrdup_(const char * str, const char * fname, int line);

#define COCO_WCHAR_MAX 65535
#define EoF            -1
#define ErrorChr       -2
#define NoChr          -3

int CcsUTF8GetCh(const char ** str, const char * stop);
int CcsUTF8GetWidth(const char * str, size_t len);
int CcsUnescapeCh(const char ** str, const char * stop);
char * CcsUnescape(const char * str);
char * CcsEscape(const char * str);

/* Path operations */
char * CcsPathJoin(char * result, size_t szresult, const char * path, ...);
char * CcsDirname(char * result, size_t szresult, const char * path);
char * CcsBasename(char * result, size_t szresult, const char * path);

EXTC_END

#endif  /* COCO_CDEFS_H */
