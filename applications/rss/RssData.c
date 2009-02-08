/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#include  "RssData.h"

/*---------- CcCloud ----------*/
static void
CcCloud_Destruct(CcObject_t * self)
{
    CcCloud_t * ccself = (CcCloud_t *)self;
    if (ccself->domain) CcFree(ccself->domain);
    if (ccself->port) CcFree(ccself->port);
    if (ccself->path) CcFree(ccself->path);
    if (ccself->registerProcedure) CcFree(ccself->registerProcedure);
    if (ccself->protocol) CcFree(ccself->protocol);
    CcObject_Destruct(self);
}

static const CcObjectType_t CloudType = {
    sizeof(CcCloud_t), "Cloud", CcCloud_Destruct
};

CcCloud_t *
CcCloud(void)
{
    return (CcCloud_t *)CcObject(&CloudType);
}

/*---------- CcImage ----------*/
static void
CcImage_Destruct(CcObject_t * self)
{
    CcImage_t * ccself = (CcImage_t *)self;
    if (ccself->url) CcFree(ccself->url);
    if (ccself->title) CcFree(ccself->title);
    if (ccself->link) CcFree(ccself->link);
    if (ccself->width) CcFree(ccself->width);
    if (ccself->height) CcFree(ccself->height);
    CcObject_Destruct(self);
}

static const CcObjectType_t ImageType = {
    sizeof(CcImage_t), "Image", CcImage_Destruct
};

CcImage_t *
CcImage(void)
{
    return (CcImage_t *)CcObject(&ImageType);
}

/*---------- CcTextInput ----------*/
static void
CcTextInput_Destruct(CcObject_t * self)
{
    CcTextInput_t * ccself = (CcTextInput_t *)self;
    if (ccself->title) CcFree(ccself->title);
    if (ccself->description) CcFree(ccself->description);
    if (ccself->name) CcFree(ccself->name);
    if (ccself->link) CcFree(ccself->link);
    CcObject_Destruct(self);
}

static const CcObjectType_t TextInputType = {
    sizeof(CcTextInput_t), "TextInput", CcTextInput_Destruct
};

CcTextInput_t *
CcTextInput(void)
{
    return (CcTextInput_t *)CcObject(&TextInputType);
}

/*---------- CcItem ----------*/
static void
CcItem_Destruct(CcObject_t * self)
{
    CcItem_t * ccself = (CcItem_t *)self;
    if (ccself->title) CcFree(ccself->title);
    if (ccself->link) CcFree(ccself->link);
    if (ccself->description) CcFree(ccself->description);
    if (ccself->author) CcFree(ccself->author);
    if (ccself->category) CcFree(ccself->category);
    if (ccself->comments) CcFree(ccself->comments);
    if (ccself->enclosure) CcFree(ccself->enclosure);
    if (ccself->guid) CcFree(ccself->guid);
    if (ccself->pubdate) CcFree(ccself->pubdate);
    if (ccself->source) CcFree(ccself->source);
    if (ccself->image) CcImage_Destruct((CcObject_t *)ccself->image);
    CcObject_Destruct(self);
}

static const CcObjectType_t ItemType = {
    sizeof(CcItem_t), "Item", CcItem_Destruct
};

CcItem_t *
CcItem(void)
{
    return (CcItem_t *)CcObject(&ItemType);
}

/*---------- CcChannel ----------*/
static void
CcChannel_Destruct(CcObject_t * self)
{
    CcChannel_t * ccself = (CcChannel_t *)self;
    if (ccself->title) CcFree(ccself->title);
    if (ccself->link) CcFree(ccself->link);
    if (ccself->description) CcFree(ccself->description);
    if (ccself->language) CcFree(ccself->language);
    if (ccself->copyright) CcFree(ccself->copyright);
    if (ccself->managingEditor) CcFree(ccself->managingEditor);
    if (ccself->webMaster) CcFree(ccself->webMaster);
    if (ccself->pubDate) CcFree(ccself->pubDate);
    if (ccself->lastBuildDate) CcFree(ccself->lastBuildDate);
    if (ccself->category) CcFree(ccself->category);
    if (ccself->generator) CcFree(ccself->generator);
    if (ccself->docs) CcFree(ccself->docs);
    if (ccself->cloud) CcCloud_Destruct((CcObject_t *)ccself->cloud);
    if (ccself->ttl) CcFree(ccself->ttl);
    if (ccself->image) CcImage_Destruct((CcObject_t *)ccself->image);
    if (ccself->rating) CcFree(ccself->rating);
    if (ccself->textInput)
	CcTextInput_Destruct((CcObject_t *)ccself->textInput);
    if (ccself->skipHours) CcFree(ccself->skipHours);
    if (ccself->skipDays) CcFree(ccself->skipDays);
    CcArrayList_Destruct(&ccself->itemList);
    CcObject_Destruct(self);
}

static const CcObjectType_t ChannelType = {
    sizeof(CcChannel_t), "Channel", CcChannel_Destruct
};

CcChannel_t *
CcChannel(void)
{
    CcChannel_t * self = (CcChannel_t *)CcObject(&ChannelType);
    CcArrayList(&self->itemList);
    return self;
}

void
CcChannel_AddItem(CcChannel_t * self, CcItem_t * item)
{
    CcArrayList_New(&self->itemList, (CcObject_t *)item);
}

/*---------- CcRss ----------*/
CcRss_t *
CcRss(CcRss_t * self)
{
    CcArrayList(&self->channelList);
    return self;
}

void
CcRss_AddChannel(CcRss_t * self, CcChannel_t * channel)
{
    CcArrayList_New(&self->channelList, (CcObject_t *)channel);
}

void
CcRss_Destruct(CcRss_t * self)
{
    CcArrayList_Destruct(&self->channelList);
}
