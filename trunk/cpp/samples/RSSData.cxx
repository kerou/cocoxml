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

ostream &operator<<(ostream &s, CloudClass p){
    s << "Cloud('" << 
        p.domain << "', '" << 
        p.port << "', '" << 
        p.path << "', '" << 
        p.registerProcedure << "', '" << 
        p.protocol << "')";
    return s;
};

ostream &operator<<(ostream &s, ImageClass p){
    s << "Image('" << p.url << "', '" << p.title << "', '" << p.link << "', '" << p.width << "', '" << p.height << "')";
    return s;
};

ostream &operator<<(ostream &s, TextInputClass p){
    s <<  "TextInput('" << 
        p.title << "', '" << 
        p.description << "', '" << 
        p.name << "', '" << 
        p.link << "')";
    return s;
};

ostream &operator<<(ostream &s, ItemClass p)
{
    s << "Item('" << 
        p.title << "', '" << 
        p.link << "', '" << 
        p.description << "', '" << 
        p.author << "', '" << 
        p.category << "', '" << 
        p.comments << "', '" << 
        p.enclosure << "', '" << 
        p.guid << "', '" << 
        p.pubdate << "', '" << 
        p.source << "', '" << 
        p.image << "')";
    return s;
};

ChannelClass::ChannelClass() {
    itemList = new list<ItemClass*>();
};

void ChannelClass::AddItem(ItemClass *item) {
        itemList->push_back(item);
};

ostream &operator<<(ostream &s, ChannelClass p)
{
    s << "Channel: '" << p.title << "'\n" <<
        "\tlink: '" << p.link << "'\n" <<
        "\tdescription: '" << p.description << "'\n" <<
        "\tlanguage: '" << p.language << "'\n" <<
        "\tcopyright: '" << p.copyright << "'\n" <<
        "\tmanagingEditor: '" << p.managingEditor << "'\n" <<
        "\twebMaster: '" << p.webMaster << "'\n" <<
        "\tpubDate: '" << p.pubDate << "'\n" <<
        "\tlastBuildDate: '" << p.lastBuildDate << "'\n" <<
        "\tcategory: '" << p.category << "'\n" <<
        "\tgenerator: '" << p.generator << "'\n" <<
        "\tdocs: '" << p.docs << "'\n" <<
        "\tcloud: '" << p.cloud << "'\n" <<
        "\tttl: '" << p.ttl << "'\n" <<
        "\timage: '" << p.image << "'\n" <<
        "\trating: '" << p.rating << "'\n" <<
        "\ttextInput: '" << p.textInput << "'\n" <<
        "\tskipHours: '" << p.skipHours << "'\n" <<
        "\tskipDays: '" << p.skipDays << "'\n";
#if 0
    foreach (Item i in itemList) {
        s << s << "\t" << i << "\n";
    }
#endif
    return s;
};

RssClass::RssClass() {
    channelList = new list<ChannelClass*>();
};

void RssClass::AddChannel(ChannelClass* channel) {
        channelList->push_back(channel);
};

ostream &operator<<(ostream &s, RssClass p)
{
#if 0
    string s = "";
    foreach (Channel c in channelList) {
        s = s << c << "\n";
    }
#endif
    s<<"???";
    return s;
}
