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
/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --yacc --name-prefix=cmExpr_yy --defines=cmExprParserTokens.h -ocmExprParser.cxx cmExprParser.y

Modify cmExprParser.cxx:
  - remove TABs
  - remove use of the 'register' storage class specifier
  - add __HP_aCC to the #if test for yyerrorlab warning suppression

*/

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmExpr_yyerror(x) \
        cmExprError(yyscanner, x)
#define yyGetParser (cmExpr_yyget_extra(yyscanner))

/*-------------------------------------------------------------------------*/
#include "cmExprParserHelper.h" /* Interface to parser object.  */
#include "cmExprLexer.h"  /* Interface to lexer object.  */
#include "cmExprParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

#include <math.h>

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Internal utility functions.  */
static void cmExprError(yyscan_t yyscanner, const char* message);

#define YYDEBUG 1
//#define YYMAXDEPTH 100000
//#define YYINITDEPTH 10000


/* Disable some warnings in the generated code.  */
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch statement contains default but no case. */
#endif
%}

/* Generate a reentrant parser object.  */
%pure_parser

/*-------------------------------------------------------------------------*/
/* Tokens */
%token exp_PLUS
%token exp_MINUS
%token exp_TIMES
%token exp_DIVIDE
%token exp_MOD
%token exp_SHIFTLEFT
%token exp_SHIFTRIGHT
%token exp_OPENPARENT
%token exp_CLOSEPARENT
%token exp_OR;
%token exp_AND;
%token exp_XOR;
%token exp_NOT;
%token exp_NUMBER;

/*-------------------------------------------------------------------------*/
/* grammar */
%%


Start:
exp
{
  yyGetParser->SetResult($<Number>1);
}

exp:
bitwiseor
{$<Number>$ = $<Number>1;}
|
exp exp_OR bitwiseor
{$<Number>$ = $<Number>1 | $<Number>3;}

bitwiseor:
bitwisexor
{$<Number>$ = $<Number>1;}
|
bitwiseor exp_XOR bitwisexor
{$<Number>$ = $<Number>1 ^ $<Number>3;}

bitwisexor:
bitwiseand
{$<Number>$ = $<Number>1;}
|
bitwisexor exp_AND bitwiseand
{$<Number>$ = $<Number>1 & $<Number>3;}

bitwiseand:
shift
{$<Number>$ = $<Number>1;}
|
bitwiseand exp_SHIFTLEFT shift
{$<Number>$ = $<Number>1 << $<Number>3;}
|
bitwiseand exp_SHIFTRIGHT shift
{$<Number>$ = $<Number>1 >> $<Number>3;}


shift:
term
{$<Number>$ = $<Number>1;}
|
shift exp_PLUS term
{$<Number>$ = $<Number>1 + $<Number>3;}
|
shift exp_MINUS term
{$<Number>$ = $<Number>1 - $<Number>3;}

term:
factor
{$<Number>$ = $<Number>1;}
|
term exp_TIMES factor
{$<Number>$ = $<Number>1 * $<Number>3;}
|
term exp_DIVIDE factor
{$<Number>$ = $<Number>1 / $<Number>3;}
|
term exp_MOD factor
{$<Number>$ = $<Number>1 % $<Number>3;}

factor:
exp_NUMBER
{$<Number>$ = $<Number>1;}
|
exp_OPENPARENT exp exp_CLOSEPARENT
{$<Number>$ = $<Number>2;}
;


%%
/* End of grammar */

/*--------------------------------------------------------------------------*/
void cmExprError(yyscan_t yyscanner, const char* message)
{
  yyGetParser->Error(message);
}

