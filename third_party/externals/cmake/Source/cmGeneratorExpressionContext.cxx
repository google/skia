/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGeneratorExpressionContext.h"

cmGeneratorExpressionContext::cmGeneratorExpressionContext(
      cmMakefile* mf, std::string const& config,
      bool quiet, cmTarget const* headTarget,
      cmTarget const* currentTarget,
      bool evaluateForBuildsystem,
      cmListFileBacktrace const& backtrace,
      std::string const& language)
  : Backtrace(backtrace),
    Makefile(mf),
    Config(config),
    Language(language),
    HeadTarget(headTarget),
    CurrentTarget(currentTarget),
    Quiet(quiet),
    HadError(false),
    HadContextSensitiveCondition(false),
    HadHeadSensitiveCondition(false),
    EvaluateForBuildsystem(evaluateForBuildsystem)
{
}
