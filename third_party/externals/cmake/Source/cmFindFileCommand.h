/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindFileCommand_h
#define cmFindFileCommand_h

#include "cmFindPathCommand.h"

/** \class cmFindFileCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindFileCommand is used to define a CMake variable
 * that specifies an executable program. The command searches
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindFileCommand : public cmFindPathCommand
{
public:
  cmFindFileCommand();
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFindFileCommand;
    }
  virtual std::string GetName() const { return "find_file";}

  cmTypeMacro(cmFindFileCommand, cmFindPathCommand);
};



#endif
