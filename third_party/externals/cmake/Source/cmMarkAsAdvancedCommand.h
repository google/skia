/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMarkAsAdvancedCommand_h
#define cmMarkAsAdvancedCommand_h

#include "cmCommand.h"

/** \class cmMarkAsAdvancedCommand
 * \brief mark_as_advanced command
 *
 * cmMarkAsAdvancedCommand implements the mark_as_advanced CMake command
 */
class cmMarkAsAdvancedCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmMarkAsAdvancedCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual std::string GetName() const {return "mark_as_advanced";}

  /**
   * This determines if the command is invoked when in script mode.
   * mark_as_advanced() will have no effect in script mode, but this will
   * make many of the modules usable in cmake/ctest scripts, (among them
   * FindUnixMake.cmake used by the CTEST_BUILD command.
  */
  virtual bool IsScriptable() const { return true; }

  cmTypeMacro(cmMarkAsAdvancedCommand, cmCommand);
};



#endif
