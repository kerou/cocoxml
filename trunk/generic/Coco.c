/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

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

  As an exception, it is allowed to write an extension of Coco/R that is
  used as a plugin in non-free software.

  If not otherwise stated, any source code generated by Coco/R (other than 
  Coco/R itself) does not fall under the GNU General Public License.
-------------------------------------------------------------------------*/
#include  "Globals.h"
#include  "Arguments.h"
#include  "OutputScheme.h"
#include  "c/COutputScheme.h"
#include  "dump/DumpOutputScheme.h"

static const char * usage_format =
    "Usage: %s Grammar.atg {{Option}}\n"
    "Options:\n"
    "  -namespace <namespaceName>\n"
    "  -frames    <frameFilesDirectory>\n"
    "  -trace     <traceString>\n"
    "  -o         <outputDirectory>\n"
    "Valid characters in the trace string:\n"
    "  A  trace automaton\n"
    "  F  list first/follow sets\n"
    "  G  print syntax graph\n"
    "  I  trace computation of first sets\n"
    "  J  list ANY and SYNC sets\n"
    "  P  print statistics\n"
    "  S  list symbol table\n"
    "  X  list cross reference table\n"
    "Scanner[.lang].frame and Parser[.lang].frame files needed in ATG directory\n"
    "or in a directory specified in the -frames option.\n";

static const char * hIncludes =
    "#ifndef   COCO_DEFS_H\n"
    "#include  \"Defs.h\"\n"
    "#endif\n";

static const char * cIncludes =
    "#include  \"Globals.h\"\n"
    "#include  \"lexical/CharSet.h\"\n"
    "#include  \"lexical/CharClass.h\"\n"
    "#include  \"lexical/Nodes.h\"\n"
    "#include  \"syntax/Nodes.h\"\n"
    "static const int CcsParser_id = 0;\n"
    "static const int CcsParser_str = 1;\n"
    "static const char * noString = \"~none~\";\n";

int
main(int argc, char * argv[])
{
    CcArguments_t arguments;
    CcArgumentsIter_t iter;
    CcGlobals_t   globals;
    const char * atgName, * schemeName;
    CcOutputScheme_t * scheme;

    printf("Coco/R (Oct22, 2008)\n");
    CcArguments(&arguments, argc, argv);
    atgName = CcArguments_First(&arguments, "", &iter);
    if (atgName == NULL) {
	printf(usage_format, argv[0]);
	return 0;
    }
    if (!CcGlobals(&globals, atgName, stderr)) goto errquit0;
    if (!CcGlobals_Parse(&globals)) goto errquit1;

    globals.base.parser.hIncludes = CcsPosition(0, strlen(hIncludes), 0, hIncludes);
    globals.base.parser.cIncludes = CcsPosition(0, strlen(cIncludes), 0, cIncludes);

    schemeName = CcArguments_First(&arguments, "scheme", &iter);
    if (schemeName == NULL || !strcmp(schemeName, "c")) {
	if (!(scheme = (CcOutputScheme_t *)
	      CcCOutputScheme(&globals, &arguments)))
	    goto errquit1;
    } else if (!strcmp(schemeName, "dump")) {
	if (!(scheme = (CcOutputScheme_t *)
	      CcDumpOutputScheme(&globals, &arguments)))
	    goto errquit1;
    } else {
	scheme = NULL;
    }
    if (scheme) {
	if (!CcOutputScheme_GenerateOutputs(scheme)) goto errquit2;
	CcObject_VDestruct((CcObject_t *)scheme);
    }
    CcGlobals_Destruct(&globals);
    CcArguments_Destruct(&arguments);
    return 0;
 errquit2:
    CcObject_VDestruct((CcObject_t *)scheme);
 errquit1:
    CcGlobals_Destruct(&globals);
 errquit0:
    CcArguments_Destruct(&arguments);
    return -1;
}
