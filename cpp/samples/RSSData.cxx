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

ostream &operator<<(ostream &s, Cloud p){
    s << "Cloud('" << 
        domain << "', '" << 
        port << "', '" << 
        path << "', '" << 
        registerProcedure << "', '" << 
        protocol << "')";
    return s;
};

ostream &operator<<(ostream &s, Image p){
    s << "Image('" << url << "', '" << title << "', '" << link << "', '" << width << "', '" << height << "')";
    return s;
};

ostream &operator<<(ostream &s, TextInput p){
    s <<  "TextInput('" << 
        title << "', '" << 
        description << "', '" << 
        name << "', '" << 
        link << "')";
    return s;
};

ostream &operator<<(ostream &s, Item p)
{
    s << "Item('" << 
        title << "', '" << 
        link << "', '" << 
        description << "', '" << 
        author << "', '" << 
        category << "', '" << 
        comments << "', '" << 
        enclosure << "', '" << 
        guid << "', '" << 
        pubdate << "', '" << 
        source << "', '" << 
        image << "')";
    return s;
};

Channel::Channel() {
    itemList = new List<Item>();
};

void Channel::AddItem(Item item) {
        itemList.Add(item);
};

ostream &operator<<(ostream &s, Channel p)
{
    s << "Channel: '" << title << "'\n" <<
        "\tlink: '" << link << "'\n" <<
        "\tdescription: '" << description << "'\n" <<
        "\tlanguage: '" << language << "'\n" <<
        "\tcopyright: '" << copyright << "'\n" <<
        "\tmanagingEditor: '" << managingEditor << "'\n" <<
        "\twebMaster: '" << webMaster << "'\n" <<
        "\tpubDate: '" << pubDate << "'\n" <<
        "\tlastBuildDate: '" << lastBuildDate << "'\n" <<
        "\tcategory: '" << category << "'\n" <<
        "\tgenerator: '" << generator << "'\n" <<
        "\tdocs: '" << docs << "'\n" <<
        "\tcloud: '" << cloud << "'\n" <<
        "\tttl: '" << ttl << "'\n" <<
        "\timage: '" << image << "'\n" <<
        "\trating: '" << rating << "'\n" <<
        "\ttextInput: '" << textInput << "'\n" <<
        "\tskipHours: '" << skipHours << "'\n" <<
        "\tskipDays: '" << skipDays << "'\n";
    foreach (Item i in itemList) {
        s << s << "\t" << i << "\n";
    }
    return s;
};

Rss::Rss() {
    channelList = new List<Channel>();
}

void Rss::AddChannel(Channel channel) {
        channelList.Add(channel);
}

ostream &operator<<(ostream &s, Rss p)
{
    string s = "";
    foreach (Channel c in channelList) {
        s = s << c << "\n";
    }
    return s;
}
