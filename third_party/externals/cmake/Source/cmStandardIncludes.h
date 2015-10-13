/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/**
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

#include <cmConfigure.h>
#include <cmsys/Configure.hxx>

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#endif


#ifdef __ICL
#pragma warning ( disable : 985 )
#pragma warning ( disable : 1572 ) /* floating-point equality test */
#endif

// Provide fixed-size integer types.
#include <cmIML/INT.h>

// Include stream compatibility layer from KWSys.
// This is needed to work with large file support
// on some platforms whose stream operators do not
// support the large integer types.
#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/IOStream.hxx>
#endif

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// we must have stl with the standard include style
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <map>
#include <set>

// include the "c" string header
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( _MSC_VER )
typedef unsigned short mode_t;
#endif

// use this class to shrink the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
#include <cmsys/String.hxx>
//typedef cmsys::String std::string;

/* Poison this operator to avoid common mistakes.  */
extern void operator << (std::ostream&, const std::ostringstream&);

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  std::string Name;
  std::string Brief;
  cmDocumentationEntry(){}
  cmDocumentationEntry(const char *doc[2])
  { if (doc[0]) this->Name = doc[0];
  if (doc[1]) this->Brief = doc[1];}
  cmDocumentationEntry(const char *n, const char *b)
  { if (n) this->Name = n; if (b) this->Brief = b; }
};

/** Data structure to represent a single command line.  */
class cmCustomCommandLine: public std::vector<std::string>
{
public:
  typedef std::vector<std::string> Superclass;
  typedef Superclass::iterator iterator;
  typedef Superclass::const_iterator const_iterator;
};

/** Data structure to represent a list of command lines.  */
class cmCustomCommandLines: public std::vector<cmCustomCommandLine>
{
public:
  typedef std::vector<cmCustomCommandLine> Superclass;
  typedef Superclass::iterator iterator;
  typedef Superclass::const_iterator const_iterator;
};

// All subclasses of cmCommand or cmCTestGenericHandler should
// invoke this macro.
#define cmTypeMacro(thisClass,superclass) \
virtual const char* GetNameOfClass() { return #thisClass; } \
typedef superclass Superclass; \
static bool IsTypeOf(const char *type) \
{ \
  if ( !strcmp(#thisClass,type) ) \
    { \
    return true; \
    } \
  return Superclass::IsTypeOf(type); \
} \
virtual bool IsA(const char *type) \
{ \
  return thisClass::IsTypeOf(type); \
} \
static thisClass* SafeDownCast(cmObject *c) \
{ \
  if ( c && c->IsA(#thisClass) ) \
    { \
    return static_cast<thisClass *>(c); \
    } \
  return 0;\
} \
class cmTypeMacro_UseTrailingSemicolon

#endif
