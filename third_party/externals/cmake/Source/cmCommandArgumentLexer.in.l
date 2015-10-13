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

  flex --prefix=cmCommandArgument_yy --header-file=cmCommandArgumentLexer.h -ocmCommandArgumentLexer.cxx cmCommandArgumentLexer.in.l

Modify cmCommandArgumentLexer.cxx:
  - add #include "cmStandardIncludes.h" to top of file
  - put header block at top of file
  - remove TABs
  - remove use of the 'register' storage class specifier
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, cmCommandArgument_yyalloc, cmCommandArgument_yyrealloc, cmCommandArgument_yyfree
  - remove all YY_BREAK lines occurring right after return statements
  - change while ( 1 ) to for(;;)
  - add "return 0;" to end of cmCommandArgument_yylex

Modify cmCommandArgumentLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#include "cmStandardLexer.h"

#include "cmCommandArgumentParserHelper.h"

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = yyextra->LexInput(buf, max_size); }

/* Include the set of tokens from the parser.  */
#include "cmCommandArgumentParserTokens.h"

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option noyywrap
%option nounput
%pointer
%s ESCAPES
%s NOESCAPES

%%

\$ENV\{ {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2);
  return cal_ENVCURLY;
}

\$[A-Za-z0-9/_.+-]+\{ {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2);
  return cal_NCURLY;
}

@[A-Za-z0-9/_.+-]+@ {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2);
  return cal_ATNAME;
}

"${" {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->DCURLYVariable;
  return cal_DCURLY;
}

"}" {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->RCURLYVariable;
  return cal_RCURLY;
}

"@" {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->ATVariable;
  return cal_AT;
}

[A-Za-z0-9/_.+-]+ {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  return cal_NAME;
}

<ESCAPES>\\. {
  if ( !yyextra->HandleEscapeSymbol(yylvalp, *(yytext+1)) )
    {
    return cal_ERROR;
    }
  return cal_SYMBOL;
}

[^\${}\\@]+ {
  //std::cerr << __LINE__ << " here: [" << yytext << "]" << std::endl;
  yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  return cal_SYMBOL;
}

"$" {
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->DOLLARVariable;
  return cal_DOLLAR;
}

"{" {
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->LCURLYVariable;
  return cal_LCURLY;
}

<ESCAPES>"\\" {
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->BSLASHVariable;
  return cal_BSLASH;
}

<NOESCAPES>"\\" {
  //yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext));
  yylvalp->str = yyextra->BSLASHVariable;
  return cal_SYMBOL;
}

%%

/*--------------------------------------------------------------------------*/
void cmCommandArgument_SetupEscapes(yyscan_t yyscanner, bool noEscapes)
{
  /* Hack into the internal flex-generated scanner to set the state.  */
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  if(noEscapes)
    {
    BEGIN(NOESCAPES);
    }
  else
    {
    BEGIN(ESCAPES);
    }
}
