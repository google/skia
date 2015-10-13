/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorExpressionParser_h
#define cmGeneratorExpressionParser_h

#include "cmGeneratorExpressionLexer.h"

#include <set>
#include <vector>

#include "cmListFileCache.h"

class cmMakefile;
class cmTarget;
struct cmGeneratorExpressionEvaluator;

//----------------------------------------------------------------------------
struct cmGeneratorExpressionParser
{
  cmGeneratorExpressionParser(
                      const std::vector<cmGeneratorExpressionToken> &tokens);

  void Parse(std::vector<cmGeneratorExpressionEvaluator*> &result);

private:
  void ParseContent(std::vector<cmGeneratorExpressionEvaluator*> &);
  void ParseGeneratorExpression(
                              std::vector<cmGeneratorExpressionEvaluator*> &);

private:
  std::vector<cmGeneratorExpressionToken>::const_iterator it;
  const std::vector<cmGeneratorExpressionToken> Tokens;
  unsigned int NestingLevel;
};

#endif
