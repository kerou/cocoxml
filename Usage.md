Because Coco is a source code generater, you should choose the programming
language and the atg grammer you want. In CocoXml, it is treated as scheme.

Now CocoXml implement three scheme now:
  * dump: Dump symbol table, EBNF nodes, lexical states.
> > Used to debug the generated scanner/parser.
  * c: Original C Coco with some enhancements. Can be used to generate scanner,
> > parser in C. These scanner & parser can be used to parse common text
> > sources, such as C#, Java and so on.
  * cxml: Work like CocoXml-0.9.0. Can be used to generate scanner, parser in C.
> > These scanner & parser are based on expat and can be used to parse the
> > structure of various XML elements, such as tags, attributes. For example,
> > the scanner/parser can be generated for XSLT, RSS and so on.

To integrate multiple generated scanner/parser into one binary, prefix
replacement has to be supported.

The original syntax/semantic of atg file is defined by Hanspeter Moessenboeck,
and the detail documentation about it is located at:
http://www.ssw.uni-linz.ac.at/coco/Doc/UserManual.pdf

In the using of 'Coco', I found some enhancements are required to support the
real usage, these enhancements are introduced at: [Enhancements](Enhancements.md).

In every atg/xatg, SCHEME is used to specify the required scheme and the
scanner/parser prefix.

CocoXml provide one binary 'Coco' only. It can be used to generate sources
from atg/xatg and source templates. Template sources are provided with each
scheme. For 'c' scheme: Scanner.h, Scanner.c, Parser.h, Parser.c are provided.
For 'cxml' scheme: Scanner4Xml.h, Scanner4Xml.c, Parser4Xml, Parser4Xml.c are
provided.

To reduce the size of generated sources, public functions are placed into
library: 'libcoco.a'. The corresponding headers are installed too.

For the schemes provided for other programming languages, no library can be
used, so all code are provided in sources.

The usage of Coco command line is:
```
$ Coco {{Options}} Grammar.atg/Grammar.xatg
```

The possible options are:
  * -s SCHEME
Commonly the scheme is specified by atg/xatg, but sometimes the user want to
switch to another output scheme temporarily, such as use 'dump' scheme to
check the symbol table and so on. So the user can use '-s dump' for this
purpose.
  * -o OUTPUT-METHOD
> > Sometimes user might want to modify the generated sources (not suggested)
> > for some special purpose. So 'Coco' won't to override the already exists
> > sources automatically. There output method are provided: 'auto',
> > 'generate', 'update'. For 'generate', 'Coco' always generate the sources by
> > searching the templates and atg/xatg, any changes in the current sources
> > are over written. For 'update', 'Coco' only refresh the generated part but
> > not touch any template sources. By default, 'auto' is used, which will
> > check whether the target sources are present, if present, update them,
> > otherwise generate them from installed templates.
  * -g
> > Shortcut of '-o generate' for the users who never change the generated
> > sources mannually.
  * -d OUTPUT-DIR
> > Specify the directory which the output sources located. By default, it is
> > the atg/xatg located directory.
  * -t TEMPLATE-DIR
> > Tell 'Coco' where to find the template sources.

So the commonly usage of 'Coco' is to write an atg/xatg, and run:
```
$ Coco my.atg
```
Then Scanner.h, Scanner.c, Parser.h, Parser.c are generated. User can use them
directly. After any change of 'my.atg', rerun:
```
$ Coco my.atg
```

If 'Coco' is upgraded, just run:
```
$ Coco -g my.atg
```
So the latest template changes will be merged.