/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSetTestsPropertiesCommand_h
#define cmSetTestsPropertiesCommand_h

#include "cmCommand.h"

class cmSetTestsPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone()
    {
    return new cmSetTestsPropertiesCommand;
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
  virtual std::string GetName() const { return "set_tests_properties";}

  cmTypeMacro(cmSetTestsPropertiesCommand, cmCommand);

  static bool SetOneTest(const std::string& tname,
                         std::vector<std::string> &propertyPairs,
                         cmMakefile *mf,
                         std::string &errors);
};



#endif
