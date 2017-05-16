/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGeneratorExpressionDAGChecker_h
#define cmGeneratorExpressionDAGChecker_h

#include "cmStandardIncludes.h"

#include "cmGeneratorExpressionEvaluator.h"

#define CM_SELECT_BOTH(F, A1, A2) F(A1, A2)
#define CM_SELECT_FIRST(F, A1, A2) F(A1)
#define CM_SELECT_SECOND(F, A1, A2) F(A2)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, SELECT) \
  SELECT(F, EvaluatingIncludeDirectories,       INCLUDE_DIRECTORIES) \
  SELECT(F, EvaluatingSystemIncludeDirectories, SYSTEM_INCLUDE_DIRECTORIES) \
  SELECT(F, EvaluatingCompileDefinitions,       COMPILE_DEFINITIONS) \
  SELECT(F, EvaluatingCompileOptions,           COMPILE_OPTIONS) \
  SELECT(F, EvaluatingAutoUicOptions,           AUTOUIC_OPTIONS) \
  SELECT(F, EvaluatingSources,                  SOURCES) \
  SELECT(F, EvaluatingCompileFeatures,          COMPILE_FEATURES)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY(F) \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_BOTH)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(F) \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_FIRST)

#define CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(F) \
  CM_FOR_EACH_TRANSITIVE_PROPERTY_IMPL(F, CM_SELECT_SECOND)

//----------------------------------------------------------------------------
struct cmGeneratorExpressionDAGChecker
{
  cmGeneratorExpressionDAGChecker(const cmListFileBacktrace &backtrace,
                                  const std::string &target,
                                  const std::string &property,
                                  const GeneratorExpressionContent *content,
                                  cmGeneratorExpressionDAGChecker *parent);
  cmGeneratorExpressionDAGChecker(const std::string &target,
                                  const std::string &property,
                                  const GeneratorExpressionContent *content,
                                  cmGeneratorExpressionDAGChecker *parent);

  enum Result {
    DAG,
    SELF_REFERENCE,
    CYCLIC_REFERENCE,
    ALREADY_SEEN
  };

  Result Check() const;

  void ReportError(cmGeneratorExpressionContext *context,
                   const std::string &expr);

  bool EvaluatingLinkLibraries(const char *tgt = 0);

#define DECLARE_TRANSITIVE_PROPERTY_METHOD(METHOD) \
  bool METHOD () const;

  CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(DECLARE_TRANSITIVE_PROPERTY_METHOD)

#undef DECLARE_TRANSITIVE_PROPERTY_METHOD

  bool GetTransitivePropertiesOnly();
  void SetTransitivePropertiesOnly()
    { this->TransitivePropertiesOnly = true; }

  std::string TopTarget() const;

private:
  Result CheckGraph() const;
  void Initialize();

private:
  const cmGeneratorExpressionDAGChecker * const Parent;
  const std::string Target;
  const std::string Property;
  std::map<std::string, std::set<std::string> > Seen;
  const GeneratorExpressionContent * const Content;
  const cmListFileBacktrace Backtrace;
  Result CheckResult;
  bool TransitivePropertiesOnly;
};

#endif
