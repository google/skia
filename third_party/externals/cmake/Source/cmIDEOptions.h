/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIDEOptions_h
#define cmIDEOptions_h

#include "cmStandardIncludes.h"
#include "cmIDEFlagTable.h"

/** \class cmIDEOptions
 * \brief Superclass for IDE option processing
 */
class cmIDEOptions
{
public:
  cmIDEOptions();
  virtual ~cmIDEOptions();

  // Store definitions and flags.
  void AddDefine(const std::string& define);
  void AddDefines(const char* defines);
  void AddDefines(const std::vector<std::string> &defines);
  void AddFlag(const char* flag, const char* value);
  void AddFlag(const char* flag, std::vector<std::string> const& value);
  void AppendFlag(std::string const& flag, std::string const& value);
  void AppendFlag(std::string const& flag,
                  std::vector<std::string> const& value);
  void RemoveFlag(const char* flag);
  bool HasFlag(std::string const& flag) const;
  const char* GetFlag(const char* flag);

protected:
  // create a map of xml tags to the values they should have in the output
  // for example, "BufferSecurityCheck" = "TRUE"
  // first fill this table with the values for the configuration
  // Debug, Release, etc,
  // Then parse the command line flags specified in CMAKE_CXX_FLAGS
  // and CMAKE_C_FLAGS
  // and overwrite or add new values to this map
  class FlagValue: public std::vector<std::string>
  {
    typedef std::vector<std::string> derived;
  public:
    FlagValue& operator=(std::string const& r)
      {
      this->resize(1);
      this->operator[](0) = r;
      return *this;
      }
    FlagValue& operator=(std::vector<std::string> const& r)
      {
      this->derived::operator=(r);
      return *this;
      }
  };
  std::map<std::string, FlagValue > FlagMap;

  // Preprocessor definitions.
  std::vector<std::string> Defines;

  // Unrecognized flags that get no special handling.
  std::string FlagString;

  bool DoingDefine;
  bool AllowDefine;
  bool AllowSlash;
  cmIDEFlagTable const* DoingFollowing;
  enum { FlagTableCount = 16 };
  cmIDEFlagTable const* FlagTable[FlagTableCount];
  void HandleFlag(const char* flag);
  bool CheckFlagTable(cmIDEFlagTable const* table, const char* flag,
                      bool& flag_handled);
  void FlagMapUpdate(cmIDEFlagTable const* entry, const char* new_value);
  virtual void StoreUnknownFlag(const char* flag) = 0;
};

#endif
