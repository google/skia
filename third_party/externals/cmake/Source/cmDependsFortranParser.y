%{
/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/*-------------------------------------------------------------------------
  Portions of this source have been derived from makedepf90 version 2.8.8,

   Copyright (C) 2000--2006 Erik Edelmann <erik.edelmann@iki.fi>

  The code was originally distributed under the GPL but permission
  from the copyright holder has been obtained to distribute this
  derived work under the CMake license.
-------------------------------------------------------------------------*/

/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --yacc --name-prefix=cmDependsFortran_yy
        --defines=cmDependsFortranParserTokens.h
         -ocmDependsFortranParser.cxx
          cmDependsFortranParser.y

Modify cmDependsFortranParser.cxx:
  - remove TABs
  - remove use of the 'register' storage class specifier
  - Remove the yyerrorlab block in range ["goto yyerrlab1", "yyerrlab1:"]
*/

/*-------------------------------------------------------------------------*/
#define cmDependsFortranParser_cxx
#include "cmDependsFortranParser.h" /* Interface to parser object.  */
#include "cmDependsFortranParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

#include <cmsys/String.h>

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmDependsFortran_yyerror(x) \
        cmDependsFortranError(yyscanner, x)

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Helper function to forward error callback.  */
static void cmDependsFortranError(yyscan_t yyscanner, const char* message)
{
  cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
  cmDependsFortranParser_Error(parser, message);
}

static bool cmDependsFortranParserIsKeyword(const char* word,
                                            const char* keyword)
{
  return cmsysString_strcasecmp(word, keyword) == 0;
}

/* Disable some warnings in the generated code.  */
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch contains default but no case. */
# pragma warning (disable: 4701) /* Local variable may not be initialized.  */
# pragma warning (disable: 4702) /* Unreachable code.  */
# pragma warning (disable: 4127) /* Conditional expression is constant.  */
# pragma warning (disable: 4244) /* Conversion to smaller type, data loss. */
#endif
%}

/* Generate a reentrant parser object.  */
%pure-parser

%union {
  char* string;
}

/*-------------------------------------------------------------------------*/
/* Tokens */
%token EOSTMT ASSIGNMENT_OP GARBAGE
%token CPP_INCLUDE F90PPR_INCLUDE COCO_INCLUDE
%token F90PPR_DEFINE CPP_DEFINE F90PPR_UNDEF CPP_UNDEF
%token CPP_IFDEF CPP_IFNDEF CPP_IF CPP_ELSE CPP_ELIF CPP_ENDIF
%token F90PPR_IFDEF F90PPR_IFNDEF F90PPR_IF
%token F90PPR_ELSE F90PPR_ELIF F90PPR_ENDIF
%token COMMA DCOLON
%token <string> CPP_TOENDL
%token <number> UNTERMINATED_STRING
%token <string> STRING WORD
%token <string> CPP_INCLUDE_ANGLE

/*-------------------------------------------------------------------------*/
/* grammar */
%%

code: /* empty */ | code stmt;

stmt: keyword_stmt | assignment_stmt;

assignment_stmt: WORD ASSIGNMENT_OP other EOSTMT    /* Ignore */
    {
    free($1);
    }

keyword_stmt:
  WORD EOSTMT
    {
    if (cmDependsFortranParserIsKeyword($1, "interface"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, true);
      }
    free($1);
    }
| WORD WORD other EOSTMT
    {
    if (cmDependsFortranParserIsKeyword($1, "use"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleUse(parser, $2);
      }
    else if (cmDependsFortranParserIsKeyword($1, "module"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleModule(parser, $2);
      }
    else if (cmDependsFortranParserIsKeyword($1, "interface"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, true);
      }
    else if (cmDependsFortranParserIsKeyword($2, "interface") &&
             cmDependsFortranParserIsKeyword($1, "end"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, false);
      }
    free($1);
    free($2);
    }
| WORD DCOLON WORD other EOSTMT
    {
    if (cmDependsFortranParserIsKeyword($1, "use"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleUse(parser, $3);
      }
    free($1);
    free($3);
    }
| WORD COMMA WORD DCOLON WORD other EOSTMT
    {
    if (cmDependsFortranParserIsKeyword($1, "use") &&
        cmDependsFortranParserIsKeyword($3, "non_intrinsic") )
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleUse(parser, $5);
      }
    free($1);
    free($3);
    free($5);
    }
| WORD STRING other EOSTMT /* Ignore */
    {
    if (cmDependsFortranParserIsKeyword($1, "include"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleInclude(parser, $2);
      }
    free($1);
    free($2);
    }
| CPP_INCLUDE_ANGLE other EOSTMT
    {
    cmDependsFortranParser* parser =
      cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleInclude(parser, $1);
    free($1);
    }
| include STRING other EOSTMT
    {
    cmDependsFortranParser* parser =
      cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleInclude(parser, $2);
    free($2);
    }
| define WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleDefine(parser, $2);
    free($2);
    }
| undef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUndef(parser, $2);
    free($2);
    }
| ifdef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfdef(parser, $2);
    free($2);
    }
| ifndef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfndef(parser, $2);
    free($2);
    }
| if other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIf(parser);
    }
| elif other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElif(parser);
    }
| else other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElse(parser);
    }
| endif other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleEndif(parser);
    }
| WORD GARBAGE other EOSTMT             /* Ignore */
    {
    free($1);
    }
| GARBAGE other EOSTMT
| EOSTMT
| error
;



include: CPP_INCLUDE | F90PPR_INCLUDE | COCO_INCLUDE ;
define: CPP_DEFINE | F90PPR_DEFINE;
undef: CPP_UNDEF | F90PPR_UNDEF ;
ifdef: CPP_IFDEF | F90PPR_IFDEF ;
ifndef: CPP_IFNDEF | F90PPR_IFNDEF ;
if: CPP_IF | F90PPR_IF ;
elif: CPP_ELIF | F90PPR_ELIF ;
else: CPP_ELSE | F90PPR_ELSE ;
endif: CPP_ENDIF | F90PPR_ENDIF ;
other: /* empty */ | other misc_code ;

misc_code:
  WORD                { free ($1); }
| STRING              { free ($1); }
| GARBAGE
| ASSIGNMENT_OP
| DCOLON
| COMMA
| UNTERMINATED_STRING
;

%%
/* End of grammar */
