/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCMakeHostSystemInformationCommand_h
#define cmCMakeHostSystemInformationCommand_h

#include "cmCommand.h"

#include <cmsys/SystemInformation.hxx>

/** \class cmCMakeHostSystemInformationCommand
 * \brief Query host system specific information
 *
 * cmCMakeHostSystemInformationCommand queries system information of
 * the sytem on which CMake runs.
 */
class cmCMakeHostSystemInformationCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmCMakeHostSystemInformationCommand;
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
  virtual std::string GetName() const
    {
    return "cmake_host_system_information";
    }

  cmTypeMacro(cmCMakeHostSystemInformationCommand, cmCommand);

private:
  bool GetValue(cmsys::SystemInformation &info,
    std::string const& key, std::string &value);

  std::string ValueToString(size_t value) const;
  std::string ValueToString(const char *value) const;
  std::string ValueToString(std::string const& value) const;
};

#endif
