/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddSubDirectoryCommand_h
#define cmAddSubDirectoryCommand_h

#include "cmCommand.h"

/** \class cmAddSubDirectoryCommand
 * \brief Specify a subdirectory to build
 *
 * cmAddSubDirectoryCommand specifies a subdirectory to process
 * by CMake. CMake will descend
 * into the specified source directory and process any CMakeLists.txt found.
 */
class cmAddSubDirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmAddSubDirectoryCommand;
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
  virtual std::string GetName() const { return "add_subdirectory";}

  cmTypeMacro(cmAddSubDirectoryCommand, cmCommand);
};



#endif
