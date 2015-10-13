/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmEnableLanguageCommand_h
#define cmEnableLanguageCommand_h

#include "cmCommand.h"

/** \class cmEnableLanguageCommand
 * \brief Specify the name for this build project.
 *
 * cmEnableLanguageCommand is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories). Currently it just sets the name of the workspace
 * file for Microsoft Visual C++
 */
class cmEnableLanguageCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmEnableLanguageCommand;
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
  virtual std::string GetName() const {return "enable_language";}

  cmTypeMacro(cmEnableLanguageCommand, cmCommand);
};



#endif
