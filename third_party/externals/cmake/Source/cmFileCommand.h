/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFileCommand_h
#define cmFileCommand_h

#include "cmCommand.h"

struct cmFileInstaller;

/** \class cmFileCommand
 * \brief Command for manipulation of files
 *
 */
class cmFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmFileCommand;
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
  virtual std::string GetName() const { return "file";}

  cmTypeMacro(cmFileCommand, cmCommand);

protected:
  bool HandleRename(std::vector<std::string> const& args);
  bool HandleRemove(std::vector<std::string> const& args, bool recurse);
  bool HandleWriteCommand(std::vector<std::string> const& args, bool append);
  bool HandleReadCommand(std::vector<std::string> const& args);
  bool HandleHashCommand(std::vector<std::string> const& args);
  bool HandleStringsCommand(std::vector<std::string> const& args);
  bool HandleGlobCommand(std::vector<std::string> const& args, bool recurse);
  bool HandleMakeDirectoryCommand(std::vector<std::string> const& args);

  bool HandleRelativePathCommand(std::vector<std::string> const& args);
  bool HandleCMakePathCommand(std::vector<std::string> const& args,
                              bool nativePath);
  bool HandleRPathChangeCommand(std::vector<std::string> const& args);
  bool HandleRPathCheckCommand(std::vector<std::string> const& args);
  bool HandleRPathRemoveCommand(std::vector<std::string> const& args);
  bool HandleDifferentCommand(std::vector<std::string> const& args);

  bool HandleCopyCommand(std::vector<std::string> const& args);
  bool HandleInstallCommand(std::vector<std::string> const& args);
  bool HandleDownloadCommand(std::vector<std::string> const& args);
  bool HandleUploadCommand(std::vector<std::string> const& args);

  bool HandleTimestampCommand(std::vector<std::string> const& args);
  bool HandleGenerateCommand(std::vector<std::string> const& args);
  bool HandleLockCommand(std::vector<std::string> const& args);

private:
  void AddEvaluationFile(const std::string &inputName,
                         const std::string &outputExpr,
                         const std::string &condition,
                         bool inputIsContent);
};


#endif
