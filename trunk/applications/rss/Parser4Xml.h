/*---- license ----*/
/*-------------------------------------------------------------------------
  rss.xatg -- RSS(Really Simple Syndication) Grammer
  Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
/*---- enable ----*/
#ifndef  COCO_RssParser_H
#define  COCO_RssParser_H

#ifndef  COCO_ERRORPOOL_H
#include "c/ErrorPool.h"
#endif

#ifndef  COCO_RssScanner_H
#include "Scanner4Xml.h"
#endif

/*---- hIncludes ----*/
#ifndef  COCO_RSSDATA_H
#include  "RssData.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

/*---- SynDefines ----*/
/*---- enable ----*/

typedef struct {
    CcsErrorPool_t    errpool;
    RssScanner_t      scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    CcRss_t rss;
    /*---- enable ----*/
}  RssParser_t;

RssParser_t * RssParser(RssParser_t * self, FILE * infp, FILE * errfp);
RssParser_t * RssParser_ByName(RssParser_t * self, const char * infp,
			       FILE * errfp);
void RssParser_Destruct(RssParser_t * self);
void RssParser_Parse(RssParser_t * self);
CcsBool_t RssParser_Finish(RssParser_t * self);

void RssParser_SemErr(RssParser_t * self, const CcsToken_t * token,
		      const char * format, ...);
void RssParser_SemErrT(RssParser_t * self, const char * format, ...);

EXTC_END

#endif /* COCO_RssParser_H */
