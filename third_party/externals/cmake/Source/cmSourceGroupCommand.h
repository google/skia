/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSourceGroupCommand_h
#define cmSourceGroupCommand_h

#include "cmCommand.h"

/** \class cmSourceGroupCommand
 * \brief Adds a cmSourceGroup to the cmMakefile.
 *
 * cmSourceGroupCommand is used to define cmSourceGroups which split up
 * source files in to named, organized groups in the generated makefiles.
 */
class cmSourceGroupCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmSourceGroupCommand;
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
  virtual std::string GetName() const {return "source_group";}

  cmTypeMacro(cmSourceGroupCommand, cmCommand);
};



#endif
