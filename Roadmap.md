**0.9.0:
  * Implement XML structure scanner/parser generation for C#, Java and C++ in the original Coco way.**

**0.9.1:
  * Re-implement all Coco algorithm in C.
  * Split output generation from algorithm completely.
  * Integrate original Coco with CocoXml into one binary: 'Coco'.
  * The combined binary distinguish input grammers by extension.
  * 'CocoInit' is added to generate sources for user.
  * Work in sources update way.
  * Output scheme 'dump', 'c', 'cxml' are supported now.
  * ebuild is added for the gentoo user.**

**0.9.2:
  * Combine the features provided by 'CocoInit' into 'Coco'. So 'CocoInit' is removed.
  * Implement some applications:
    * CExpr: A demo for c-style expression, can be used like an integer calculator.
    * Json: A Json parser.
    * Kconfig: Can be used to parse kernel Kconfig files and .config files. Can be used as a software configuration method.
    * patch: A unified diff parser, store all patch informations into data structure.
    * pgn: A parser of chess pgn files. The output is the board of all steps.
    * rss: A XML parser of RSS.**

