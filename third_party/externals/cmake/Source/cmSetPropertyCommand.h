/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSetsPropertiesCommand_h
#define cmSetsPropertiesCommand_h

#include "cmCommand.h"

class cmSetPropertyCommand : public cmCommand
{
public:
  cmSetPropertyCommand();

  virtual cmCommand* Clone()
    {
      return new cmSetPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const { return "set_property";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  cmTypeMacro(cmSetPropertyCommand, cmCommand);

private:
  std::set<std::string> Names;
  std::string PropertyName;
  std::string PropertyValue;
  bool Remove;
  bool AppendMode;
  bool AppendAsString;

  // Implementation of each property type.
  bool HandleGlobalMode();
  bool HandleDirectoryMode();
  bool HandleTargetMode();
  bool HandleTarget(cmTarget* target);
  bool HandleSourceMode();
  bool HandleSource(cmSourceFile* sf);
  bool HandleTestMode();
  bool HandleTest(cmTest* test);
  bool HandleCacheMode();
  bool HandleCacheEntry(std::string const&);
  bool HandleInstallMode();
  bool HandleInstall(cmInstalledFile* file);
};



#endif
