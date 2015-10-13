/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetFilenameComponentCommand_h
#define cmGetFilenameComponentCommand_h

#include "cmCommand.h"

/** \class cmGetFilenameComponentCommand
 * \brief Get a specific component of a filename.
 *
 * cmGetFilenameComponentCommand is a utility command used to get the path,
 * name, extension or name without extension of a full filename.
 */
class cmGetFilenameComponentCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmGetFilenameComponentCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
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
  virtual std::string GetName() const { return "get_filename_component";}

  cmTypeMacro(cmGetFilenameComponentCommand, cmCommand);
};



#endif
