/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependsFortranParser_h
#define cmDependsFortranParser_h

#include <stddef.h> /* size_t */

/* Forward declare parser object type.  */
typedef struct cmDependsFortranParser_s cmDependsFortranParser;

/* Functions to enter/exit #include'd files in order.  */
bool cmDependsFortranParser_FilePush(cmDependsFortranParser* parser,
                                    const char* fname);
bool cmDependsFortranParser_FilePop(cmDependsFortranParser* parser);

/* Callbacks for lexer.  */
int cmDependsFortranParser_Input(cmDependsFortranParser* parser,
                                 char* buffer, size_t bufferSize);


void cmDependsFortranParser_StringStart(cmDependsFortranParser* parser);
const char* cmDependsFortranParser_StringEnd(cmDependsFortranParser* parser);
void cmDependsFortranParser_StringAppend(cmDependsFortranParser* parser,
                                         char c);

void cmDependsFortranParser_SetInInterface(cmDependsFortranParser* parser,
                                           bool is_in);
bool cmDependsFortranParser_GetInInterface(cmDependsFortranParser* parser);


void cmDependsFortranParser_SetInPPFalseBranch(cmDependsFortranParser* parser,
                                               bool is_in);
bool cmDependsFortranParser_GetInPPFalseBranch(cmDependsFortranParser* parser);


void cmDependsFortranParser_SetOldStartcond(cmDependsFortranParser* parser,
                                            int arg);
int cmDependsFortranParser_GetOldStartcond(cmDependsFortranParser* parser);

/* Callbacks for parser.  */
void cmDependsFortranParser_Error(cmDependsFortranParser* parser,
                                  const char* message);
void cmDependsFortranParser_RuleUse(cmDependsFortranParser* parser,
                                    const char* name);
void cmDependsFortranParser_RuleInclude(cmDependsFortranParser* parser,
                                        const char* name);
void cmDependsFortranParser_RuleModule(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleDefine(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleUndef(cmDependsFortranParser* parser,
                                      const char* name);
void cmDependsFortranParser_RuleIfdef(cmDependsFortranParser* parser,
                                      const char* name);
void cmDependsFortranParser_RuleIfndef(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleIf(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleElif(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleElse(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleEndif(cmDependsFortranParser* parser);

/* Define the parser stack element type.  */
typedef union cmDependsFortran_yystype_u cmDependsFortran_yystype;
union cmDependsFortran_yystype_u
{
  char* string;
};

/* Setup the proper yylex interface.  */
#define YY_EXTRA_TYPE cmDependsFortranParser*
#define YY_DECL \
int cmDependsFortran_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)
#define YYSTYPE cmDependsFortran_yystype
#define YYSTYPE_IS_DECLARED 1
#if !defined(cmDependsFortranLexer_cxx)
# include "cmDependsFortranLexer.h"
#endif
#if !defined(cmDependsFortranLexer_cxx)
#if !defined(cmDependsFortranParser_cxx)
# undef YY_EXTRA_TYPE
# undef YY_DECL
# undef YYSTYPE
# undef YYSTYPE_IS_DECLARED
#endif
#endif

#endif
