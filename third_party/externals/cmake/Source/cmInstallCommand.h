/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallCommand_h
#define cmInstallCommand_h

#include "cmCommand.h"

/** \class cmInstallCommand
 * \brief Specifies where to install some files
 *
 * cmInstallCommand is a general-purpose interface command for
 * specifying install rules.
 */
class cmInstallCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmInstallCommand;
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
  virtual std::string GetName() const { return "install";}

  cmTypeMacro(cmInstallCommand, cmCommand);

private:
  bool HandleScriptMode(std::vector<std::string> const& args);
  bool HandleTargetsMode(std::vector<std::string> const& args);
  bool HandleFilesMode(std::vector<std::string> const& args);
  bool HandleDirectoryMode(std::vector<std::string> const& args);
  bool HandleExportMode(std::vector<std::string> const& args);
  bool MakeFilesFullPath(const char* modeName,
                         const std::vector<std::string>& relFiles,
                         std::vector<std::string>& absFiles);
  bool CheckCMP0006(bool& failure);

  std::string DefaultComponentName;
};


#endif
