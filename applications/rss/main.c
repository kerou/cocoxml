/*-------------------------------------------------------------------------
  Copyright (C) 2008, Charles Wang <charlesw123456@gmail.com>
  Author: Charles Wang <charlesw123456@gmail.com>
  License: BSD (see LICENSE-BSD)
-------------------------------------------------------------------------*/
#include  "Parser4Xml.h"

static void
ShowItem(const CcItem_t * self)
{
    printf("    Item:\n");
    printf("        Title: [%s]\n", self->title);
    printf("        Link: [%s]\n", self->link);
}

static void
ShowChannel(const CcChannel_t * self)
{
    const CcItem_t * item; CcArrayListIter_t iter;
    printf("Channel:\n");
    printf("    Title: [%s]\n", self->title);
    printf("    Link: [%s]\n", self->link);
    for (item = (const CcItem_t *)CcArrayList_FirstC(&self->itemList, &iter);
	 item;
	 item = (const CcItem_t *)CcArrayList_NextC(&self->itemList, &iter))
	ShowItem(item);
    printf("    %d Item(s)\n", self->itemList.Count);
}

static void
ShowRss(const CcRss_t * self)
{
    const CcChannel_t * channel; CcArrayListIter_t iter;
    for (channel = (const CcChannel_t *)
	     CcArrayList_FirstC(&self->channelList, &iter);
	 channel; channel = (const CcChannel_t *)
	     CcArrayList_NextC(&self->channelList, &iter))
	ShowChannel(channel);
    printf("%d Channel(s)\n", self->channelList.Count);
}

int
main(int argc, char * argv[])
{
    RssParser_t parser;

    if (argc != 2) {
	fprintf(stderr, "%s rss\n", argv[0]);
	return -1;
    }
    if (!strcmp(argv[1], "-")) {
	if (!RssParser(&parser, stdin, stderr)) goto errquit0;
    } else {
	if (!RssParser_ByName(&parser, argv[1], stderr)) goto errquit0;
    }
    RssParser_Parse(&parser);
    if (!RssParser_Finish(&parser)) goto errquit1;
    ShowRss(&parser.rss);
    RssParser_Destruct(&parser);
    return 0;
 errquit1:
    RssParser_Destruct(&parser);
 errquit0:
    return -1;
}
