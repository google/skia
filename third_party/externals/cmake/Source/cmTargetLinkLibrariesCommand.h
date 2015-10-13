/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetLinkLibrariesCommand_h
#define cmTargetLinkLibrariesCommand_h

#include "cmCommand.h"

/** \class cmTargetLinkLibrariesCommand
 * \brief Specify a list of libraries to link into executables.
 *
 * cmTargetLinkLibrariesCommand is used to specify a list of libraries to link
 * into executable(s) or shared objects. The names of the libraries
 * should be those defined by the LIBRARY(library) command(s).
 */
class cmTargetLinkLibrariesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetLinkLibrariesCommand;
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
  virtual std::string GetName() const { return "target_link_libraries";}

  cmTypeMacro(cmTargetLinkLibrariesCommand, cmCommand);
private:
  void LinkLibraryTypeSpecifierWarning(int left, int right);
  static const char* LinkLibraryTypeNames[3];

  cmTarget* Target;
  enum ProcessingState {
    ProcessingLinkLibraries,
    ProcessingPlainLinkInterface,
    ProcessingKeywordLinkInterface,
    ProcessingPlainPublicInterface,
    ProcessingKeywordPublicInterface,
    ProcessingPlainPrivateInterface,
    ProcessingKeywordPrivateInterface
  };

  ProcessingState CurrentProcessingState;

  bool HandleLibrary(const std::string& lib, cmTarget::LinkLibraryType llt);
};



#endif
