/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmListFileCache_h
#define cmListFileCache_h

#include "cmStandardIncludes.h"

class cmLocalGenerator;

/** \class cmListFileCache
 * \brief A class to cache list file contents.
 *
 * cmListFileCache is a class used to cache the contents of parsed
 * cmake list files.
 */

class cmMakefile;

struct cmListFileArgument
{
  enum Delimiter
    {
    Unquoted,
    Quoted,
    Bracket
    };
  cmListFileArgument(): Value(), Delim(Unquoted), FilePath(0), Line(0) {}
  cmListFileArgument(const cmListFileArgument& r):
    Value(r.Value), Delim(r.Delim), FilePath(r.FilePath), Line(r.Line) {}
  cmListFileArgument(const std::string& v, Delimiter d, const char* file,
                     long line): Value(v), Delim(d),
                                 FilePath(file), Line(line) {}
  bool operator == (const cmListFileArgument& r) const
    {
    return (this->Value == r.Value) && (this->Delim == r.Delim);
    }
  bool operator != (const cmListFileArgument& r) const
    {
    return !(*this == r);
    }
  std::string Value;
  Delimiter Delim;
  const char* FilePath;
  long Line;
};

struct cmListFileContext
{
  std::string Name;
  std::string FilePath;
  long Line;
  cmListFileContext(): Name(), FilePath(), Line(0) {}
};

std::ostream& operator<<(std::ostream&, cmListFileContext const&);
bool operator<(const cmListFileContext& lhs, const cmListFileContext& rhs);
bool operator==(cmListFileContext const& lhs, cmListFileContext const& rhs);
bool operator!=(cmListFileContext const& lhs, cmListFileContext const& rhs);

struct cmListFileFunction: public cmListFileContext
{
  std::vector<cmListFileArgument> Arguments;
};

class cmListFileBacktrace: private std::vector<cmListFileContext>
{
  public:
    cmListFileBacktrace(cmLocalGenerator* localGen)
      : LocalGenerator(localGen)
      , Relative(localGen ? false : true)
    {
    }

    void Append(cmListFileContext const& context);

    void MakeRelative();

    void PrintTitle(std::ostream& out);
    void PrintCallStack(std::ostream& out);
  private:
    cmLocalGenerator* LocalGenerator;
    bool Relative;
};

struct cmListFile
{
  bool ParseFile(const char* path,
                 bool topLevel,
                 cmMakefile *mf);

  std::vector<cmListFileFunction> Functions;
};

struct cmValueWithOrigin {
  cmValueWithOrigin(const std::string &value,
                          const cmListFileBacktrace &bt)
    : Value(value), Backtrace(bt)
  {}
  std::string Value;
  cmListFileBacktrace Backtrace;
};

#endif
