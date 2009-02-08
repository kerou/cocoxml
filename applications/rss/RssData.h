/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#ifndef  COCO_RSSDATA_H
#define  COCO_RSSDATA_H

#ifndef  COCO_OBJECT_H
#include  "Object.h"
#endif

#ifndef  COCO_ARRAYLIST_H
#include  "ArrayList.h"
#endif

EXTC_BEGIN

typedef struct {
    CcObject_t base;
    char * domain;
    char * port;
    char * path;
    char * registerProcedure;
    char * protocol;
}  CcCloud_t;

CcCloud_t * CcCloud(void);

typedef struct {
    CcObject_t base;
    char * url;
    char * title;
    char * link;
    char * width;
    char * height;
}  CcImage_t;

CcImage_t * CcImage(void);

typedef struct {
    CcObject_t base;
    char * title;
    char * description;
    char * name;
    char * link;
}  CcTextInput_t;

CcTextInput_t * CcTextInput(void);

typedef struct {
    CcObject_t base;
    char * title;
    char * link;
    char * description;
    char * author;
    char * category;
    char * comments;
    char * enclosure;
    char * guid;
    char * pubdate;
    char * source;
    CcImage_t * image;
}  CcItem_t;

CcItem_t * CcItem(void);

typedef struct {
    CcObject_t base;
    char * title;
    char * link;
    char * description;
    char * language;
    char * copyright;
    char * managingEditor;
    char * webMaster;
    char * pubDate;
    char * lastBuildDate;
    char * category;
    char * generator;
    char * docs;
    CcCloud_t * cloud;
    char * ttl;
    CcImage_t * image;
    char * rating;
    CcTextInput_t * textInput;
    char * skipHours;
    char * skipDays;
    CcArrayList_t itemList;
}  CcChannel_t;

CcChannel_t * CcChannel(void);
void CcChannel_AddItem(CcChannel_t * self, CcItem_t * item);

typedef struct {
    CcArrayList_t channelList;
} CcRss_t;

CcRss_t * CcRss(CcRss_t * self);
void CcRss_AddChannel(CcRss_t * self, CcChannel_t * channel);
void CcRss_Destruct(CcRss_t * self);

EXTC_END

#endif  /* COCO_RSSDATA_H */
