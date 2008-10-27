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
#ifndef __RSS_DATA_H
#define __RSS_DATA_H

#include <list>

using namespace std;

class CloudClass {
public:
    wchar_t* domain;
    wchar_t* port;
    wchar_t* path;
    wchar_t* registerProcedure;
    wchar_t* protocol;

    friend ostream &operator<<(ostream &s, CloudClass p);
};

class ImageClass {
public:
    wchar_t* url;
    wchar_t* title;
    wchar_t* link;
    wchar_t* width;
    wchar_t* height;

    friend ostream &operator<<(ostream &s, ImageClass p);
};

class TextInputClass {
public:
    wchar_t* title;
    wchar_t* description;
    wchar_t* name;
    wchar_t* link;

    friend ostream &operator<<(ostream &s, TextInputClass p);
};

class ItemClass {
public:
    wchar_t* title;
    wchar_t* link;
    wchar_t* description;
    wchar_t* author;
    wchar_t* category;
    wchar_t* comments;
    wchar_t* enclosure;
    wchar_t* guid;
    wchar_t* pubdate;
    wchar_t* source;
    ImageClass*  image;

    friend ostream &operator<<(ostream &s, ItemClass p);
};

class ChannelClass {
public:
    wchar_t*    title;
    wchar_t*    link;
    wchar_t*    description;
    wchar_t*    language;
    wchar_t*    copyright;
    wchar_t*    managingEditor;
    wchar_t*    webMaster;
    wchar_t*    pubDate;
    wchar_t*    lastBuildDate;
    wchar_t*    category;
    wchar_t*    generator;
    wchar_t*    docs;
    CloudClass* cloud;
    wchar_t*    ttl;
    ImageClass* image;
    wchar_t*    rating;
    TextInputClass*  textInput;
    wchar_t*    skipHours;
    wchar_t*    skipDays;
    list<ItemClass*>  *itemList;

    ChannelClass();
    void AddItem(ItemClass *item);

    friend ostream &operator<<(ostream &s, ChannelClass p);
};
    
class RssClass {
public:
    list<ChannelClass*>  *channelList;
    
    RssClass();
    void AddChannel(ChannelClass *channel);

    friend ostream &operator<<(ostream &s, RssClass p);
};

#endif
