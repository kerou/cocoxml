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
class Cloud {
public:
    string domain;
    string port;
    string path;
    string registerProcedure;
    string protocol;

    friend ostream &operator<<(ostream &s, Cloud p);
};

class Image {
public:
    string url;
    string title;
    string link;
    string width;
    string height;

    friend ostream &operator<<(ostream &s, Image p);
};

class TextInput {
public:
    string title;
    string description;
    string name;
    string link;

    friend ostream &operator<<(ostream &s, TextInput p);
};

class Item {
public:
    string title;
    string link;
    string description;
    string author;
    string category;
    string comments;
    string enclosure;
    string guid;
    string pubdate;
    string source;
    Image  image;

    friend ostream &operator<<(ostream &s, Item p);
};

class Channel {
public:
    string    title;
    string    link;
    string    description;
    string    language;
    string    copyright;
    string    managingEditor;
    string    webMaster;
    string    pubDate;
    string    lastBuildDate;
    string    category;
    string    generator;
    string    docs;
    Cloud     cloud;
    string    ttl;
    Image     image;
    string    rating;
    TextInput textInput;
    string    skipHours;
    string    skipDays;
    List<Item>  itemList;

    Channel();
    void AddItem(Item item);

    friend ostream &operator<<(ostream &s, Channel p);
};
    
class Rss {
public:
    List<Channel>  channelList;
    
    Rss();
    void AddChannel(Channel channel);

    friend ostream &operator<<(ostream &s, Rss p);
};
