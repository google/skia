/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecutionStatus_h
#define cmExecutionStatus_h

#include "cmStandardIncludes.h"

/** \class cmExecutionStatus
 * \brief Superclass for all command status classes
 *
 * when a command is involked it may set values on a command status instance
 */
class cmExecutionStatus
{
public:
  cmExecutionStatus() { this->Clear(); }

  void SetReturnInvoked(bool val)
  { this->ReturnInvoked = val; }
  bool GetReturnInvoked()
  { return this->ReturnInvoked; }

  void SetBreakInvoked(bool val)
  { this->BreakInvoked = val; }
  bool GetBreakInvoked()
  { return this->BreakInvoked; }

  void SetContinueInvoked(bool val)
  { this->ContinueInvoked = val; }
  bool GetContinueInvoked()
  { return this->ContinueInvoked; }

  void Clear()
    {
    this->ReturnInvoked = false;
    this->BreakInvoked = false;
    this->ContinueInvoked = false;
    this->NestedError = false;
    }
  void SetNestedError(bool val) { this->NestedError = val; }
  bool GetNestedError() { return this->NestedError; }

private:
  bool ReturnInvoked;
  bool BreakInvoked;
  bool ContinueInvoked;
  bool NestedError;
};

#endif
