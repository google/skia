/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCoreTryCompile_h
#define cmCoreTryCompile_h

#include "cmCommand.h"

/** \class cmCoreTryCompile
 * \brief Base class for cmTryCompileCommand and cmTryRunCommand
 *
 * cmCoreTryCompile implements the functionality to build a program.
 * It is the base class for cmTryCompileCommand and cmTryRunCommand.
 */
class cmCoreTryCompile : public cmCommand
{
public:

  protected:
  /**
   * This is the core code for try compile. It is here so that other
   * commands, such as TryRun can access the same logic without
   * duplication.
   */
  int TryCompileCode(std::vector<std::string> const& argv);

  /**
   * This deletes all the files created by TryCompileCode.
   * This way we do not have to rely on the timing and
   * dependencies of makefiles.
   */
  void CleanupFiles(const char* binDir);

  /**
   * This tries to find the (executable) file created by
  TryCompileCode. The result is stored in OutputFile. If nothing is found,
  the error message is stored in FindErrorMessage.
   */
  void FindOutputFile(const std::string& targetName);


  cmTypeMacro(cmCoreTryCompile, cmCommand);

  std::string BinaryDirectory;
  std::string OutputFile;
  std::string FindErrorMessage;
  bool SrcFileSignature;

};


#endif
