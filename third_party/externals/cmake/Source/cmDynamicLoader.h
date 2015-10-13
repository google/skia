/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// .NAME cmDynamicLoader - class interface to system dynamic libraries
// .SECTION Description
// cmDynamicLoader provides a portable interface to loading dynamic
// libraries into a process.


#ifndef cmDynamicLoader_h
#define cmDynamicLoader_h

#include "cmStandardIncludes.h"

#include <cmsys/DynamicLoader.hxx>

class cmDynamicLoader
{
public:
  // Description:
  // Load a dynamic library into the current process.
  // The returned cmsys::DynamicLoader::LibraryHandle can be used to access
  // the symbols in the library.
  static cmsys::DynamicLoader::LibraryHandle OpenLibrary(const char*);

  // Description:
  // Flush the cache of dynamic loader.
  static void FlushCache();

protected:
  cmDynamicLoader() {}
  ~cmDynamicLoader() {}

private:
  cmDynamicLoader(const cmDynamicLoader&);  // Not implemented.
  void operator=(const cmDynamicLoader&);  // Not implemented.
};

#endif
