# Enhancements List #

Here is a simple list of CocoXml enhancements.

  * Self template : Treat the source as template.
  * Self-defined sections :  The named sections is replaced by the corresponding sections in atg/xatg.
  * Specify scheme : The scheme used by atg/xatg is binding to some programming language, so it should be written in atg/xatg
  * Specify prefix : The prefix of scanner and parser has to be added to let multiple scanner/parser to be linked into one binary.
  * Options : Add options into atg/xatg.
  * Indentation : Generate IndentIn, IndentOut, IndentErr for the syntax which is depend on indentation.
  * Whitespace : Do not ignore white space.
  * Backslash new line : Place backslash at the end of line to continue the current line.
  * Source including : Include text from another file.
  * Semantic terminals : Insert user defined terminals according to semantic.

# Self template #

In order to let the user modify their template easily, 'Coco' provide an
'update' output method. In this method, the sources is updated with the
generated code and data. All sources out of the generated area is not touched.

So the user can modify them directly.

To let the sources work, edit easily, all generated area is marked by:
```
/*---- section name ----*/
Here is the generated area.
/*---- enable ----*/
```

The original 'Coco' template format use '--->XXX', this format will confuse the
editors which can do auto-indentation, such as emacs. And one template will
generate many sources, this is not a convenient way to maintain it.

In CocoXml, source is template, and template is source. When it is processed by
compiler, it is source. When it is proccessed by 'Coco', it is template.

User determine the code outside the generated area, and 'Coco' generate the
code in the generated area. And compiler, editor treat the source as a whole
thing. When user update their source, they are update their template in the
same time.

# Self-defined sections #

'Coco' is used to generate the codes in generate area of templates(sources).
So it define some pre-defined sections with different names. User can define
their-own sections with different names too. The sources placed in the atg/xatg
will be copied to templates(sources) without any changes.

For examples, 'license' is a commonly used named-section which will copy license
information from atg/xatg to templates(sources).

In this way, there is no need to modify the Coco itself to support some new
defined sections if they just copy some code from atg/xatg to tempaltes.

# Specify scheme #

For an atg/xatg, the semantic actions are embedded into it. These semantic
actions are written in their own programming languages, so the required scheme
is determined. For this reason, the default scheme used by atg/xatg should not
be changed to another scheme. For example, a 'c' semantic actions atg shuold
never be applied by 'csharp' scheme.

But sometimes the other schemes, such as 'dump', should be applied too. This
is a rare usage, so it can be specified at command line too.

To specify scheme name, use 'SCHEME' directive.

# Specify prefix #

In c, there is no namespace and any other method. So if we want to link
multiple scanner/parser into one binary, we have to prefix them with different
prefix.

To specify the generated scanner/parser prefix, use 'SCHEME' directive. The
prefix should be placed after the scheme name.

# Options #

There are some enhancements of CocoXml, so it is not convenient for CocoXml to
define directives for each enhancements. The 'OPTIONS' directive is defined
for all boolean enhancements, such as indentation, space, backslash newline,
ignore case.

# Indentation #

Some syntax is determine their structures by indentation, such as python,
Kconfig. Commonly, Coco ignore whitespace and tab, and counting the column of
first non-blank characters in a line is difficult in EBNF. So the indentation
support is added into Scanner. Scanner will detect the change of indentation
and generate 'IndentIn' when indentation is increased, generate 'IndentOut'
when indentation is decreased. If the indentation is decreased to an unused
indentation, 'IndentErr' is generated.

Then the user of 'Coco' and use 'IndentIn', 'IndentOut', 'IndentErr' in their
syntax to mark the syntax structure. The kind value of these three terminals
are defined as macros in the generated 'Scanner.h'.

There are also some other circumstance that we want to close the indentation
generation after the specified column, this is implemented by:
CcsScanner\_IndentLimit.

To enable indentation support, add 'indentation' to OPTIONS.

For the examples, please see [trunk/applications/Kconfig/desc/Kconfig.atg].

# Whitespace #

Whitespace is ignored in original 'Coco', but it is meaningful in some syntax,
such as 'patch'. So the user should have a chance the generate token for
whitespace.

To enable whitespace support, add 'space' to OPTIONS.

# Backslash new line #

For some syntaxes, such as c preprocesser, Kconfig, backslash is placed at the
end of line to indicate that the next line is the continue line of the current
line. This affect the behaviour of indentation support. In the continue line,
not any indentation terminals should be generated.

To enable backslash new line support, add 'backslash newline' to OPTIONS.

# Source including #

For some syntaxes, such as c preprocesser, Kconfig, 'include'/'source' are
supported to include some other sources. So CocoXml add the support about
source including by add the following functions:

  * CcsScanner\_Include : Include sources from the opened file.
  * CcsScanner\_IncludeByName : Include sources specified by the provided filename. The include path object is provided to specify the search directories.

Because the 'la' of parser should be the first token of the included sources,
so the original 'la' has to be withdraw of the current sources. The last
parameter of CcsScanner\_Include, CcsScanner\_IncludeByName is the pointer to
'la'.

To help the searching of the included file, CcsIncPathList\_t is defined. For
the usage of CcsIncPathList\_t, refer to [trunk/schemes/c/IncPathList.h].

[trunk/applications/Kconfig/desc/Kconfig.atg] is an example of source including.

# Semantic terminals #

Sometimes, the syntax is determined by semantic too. For example, in unified
patch, the number of patch lines is determined by its title. This is impossible
to expressed in EBNF.

So user defined terminals can be generated by semantic actions and used in
EBNF syntax expression. For example, patch semantic action generate 'InPiece'
terminals for the lines which to be treated as a diff lines.

To define a user defined terminals, use 'TERMINALS' directive.

For example, refer to [trunk/applications/patch].