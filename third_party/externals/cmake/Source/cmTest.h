/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTest_h
#define cmTest_h

#include "cmCustomCommand.h"
#include "cmPropertyMap.h"
#include "cmListFileCache.h"
class cmMakefile;

/** \class cmTest
 * \brief Represent a test
 *
 * cmTest is representation of a test.
 */
class cmTest
{
public:
  /**
   */
  cmTest(cmMakefile* mf);
  ~cmTest();

  ///! Set the test name
  void SetName(const std::string& name);
  std::string GetName() const { return this->Name; }

  void SetCommand(std::vector<std::string> const& command);
  std::vector<std::string> const& GetCommand() const
    {
    return this->Command;
    }

  ///! Set/Get a property of this source file
  void SetProperty(const std::string& prop, const char *value);
  void AppendProperty(const std::string& prop,
                      const char* value,bool asString=false);
  const char *GetProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  cmPropertyMap &GetProperties() { return this->Properties; }

  /** Get the cmMakefile instance that owns this test.  */
  cmMakefile *GetMakefile() { return this->Makefile;}

  /** Get the backtrace of the command that created this test.  */
  cmListFileBacktrace const& GetBacktrace() const;

  /** Get/Set whether this is an old-style test.  */
  bool GetOldStyle() const { return this->OldStyle; }
  void SetOldStyle(bool b) { this->OldStyle = b; }

private:
  cmPropertyMap Properties;
  std::string Name;
  std::vector<std::string> Command;

  bool OldStyle;

  cmMakefile* Makefile;
  cmListFileBacktrace Backtrace;
};

#endif

