/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"

#include KWSYS_HEADER(DynamicLoader.hxx)
#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(stl/string)

#if defined(__BEOS__) || defined(__HAIKU__)
#include <be/kernel/OS.h>  /* disable_debugger() API. */
#endif

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "DynamicLoader.hxx.in"
# include "kwsys_ios_iostream.h.in"
# include "kwsys_stl_string.hxx.in"
#endif

// Include with <> instead of "" to avoid getting any in-source copy
// left on disk.
#include <testSystemTools.h>

static kwsys_stl::string GetLibName(const char* lname)
{
  // Construct proper name of lib
  kwsys_stl::string slname;
  slname = EXECUTABLE_OUTPUT_PATH;
#ifdef CMAKE_INTDIR
  slname += "/";
  slname += CMAKE_INTDIR;
#endif
  slname += "/";
  slname += kwsys::DynamicLoader::LibPrefix();
  slname += lname;
  slname += kwsys::DynamicLoader::LibExtension();

  return slname;
}

/* libname = Library name (proper prefix, proper extension)
 * System  = symbol to lookup in libname
 * r1: should OpenLibrary succeed ?
 * r2: should GetSymbolAddress succeed ?
 * r3: should CloseLibrary succeed ?
 */
int TestDynamicLoader(const char* libname, const char* symbol, int r1, int r2, int r3)
{
  kwsys_ios::cerr << "Testing: " << libname << kwsys_ios::endl;
  kwsys::DynamicLoader::LibraryHandle l
    = kwsys::DynamicLoader::OpenLibrary(libname);
  // If result is incompatible with expectation just fails (xor):
  if( (r1 && !l) || (!r1 && l) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
  kwsys::DynamicLoader::SymbolPointer f
    = kwsys::DynamicLoader::GetSymbolAddress(l, symbol);
  if( (r2 && !f) || (!r2 && f) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
#ifndef __APPLE__
  int s = kwsys::DynamicLoader::CloseLibrary(l);
  if( (r3 && !s) || (!r3 && s) )
    {
    kwsys_ios::cerr
      << kwsys::DynamicLoader::LastError() << kwsys_ios::endl;
    return 1;
    }
#else
  (void)r3;
#endif
  return 0;
}

int testDynamicLoader(int argc, char *argv[])
{
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#elif defined(__BEOS__) || defined(__HAIKU__)
  disable_debugger(1);
#endif
  int res = 0;
  if( argc == 3 )
    {
    // User specify a libname and symbol to check.
    res = TestDynamicLoader(argv[1], argv[2],1,1,1);
    return res;
    }

// dlopen() on Syllable before 11/22/2007 doesn't return 0 on error
#ifndef __SYLLABLE__
  // Make sure that inexistent lib is giving correct result
  res += TestDynamicLoader("azerty_", "foo_bar",0,0,0);
  // Make sure that random binary file cannot be assimilated as dylib
  res += TestDynamicLoader(TEST_SYSTEMTOOLS_SOURCE_DIR "/testSystemTools.bin", "wp",0,0,0);
#endif

#ifdef __linux__
  // This one is actually fun to test, since dlopen is by default loaded...wonder why :)
  res += TestDynamicLoader("foobar.lib", "dlopen",0,1,0);
  res += TestDynamicLoader("libdl.so", "dlopen",1,1,1);
  res += TestDynamicLoader("libdl.so", "TestDynamicLoader",1,0,1);
#endif
  // Now try on the generated library
  kwsys_stl::string libname = GetLibName(KWSYS_NAMESPACE_STRING "TestDynload");
  res += TestDynamicLoader(libname.c_str(), "dummy",1,0,1);
  res += TestDynamicLoader(libname.c_str(), "TestDynamicLoaderSymbolPointer",1,1,1);
  res += TestDynamicLoader(libname.c_str(), "_TestDynamicLoaderSymbolPointer",1,0,1);
  res += TestDynamicLoader(libname.c_str(), "TestDynamicLoaderData",1,1,1);
  res += TestDynamicLoader(libname.c_str(), "_TestDynamicLoaderData",1,0,1);

  return res;
}
