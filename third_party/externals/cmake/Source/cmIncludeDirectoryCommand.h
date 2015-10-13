/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIncludeDirectoryCommand_h
#define cmIncludeDirectoryCommand_h

#include "cmCommand.h"

/** \class cmIncludeDirectoryCommand
 * \brief Add include directories to the build.
 *
 * cmIncludeDirectoryCommand is used to specify directory locations
 * to search for included files.
 */
class cmIncludeDirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmIncludeDirectoryCommand;
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
  virtual std::string GetName() const { return "include_directories";}

  cmTypeMacro(cmIncludeDirectoryCommand, cmCommand);

protected:
  // used internally
  void GetIncludes(const std::string &arg, std::vector<std::string> &incs);
  void NormalizeInclude(std::string &inc);
};



#endif
