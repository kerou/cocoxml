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

CloudClass::CloudClass(){
    this->domain = (wchar_t*)L"";
    this->port = (wchar_t*)L"";
    this->path = (wchar_t*)L"";
    this->registerProcedure = (wchar_t*)L"";
    this->protocol = (wchar_t*)L"";
};

void CloudClass::Print(){
    wprintf(L"Cloud('%ls','%ls','%ls','%ls'')",
            this->domain,
            this->port,
            this->path,
            this->registerProcedure,
            this->protocol);
};

ImageClass::ImageClass(){
    this->url = (wchar_t*)L"";
    this->title = (wchar_t*)L"";
    this->link = (wchar_t*)L"";
    this->width = (wchar_t*)L"";
    this->height = (wchar_t*)L"";
};

void ImageClass::Print(){
    wprintf(L"Image('%ls','%ls','%ls','%ls')",
            this->url,
            this->title,
            this->link,
            this->width,
            this->height);
};

TextInputClass::TextInputClass(){
    this->title = (wchar_t*)L"";
    this->description = (wchar_t*)L"";
    this->name = (wchar_t*)L"";
    this->link = (wchar_t*)L"";
};

void TextInputClass::Print(){
    wprintf(L"TextInput('%ls','%ls','%ls','%ls')",
            this->title,
            this->description,
            this->name,
            this->link);
};

ItemClass::ItemClass(){
    this->title = (wchar_t*)L"";
    this->link = (wchar_t*)L"";
    this->description = (wchar_t*)L"";
    this->author = (wchar_t*)L"";
    this->category = (wchar_t*)L"";
    this->comments = (wchar_t*)L"";
    this->enclosure = (wchar_t*)L"";
    this->guid = (wchar_t*)L"";
    this->pubdate = (wchar_t*)L"";
    this->source = (wchar_t*)L"";
    this->image = NULL;
};

void ItemClass::Print(){
    wprintf(L"Item('%ls','%ls','%ls','%ls','%ls','%ls','%ls','%ls','%ls','%ls','",
            this->title,
            this->link,
            this->description,
            this->author,
            this->category,
            this->comments,
            this->enclosure,
            this->guid,
            this->pubdate,
            this->source);
    if(this->image)
        this->image->Print();
    wprintf(L"')");
};

void ChannelClass::AddItem(ItemClass *item){
    itemList.push_back(item);
};

ChannelClass::ChannelClass(){
    this->title = (wchar_t*)L"";
    this->link = (wchar_t*)L"";
    this->description = (wchar_t*)L"";
    this->language = (wchar_t*)L"";
    this->copyright = (wchar_t*)L"";
    this->managingEditor = (wchar_t*)L"";
    this->webMaster = (wchar_t*)L"";
    this->pubDate = (wchar_t*)L"";
    this->lastBuildDate = (wchar_t*)L"";
    this->category = (wchar_t*)L"";
    this->generator = (wchar_t*)L"";
    this->docs = (wchar_t*)L"";
    this->ttl = (wchar_t*)L"";
    this->rating = (wchar_t*)L"";
    this->skipHours = (wchar_t*)L"";
    this->skipDays = (wchar_t*)L"";

    this->cloud = NULL;
    this->image = NULL;
    this->textInput = NULL;
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
    if(this->cloud) 
        this->cloud->Print();
    wprintf(L"'\n");

    wprintf(L"\tttl: '%ls'\n", this->ttl);

    wprintf(L"\timage: '");
    if(this->image)
        this->image->Print();
    wprintf(L"'\n");

    wprintf(L"\trating: '%ls'\n", this->rating);

    wprintf(L"\ttextInput: '");
    if(this->textInput)
        this->textInput->Print();
    wprintf(L"'\n");

    wprintf(L"\tskipHours: '%ls'\n", this->skipHours);
    wprintf(L"\tskipDays: '%ls'\n", this->skipDays);

    vector<ItemClass *>::iterator it;
    for(it=itemList.begin(); it!=itemList.end(); it++){
        /* print item */
        (*it)->Print();
        wprintf(L"\n");
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
