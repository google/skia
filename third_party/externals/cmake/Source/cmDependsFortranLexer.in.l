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

Run flex like this:

  flex -i --prefix=cmDependsFortran_yy --header-file=cmDependsFortranLexer.h -ocmDependsFortranLexer.cxx cmDependsFortranLexer.in.l

Modify cmDependsFortranLexer.cxx:
  - remove TABs
  - remove use of the 'register' storage class specifier
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, cmDependsFortran_yyalloc, cmDependsFortran_yyrealloc, cmDependsFortran_yyfree
  - remove "yyscanner = NULL" from end of cmDependsFortran_yylex_destroy
  - remove all YY_BREAK lines occurring right after return statements
  - change while ( 1 ) to for(;;)

Modify cmDependsFortranLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#include "cmStandardLexer.h"

#define cmDependsFortranLexer_cxx
#include "cmDependsFortranParser.h" /* Interface to parser object.  */

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = cmDependsFortranParser_Input(yyextra, buf, max_size); }

/* Include the set of tokens from the parser.  */
#include "cmDependsFortranParserTokens.h"

/*--------------------------------------------------------------------------*/
%}


%option reentrant
%option noyywrap
%pointer

%s free_fmt fixed_fmt
%x str_sq str_dq

%%

\"              {
  cmDependsFortranParser_StringStart(yyextra);
  cmDependsFortranParser_SetOldStartcond(yyextra, YY_START);
  BEGIN(str_dq);
}

'               {
  cmDependsFortranParser_StringStart(yyextra);
  cmDependsFortranParser_SetOldStartcond(yyextra, YY_START);
  BEGIN(str_sq);
}

<str_dq>\" |
<str_sq>'  {
  BEGIN(cmDependsFortranParser_GetOldStartcond(yyextra) );
  yylvalp->string = strdup(cmDependsFortranParser_StringEnd(yyextra));
  return STRING;
}

<str_dq,str_sq>&[ \t]*\n |
<str_dq,str_sq>&[ \t]*\n[ \t]*&  /* Ignore (continued strings, free fmt) */

<fixed_fmt,str_dq,str_sq>\n[ ]{5}[^ \t\n] {
  if (cmDependsFortranParser_GetOldStartcond(yyextra) == fixed_fmt)
    ; /* Ignore (cont. strings, fixed fmt) */
  else
    {
    unput(yytext[strlen(yytext)-1]);
    }
}


<str_dq,str_sq>\n {
  unput ('\n');
  BEGIN(INITIAL);
  return UNTERMINATED_STRING;
}

<str_sq,str_dq>. {
  cmDependsFortranParser_StringAppend(yyextra, yytext[0]);
}

!.*\n                   { return EOSTMT; } /* Treat comments like */
<fixed_fmt>^[cC*dD].*\n { return EOSTMT; } /* empty lines */

^[ \t]*#[ \t]*include[ \t]*<[^>]+> {
  yytext[yyleng-1] = 0;
  yylvalp->string = strdup(strchr(yytext, '<')+1);
  return CPP_INCLUDE_ANGLE;
}
^[ \t]*#[ \t]*include  { return CPP_INCLUDE; }
\$[ \t]*include { return F90PPR_INCLUDE; }
\?\?[ \t]*include { return COCO_INCLUDE; }

^[ \t]*#[ \t]*define   { return CPP_DEFINE; }
\$[ \t]*DEFINE   { return F90PPR_DEFINE; }

^[ \t]*#[ \t]*undef    { return CPP_UNDEF; }
\$[ \t]*UNDEF   { return F90PPR_UNDEF; }

^[ \t]*#[ \t]*ifdef    { return CPP_IFDEF; }
^[ \t]*#[ \t]*ifndef   { return CPP_IFNDEF; }
^[ \t]*#[ \t]*if       { return CPP_IF; }
^[ \t]*#[ \t]*elif     { return CPP_ELIF; }
^[ \t]*#[ \t]*else     { return CPP_ELSE; }
^[ \t]*#[ \t]*endif    { return CPP_ENDIF; }

$[ \t]*ifdef    { return F90PPR_IFDEF; }
$[ \t]*ifndef   { return F90PPR_IFNDEF; }
$[ \t]*if       { return F90PPR_IF; }
$[ \t]*elif     { return F90PPR_ELIF; }
$[ \t]*else     { return F90PPR_ELSE; }
$[ \t]*endif    { return F90PPR_ENDIF; }

 /* Line continuations, possible involving comments.  */
&([ \t\n]*|!.*)*
&([ \t\n]*|!.*)*&

, { return COMMA; }

:: { return DCOLON; }

<fixed_fmt>\n[ ]{5}[^ ]  { return GARBAGE; }

=|=>                     { return ASSIGNMENT_OP; }

[a-zA-Z_][a-zA-Z_0-9]* {
  yylvalp->string = strdup(yytext);
  return WORD;
}

[^ \t\n\r;,!'"a-zA-Z=&]+ { return GARBAGE; }

;|\n { return EOSTMT; }


[ \t\r,]         /* Ignore */
\\[ \t]*\n       /* Ignore line-endings preceeded by \ */

. { return *yytext; }

<<EOF>> {
  if(!cmDependsFortranParser_FilePop(yyextra) )
    {
    return YY_NULL;
    }
}

%%

/*--------------------------------------------------------------------------*/
YY_BUFFER_STATE cmDependsFortranLexer_GetCurrentBuffer(yyscan_t yyscanner)
{
  /* Hack into the internal flex-generated scanner to get the buffer.  */
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  return YY_CURRENT_BUFFER;
}
