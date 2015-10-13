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
#include KWSYS_HEADER(Directory.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(Encoding.hxx)

#include KWSYS_HEADER(stl/string)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Directory.hxx.in"
# include "Configure.hxx.in"
# include "Encoding.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_stl_string.hxx.in"
# include "kwsys_stl_vector.hxx.in"
#endif

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
class DirectoryInternals
{
public:
  // Array of Files
  kwsys_stl::vector<kwsys_stl::string> Files;

  // Path to Open'ed directory
  kwsys_stl::string Path;
};

//----------------------------------------------------------------------------
Directory::Directory()
{
  this->Internal = new DirectoryInternals;
}

//----------------------------------------------------------------------------
Directory::~Directory()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
unsigned long Directory::GetNumberOfFiles() const
{
  return static_cast<unsigned long>(this->Internal->Files.size());
}

//----------------------------------------------------------------------------
const char* Directory::GetFile(unsigned long dindex) const
{
  if ( dindex >= this->Internal->Files.size() )
    {
    return 0;
    }
  return this->Internal->Files[dindex].c_str();
}

//----------------------------------------------------------------------------
const char* Directory::GetPath() const
{
  return this->Internal->Path.c_str();
}

//----------------------------------------------------------------------------
void Directory::Clear()
{
  this->Internal->Path.resize(0);
  this->Internal->Files.clear();
}

} // namespace KWSYS_NAMESPACE

// First microsoft compilers

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <windows.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace KWSYS_NAMESPACE
{

bool Directory::Load(const kwsys_stl::string& name)
{
  this->Clear();
#if _MSC_VER < 1300
  long srchHandle;
#else
  intptr_t srchHandle;
#endif
  char* buf;
  size_t n = name.size();
  if ( *name.rbegin() == '/' || *name.rbegin() == '\\' )
    {
    buf = new char[n + 1 + 1];
    sprintf(buf, "%s*", name.c_str());
    }
  else
    {
    // Make sure the slashes in the wildcard suffix are consistent with the
    // rest of the path
    buf = new char[n + 2 + 1];
    if ( name.find('\\') != name.npos )
      {
      sprintf(buf, "%s\\*", name.c_str());
      }
    else
      {
      sprintf(buf, "%s/*", name.c_str());
      }
    }
  struct _wfinddata_t data;      // data of current file

  // Now put them into the file array
  srchHandle = _wfindfirst((wchar_t*)Encoding::ToWide(buf).c_str(), &data);
  delete [] buf;

  if ( srchHandle == -1 )
    {
    return 0;
    }

  // Loop through names
  do
    {
    this->Internal->Files.push_back(Encoding::ToNarrow(data.name));
    }
  while ( _wfindnext(srchHandle, &data) != -1 );
  this->Internal->Path = name;
  return _findclose(srchHandle) != -1;
}

unsigned long Directory::GetNumberOfFilesInDirectory(const kwsys_stl::string& name)
{
#if _MSC_VER < 1300
  long srchHandle;
#else
  intptr_t srchHandle;
#endif
  char* buf;
  size_t n = name.size();
  if ( *name.rbegin() == '/' )
    {
    buf = new char[n + 1 + 1];
    sprintf(buf, "%s*", name.c_str());
    }
  else
    {
    buf = new char[n + 2 + 1];
    sprintf(buf, "%s/*", name.c_str());
    }
  struct _wfinddata_t data;      // data of current file

  // Now put them into the file array
  srchHandle = _wfindfirst((wchar_t*)Encoding::ToWide(buf).c_str(), &data);
  delete [] buf;

  if ( srchHandle == -1 )
    {
    return 0;
    }

  // Loop through names
  unsigned long count = 0;
  do
    {
    count++;
    }
  while ( _wfindnext(srchHandle, &data) != -1 );
  _findclose(srchHandle);
  return count;
}

} // namespace KWSYS_NAMESPACE

#else

// Now the POSIX style directory access

#include <sys/types.h>
#include <dirent.h>

// PGI with glibc has trouble with dirent and large file support:
//  http://www.pgroup.com/userforum/viewtopic.php?
//  p=1992&sid=f16167f51964f1a68fe5041b8eb213b6
// Work around the problem by mapping dirent the same way as readdir.
#if defined(__PGI) && defined(__GLIBC__)
# define kwsys_dirent_readdir dirent
# define kwsys_dirent_readdir64 dirent64
# define kwsys_dirent kwsys_dirent_lookup(readdir)
# define kwsys_dirent_lookup(x) kwsys_dirent_lookup_delay(x)
# define kwsys_dirent_lookup_delay(x) kwsys_dirent_##x
#else
# define kwsys_dirent dirent
#endif

namespace KWSYS_NAMESPACE
{

bool Directory::Load(const kwsys_stl::string& name)
{
  this->Clear();
   
  DIR* dir = opendir(name.c_str());

  if (!dir)
    {
    return 0;
    }

  for (kwsys_dirent* d = readdir(dir); d; d = readdir(dir) )
    {
    this->Internal->Files.push_back(d->d_name);
    }
  this->Internal->Path = name;
  closedir(dir);
  return 1;
}

unsigned long Directory::GetNumberOfFilesInDirectory(const kwsys_stl::string& name)
{
  DIR* dir = opendir(name.c_str());

  if (!dir)
    {
    return 0;
    }

  unsigned long count = 0;
  for (kwsys_dirent* d = readdir(dir); d; d = readdir(dir) )
    {
    count++;
    }
  closedir(dir);
  return count;
}

} // namespace KWSYS_NAMESPACE

#endif
