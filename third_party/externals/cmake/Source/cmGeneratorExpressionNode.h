/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorExpressionNode_h
#define cmGeneratorExpressionNode_h

#include "cmMakefile.h"

#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionParser.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpression.h"
#include "cmLocalGenerator.h"
#include "cmSourceFile.h"

#include <cmsys/String.h>

#include <assert.h>
#include <errno.h>

#include "cmListFileCache.h"

//----------------------------------------------------------------------------
struct cmGeneratorExpressionNode
{
  enum {
    DynamicParameters = 0,
    OneOrMoreParameters = -1,
    OneOrZeroParameters = -2
  };
  virtual ~cmGeneratorExpressionNode() {}

  virtual bool GeneratesContent() const { return true; }

  virtual bool RequiresLiteralInput() const { return false; }

  virtual bool AcceptsArbitraryContentParameter() const
    { return false; }

  virtual int NumExpectedParameters() const { return 1; }

  virtual std::string Evaluate(const std::vector<std::string> &parameters,
                               cmGeneratorExpressionContext *context,
                               const GeneratorExpressionContent *content,
                               cmGeneratorExpressionDAGChecker *dagChecker
                              ) const = 0;

  static std::string EvaluateDependentExpression(
    std::string const& prop, cmMakefile *makefile,
    cmGeneratorExpressionContext *context,
    cmTarget const* headTarget, cmTarget const* currentTarget,
    cmGeneratorExpressionDAGChecker *dagChecker);

  static const cmGeneratorExpressionNode* GetNode(
                                              const std::string &identifier);
};

//----------------------------------------------------------------------------
void reportError(cmGeneratorExpressionContext *context,
                 const std::string &expr, const std::string &result);

#endif
