/*-------------------------------------------------------------------------
  Copyright (c) 2008, Ken Zhao <kentoo.zhao@gmail.com>

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the 
  Free Software Foundation; either version 2, or (at your option) any 
  later version.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.

  You should have received a copy of the GNU General Public License along 
  with this program; if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  -------------------------------------------------------------------------*/
#include <ostream>
#include <iostream>
#include "RSSData.hxx"

void CloudClass::Print(){
    wprintf(L"Cloud('%ls,%ls,%ls,%ls')",
            this->domain,
            this->port,
            this->path,
            this->registerProcedure,
            this->protocol);
};

void ImageClass::Print(){
    wprintf(L"Image('%ls,%ls,%ls,%ls')",
            this->url,
            this->title,
            this->link,
            this->width,
            this->height);
};

void TextInputClass::Print(){
    wprintf(L"TextInput('%ls,%ls,%ls,%ls')",
            this->title,
            this->description,
            this->name,
            this->link);
};

void ItemClass::Print(){
    wprintf(L"Item('%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls')",
            this->title,
            this->link,
            this->description,
            this->author,
            this->category,
            this->comments,
            this->enclosure,
            this->guid,
            this->pubdate,
            this->source,
            this->image);
};

void ChannelClass::AddItem(ItemClass *item){
        itemList.push_back(item);
};

void ChannelClass::Print(){
    wprintf(L"Channel: '%ls'\n", this->title);
    wprintf(L"\tlink: '%ls'\n", this->link);
    wprintf(L"\tdescription: '%ls'\n", this->description);
    wprintf(L"\tlanguage: '%ls'\n", this->language);
    wprintf(L"\tcopyright: '%ls'\n", this->copyright);
    wprintf(L"\tmanagingEditor: '%ls'\n", this->managingEditor);
    wprintf(L"\twebMaster: '%ls'\n", this->webMaster);
    wprintf(L"\tpubDate: '%ls'\n", this->pubDate);
    wprintf(L"\tlastBuildDate: '%ls'\n", this->lastBuildDate);
    wprintf(L"\tcategory: '%ls'\n", this->category);
    wprintf(L"\tgenerator: '%ls'\n", this->generator);
    wprintf(L"\tdocs: '%ls'\n", this->docs);

    wprintf(L"\tcloud: '");
    this->cloud->Print();
    wprintf(L"\n");

    wprintf(L"\tttl: '%ls'\n", this->ttl);

    wprintf(L"\timage: '");
    this->image->Print();
    wprintf(L"\n");

    wprintf(L"\trating: '%ls'\n", this->rating);

    wprintf(L"\ttextInput: '");
    this->textInput->Print();
    wprintf(L"\n");

    wprintf(L"\tskipHours: '%ls'\n", this->skipHours);
    wprintf(L"\tskipDays: '%ls'\n", this->skipDays);

    vector<ItemClass *>::iterator it;
    for(it=itemList.begin(); it!=itemList.end(); it++){
        /* print item */
        (*it)->Print();
    }
};

void RssClass::AddChannel(ChannelClass* channel) {
        channelList.push_back(channel);
};

void RssClass::Print(){
    vector<ChannelClass*>::iterator it;
    for(it=channelList.begin(); it!=channelList.end(); it++){
        /* print channel */
        (*it)->Print();
    }
}
