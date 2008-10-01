/*-------------------------------------------------------------------------
Copyright (c) 2008, Charles Wang <charlesw123456@gmail.com>

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
using System.Collections.Generic;

public class Cloud {
    public string domain;
    public string port;
    public string path;
    public string register_procedure;
    public string protocol;
}

public class Image {
    public string url;
    public string title;
    public string link;
    public string width;
    public string height;
}

public class TextInput {
    public string title;
    public string description;
    public string name;
    public string link;
}

public class Item {
    public string title;
    public string link;
    public string description;
    public string author;
    public string category;
    public string comments;
    public string enclosure;
    public string guid;
    public string pubdate;
    public string source;
    public Image  image;
}

public class Channel {
    public string    title;
    public string    link;
    public string    description;
    public string    language;
    public string    copyright;
    public string    managingEditor;
    public string    webMaster;
    public string    pubDate;
    public string    lastBuildDate;
    public string    category;
    public string    generator;
    public string    docs;
    public Cloud     cloud;
    public string    ttl;
    public Image     image;
    public string    rating;
    public TextInput textInput;
    public string    skipHours;
    public string    skipDays;

    List<Item>  itemList;

    public Channel() {
	itemList = new List<Item>();
    }

    public void AddItem(Item item) {
	itemList.Add(item);
    }
}

public class Rss {
    List<Channel>  channelList;
    
    public Rss() {
	channelList = new List<Channel>();
    }

    public void AddChannel(Channel channel) {
	channelList.Add(channel);
    }
}
