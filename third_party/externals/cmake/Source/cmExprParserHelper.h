/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExprParserHelper_h
#define cmExprParserHelper_h

#include "cmStandardIncludes.h"

#define YYSTYPE cmExprParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmExprParserHelper*
#define YY_DECL int cmExpr_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)

/** \class cmExprParserHelper
 * \brief Helper class for parsing java source files
 *
 * Finds dependencies for java file and list of outputs
 */

class cmMakefile;

class cmExprParserHelper
{
public:
  typedef struct {
    int Number;
  } ParserType;

  cmExprParserHelper();
  ~cmExprParserHelper();

  int ParseString(const char* str, int verb);

  int LexInput(char* buf, int maxlen);
  void Error(const char* str);

  void SetResult(int value);

  int GetResult() { return this->Result; }

  const char* GetError() { return this->ErrorString.c_str(); }

private:
  std::string::size_type InputBufferPos;
  std::string InputBuffer;
  std::vector<char> OutputBuffer;
  int CurrentLine;
  int Verbose;

  void Print(const char* place, const char* str);

  void CleanupParser();

  int Result;
  const char* FileName;
  long FileLine;
  std::string ErrorString;
};

#endif



