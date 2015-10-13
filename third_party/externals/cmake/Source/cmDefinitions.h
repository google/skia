/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDefinitions_h
#define cmDefinitions_h

#include "cmStandardIncludes.h"
#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
#include <unordered_map>
#else
#include "cmsys/hash_map.hxx"
#endif
#endif

#include <list>

/** \class cmDefinitions
 * \brief Store a scope of variable definitions for CMake language.
 *
 * This stores the state of variable definitions (set or unset) for
 * one scope.  Sets are always local.  Gets search parent scopes
 * transitively and save results locally.
 */
class cmDefinitions
{
  typedef std::list<cmDefinitions>::reverse_iterator StackIter;
  typedef std::list<cmDefinitions>::const_reverse_iterator StackConstIter;
public:
  static const char* Get(const std::string& key,
                         StackIter begin, StackIter end);

  static void Raise(const std::string& key, StackIter begin, StackIter end);

  static bool HasKey(const std::string& key,
                     StackConstIter begin, StackConstIter end);

  /** Set (or unset if null) a value associated with a key.  */
  void Set(const std::string& key, const char* value);

  std::vector<std::string> UnusedKeys() const;

  static std::vector<std::string> ClosureKeys(StackConstIter begin,
                                              StackConstIter end);

  static cmDefinitions MakeClosure(StackConstIter begin, StackConstIter end);

private:
  // String with existence boolean.
  struct Def: public std::string
  {
  private:
    typedef std::string std_string;
  public:
    Def(): std_string(), Exists(false), Used(false) {}
    Def(const char* v)
      : std_string(v ? v : ""),
        Exists(v ? true : false),
        Used(false)
    {}
    Def(const std_string& v): std_string(v), Exists(true), Used(false) {}
    Def(Def const& d): std_string(d), Exists(d.Exists), Used(d.Used) {}
    bool Exists;
    bool Used;
  };
  static Def NoDef;

#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
  typedef std::unordered_map<std::string, Def> MapType;
#else
  typedef cmsys::hash_map<std::string, Def> MapType;
#endif
#else
  typedef std::map<std::string, Def> MapType;
#endif
  MapType Map;

  static Def const& GetInternal(const std::string& key,
    StackIter begin, StackIter end, bool raise);
};

#endif
