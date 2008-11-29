/*---- license ----*/
/*---- enable ----*/
#ifndef  COCO_PARSER_H
#define  COCO_PARSER_H

#ifndef  COCO_CDEFS_H
#include  "CDefs.h"
#endif

/*---- hIncludes ----*/
#ifndef   COCO_DEFS_H
#include  "Defs.h"
#endif
/*---- enable ----*/

EXTC_BEGIN

struct CcsParser_s {
    CcsGlobals_t    * globals;
    CcsScanner_t    * scanner;
    CcsToken_t      * t;
    CcsToken_t      * la;
    int               maxT;
    /*---- members ----*/
    char            * tokenString;
    CcsBool_t         genScanner;
    CcsPosition_t   * hIncludes;
    CcsPosition_t   * cIncludes;
    CcsPosition_t   * members;
    CcsPosition_t   * constructor;
    CcsPosition_t   * destructor;
    /* Shortcut pointers */
    CcSymbolTable_t * symtab;
    CcLexical_t     * lexical;
    CcSyntax_t      * syntax;
    /*---- enable ----*/
};

CcsParser_t * CcsParser(CcsParser_t * self, CcsGlobals_t * globals);
void CcsParser_Destruct(CcsParser_t * self);
void CcsParser_Parse(CcsParser_t * self);

EXTC_END

#endif  /* COCO_PARSER_H */
