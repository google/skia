/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorExpressionLexer_h
#define cmGeneratorExpressionLexer_h

#include "cmStandardIncludes.h"

#include <vector>

//----------------------------------------------------------------------------
struct cmGeneratorExpressionToken
{
  cmGeneratorExpressionToken(unsigned type, const char *c, size_t l)
    : TokenType(type), Content(c), Length(l)
  {
  }
  enum {
    Text,
    BeginExpression,
    EndExpression,
    ColonSeparator,
    CommaSeparator
  };
  unsigned TokenType;
  const char *Content;
  size_t Length;
};

/** \class cmGeneratorExpressionLexer
 *
 */
class cmGeneratorExpressionLexer
{
public:
  cmGeneratorExpressionLexer();

  std::vector<cmGeneratorExpressionToken> Tokenize(const std::string& input);

  bool GetSawGeneratorExpression() const
  {
    return this->SawGeneratorExpression;
  }

private:
  bool SawBeginExpression;
  bool SawGeneratorExpression;
};

#endif
