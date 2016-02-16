CocoXml is inspired by Coco/R and modified from it.

Coco/R is released under GPLv2 and written by Hanspeter Moessenboeck,
University of Linz. Its homepage is http://www.ssw.uni-linz.ac.at/coco/ .

CocoXml is released under GPLv2 too.

Coco/R can read an atg file and tempalte files, then generate scanner and parser
for user. Different versions are implemented for different languages, such as
C#, Java and C++ and so on.

Firstly, I modify Coco/R so it can accept a different atg. This new atg can be
used to parse the tag/attribute structure base on the result of SAX parser,
just like XCC does. For the detail of XCC, please access:
[ftp://plasma-gate.weizmann.ac.il/pub/xcc/](ftp://plasma-gate.weizmann.ac.il/pub/xcc/). CocoXml use the EBNF to describe the
syntax of tags, attributes and other XML elements. It is a strict way and can
parse complex XML structure easily.

Then CocoXml-0.9.0.tar.gz is released.

After that, it is realized that if CocoXml is provided in the Coco/R way, the
core algorithm has to be implemented in all supported languages again and again.
This is not a necessitation. And I also need a stronger implement of Coco/R in
C, so it can be used to generate various parsers in C.

So I decide to re-implement Coco/R in pure-C and split the output schemes from
algorithm completely. And when a new programming language support is required,
define a new output scheme is enough.

Another advantage of C implemented Coco/R is there are least dependencies, so
the user can install it easily. And linux distribution can distribute it easily
too.

Now the new implemented Coco(in c) and CocoXml are combined into one binary.
If the extension of the input is .atg, it is treated as a syntax of the
original Coco. If the extension of the input is .xatg, it is treated as a
syntax of CocoXml.

Some more improvements are introduced too. For the details, please refer to
the wiki of CocoXml, which can be accessed at http://code.google.com/p/cocoxml/w/list .

The source of CocoXml can be checkout with the following command:
```
svn checkout http://cocoxml.googlecode.com/svn/trunk cocoxml-read-only.
```
The referenced Coco/R code are committed into svn too, so my modifications can
be show easily. :)