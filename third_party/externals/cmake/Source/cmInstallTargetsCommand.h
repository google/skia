/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallTargetsCommand_h
#define cmInstallTargetsCommand_h

#include "cmCommand.h"

/** \class cmInstallTargetsCommand
 * \brief Specifies where to install some targets
 *
 * cmInstallTargetsCommand specifies the relative path where a list of
 * targets should be installed. The targets can be executables or
 * libraries.
 */
class cmInstallTargetsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmInstallTargetsCommand;
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
  virtual std::string GetName() const { return "install_targets";}

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmInstallTargetsCommand, cmCommand);
};


#endif
