/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTryRunCommand_h
#define cmTryRunCommand_h

#include "cmCoreTryCompile.h"

/** \class cmTryRunCommand
 * \brief Specifies where to install some files
 *
 * cmTryRunCommand is used to test if soucre code can be compiled
 */
class cmTryRunCommand : public cmCoreTryCompile
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTryRunCommand;
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
  virtual std::string GetName() const { return "try_run";}

  cmTypeMacro(cmTryRunCommand, cmCoreTryCompile);
private:
  void RunExecutable(const std::string& runArgs,
                     std::string* runOutputContents);
  void DoNotRunExecutable(const std::string& runArgs,
                          const std::string& srcFile,
                          std::string* runOutputContents);

  std::string CompileResultVariable;
  std::string RunResultVariable;
  std::string OutputVariable;
  std::string RunOutputVariable;
  std::string CompileOutputVariable;
};


#endif
