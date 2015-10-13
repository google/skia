/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmTargetPropCommandBase_h
#define cmTargetPropCommandBase_h

#include "cmCommand.h"

class cmTarget;

//----------------------------------------------------------------------------
class cmTargetPropCommandBase : public cmCommand
{
public:

  enum ArgumentFlags {
    NO_FLAGS = 0,
    PROCESS_BEFORE = 1,
    PROCESS_SYSTEM = 2
  };

  bool HandleArguments(std::vector<std::string> const& args,
                       const std::string& prop,
                       ArgumentFlags flags = NO_FLAGS);

  cmTypeMacro(cmTargetPropCommandBase, cmCommand);
protected:
  std::string Property;
  cmTarget *Target;

  virtual void HandleInterfaceContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system);
private:
  virtual void HandleImportedTarget(const std::string &tgt) = 0;
  virtual void HandleMissingTarget(const std::string &name) = 0;

  virtual bool HandleDirectContent(cmTarget *tgt,
                                   const std::vector<std::string> &content,
                                   bool prepend, bool system) = 0;

  virtual std::string Join(const std::vector<std::string> &content) = 0;

  bool ProcessContentArgs(std::vector<std::string> const& args,
                          unsigned int &argIndex, bool prepend, bool system);
  bool PopulateTargetProperies(const std::string &scope,
                               const std::vector<std::string> &content,
                               bool prepend, bool system);
};

#endif
