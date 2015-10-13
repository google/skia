/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmcmd_h
#define cmcmd_h

#include "cmStandardIncludes.h"

class cmcmd
{
public:

  /**
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int ExecuteCMakeCommand(std::vector<std::string>&);
protected:

  static int SymlinkLibrary(std::vector<std::string>& args);
  static int SymlinkExecutable(std::vector<std::string>& args);
  static bool SymlinkInternal(std::string const& file,
                              std::string const& link);
  static int ExecuteEchoColor(std::vector<std::string>& args);
  static int ExecuteLinkScript(std::vector<std::string>& args);
  static int WindowsCEEnvironment(const char* version,
                                  const std::string& name);
  static int VisualStudioLink(std::vector<std::string>& args, int type);
  static int VisualStudioLinkIncremental(std::vector<std::string>& args,
                                         int type,
                                         bool verbose);
  static int VisualStudioLinkNonIncremental(std::vector<std::string>& args,
                                            int type,
                                            bool hasManifest,
                                            bool verbose);
  static int ParseVisualStudioLinkCommand(std::vector<std::string>& args,
                                          std::vector<std::string>& command,
                                          std::string& targetName);
  static bool RunCommand(const char* comment,
                         std::vector<std::string>& command,
                         bool verbose,
                         int* retCodeOut = 0);
};

#endif
