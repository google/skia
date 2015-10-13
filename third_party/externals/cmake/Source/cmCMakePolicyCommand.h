/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCMakePolicyCommand_h
#define cmCMakePolicyCommand_h

#include "cmCommand.h"

/** \class cmCMakePolicyCommand
 * \brief Set how CMake should handle policies
 *
 * cmCMakePolicyCommand sets how CMake should deal with backwards
 * compatibility policies.
 */
class cmCMakePolicyCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmCMakePolicyCommand;
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
  virtual std::string GetName() const {return "cmake_policy";}

  cmTypeMacro(cmCMakePolicyCommand, cmCommand);
private:
  bool HandleSetMode(std::vector<std::string> const& args);
  bool HandleGetMode(std::vector<std::string> const& args);
  bool HandleVersionMode(std::vector<std::string> const& args);
};



#endif
