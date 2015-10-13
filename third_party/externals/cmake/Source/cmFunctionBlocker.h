/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFunctionBlocker_h
#define cmFunctionBlocker_h

#include "cmStandardIncludes.h"
#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
class cmMakefile;

class cmFunctionBlocker
{
public:
  /**
   * should a function be blocked
   */
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile&mf,
                                 cmExecutionStatus &status) = 0;

  /**
   * should this function blocker be removed, useful when one function adds a
   * blocker and another must remove it
   */
  virtual bool ShouldRemove(const cmListFileFunction&,
                            cmMakefile&) {return false;}

  virtual ~cmFunctionBlocker() {}

  /** Set/Get the context in which this blocker is created.  */
  void SetStartingContext(cmListFileContext const& lfc)
    { this->StartingContext = lfc; }
  cmListFileContext const& GetStartingContext()
    { return this->StartingContext; }
private:
  cmListFileContext StartingContext;
};

#endif
