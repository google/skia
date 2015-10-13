/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetPropertyCommand_h
#define cmGetPropertyCommand_h

#include "cmCommand.h"

class cmGetPropertyCommand : public cmCommand
{
public:
  cmGetPropertyCommand();

  virtual cmCommand* Clone()
    {
      return new cmGetPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "get_property";}

  cmTypeMacro(cmGetPropertyCommand, cmCommand);
private:
  enum OutType { OutValue, OutDefined, OutBriefDoc, OutFullDoc, OutSet };
  std::string Variable;
  std::string Name;
  std::string PropertyName;
  OutType InfoType;

  // Implementation of result storage.
  bool StoreResult(const char* value);

  // Implementation of each property type.
  bool HandleGlobalMode();
  bool HandleDirectoryMode();
  bool HandleTargetMode();
  bool HandleSourceMode();
  bool HandleTestMode();
  bool HandleVariableMode();
  bool HandleCacheMode();
  bool HandleInstallMode();
};

#endif
