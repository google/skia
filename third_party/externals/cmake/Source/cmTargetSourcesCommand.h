/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmTargetSourcesCommand_h
#define cmTargetSourcesCommand_h

#include "cmTargetPropCommandBase.h"

//----------------------------------------------------------------------------
class cmTargetSourcesCommand : public cmTargetPropCommandBase
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetSourcesCommand;
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
  virtual std::string GetName() const { return "target_sources";}

  cmTypeMacro(cmTargetSourcesCommand, cmTargetPropCommandBase);

private:
  virtual void HandleImportedTarget(const std::string &tgt);
  virtual void HandleMissingTarget(const std::string &name);

  virtual bool HandleDirectContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);

  virtual std::string Join(const std::vector<std::string> &content);
};

#endif
