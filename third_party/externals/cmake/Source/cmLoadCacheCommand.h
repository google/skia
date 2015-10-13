/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLoadCacheCommand_h
#define cmLoadCacheCommand_h

#include "cmCommand.h"

/** \class cmLoadCacheCommand
 * \brief load a cache file
 *
 * cmLoadCacheCommand loads the non internal values of a cache file
 */
class cmLoadCacheCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmLoadCacheCommand;
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
  virtual std::string GetName() const { return "load_cache";}

  cmTypeMacro(cmLoadCacheCommand, cmCommand);
protected:

  std::set<std::string> VariablesToRead;
  std::string Prefix;

  bool ReadWithPrefix(std::vector<std::string> const& args);
  void CheckLine(const char* line);
};


#endif
