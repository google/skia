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

Run flex like this:

  flex --prefix=cmExpr_yy --header-file=cmExprLexer.h -ocmExprLexer.cxx cmExprLexer.in.l

Modify cmExprLexer.cxx:
  - remove TABs
  - remove use of the 'register' storage class specifier
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, cmExpr_yyalloc, cmExpr_yyrealloc, cmExpr_yyfree
  - remove all YY_BREAK lines occurring right after return statements
  - change while ( 1 ) to for(;;)

Modify cmExprLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#include "cmStandardLexer.h"

#include "cmExprParserHelper.h"

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = yyextra->LexInput(buf, max_size); }

/* Include the set of tokens from the parser.  */
#include "cmExprParserTokens.h"

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option noyywrap
%pointer

%%

[0-9][0-9]* { yylvalp->Number = atoi(yytext); return exp_NUMBER; }

"+" { return exp_PLUS; }
"-" { return exp_MINUS; }
"*" { return exp_TIMES; }
"/" { return exp_DIVIDE; }
"%" { return exp_MOD; }
"\|" { return exp_OR; }
"&" { return exp_AND; }
"^" { return exp_XOR; }
"~" { return exp_NOT; }
"<<" { return exp_SHIFTLEFT; }
">>" { return exp_SHIFTRIGHT; }
"(" { return exp_OPENPARENT; }
")" { return exp_CLOSEPARENT; }

%%
