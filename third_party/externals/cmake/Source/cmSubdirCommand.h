/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSubdirCommand_h
#define cmSubdirCommand_h

#include "cmCommand.h"

/** \class cmSubdirCommand
 * \brief Specify a list of subdirectories to build.
 *
 * cmSubdirCommand specifies a list of subdirectories to process
 * by CMake. For each subdirectory listed, CMake will descend
 * into that subdirectory and process any CMakeLists.txt found.
 */
class cmSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmSubdirCommand;
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
  virtual std::string GetName() const { return "subdirs";}

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmSubdirCommand, cmCommand);
};



#endif
