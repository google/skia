/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFileTimeComparison.h"

// Use a hash table to avoid duplicate file time checks from disk.
#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
#include <unordered_map>
#else
# include <cmsys/hash_map.hxx>
#endif
#endif

#include <cmsys/Encoding.hxx>

// Use a platform-specific API to get file times efficiently.
#if !defined(_WIN32) || defined(__CYGWIN__)
#  define cmFileTimeComparison_Type struct stat
#  include <ctype.h>
#  include <sys/stat.h>
#else
#  define cmFileTimeComparison_Type FILETIME
#  include <windows.h>
#endif

//----------------------------------------------------------------------------
class cmFileTimeComparisonInternal
{
public:
  // Internal comparison method.
  inline bool FileTimeCompare(const char* f1, const char* f2, int* result);

  bool FileTimesDiffer(const char* f1, const char* f2);

private:
#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Use a hash table to efficiently map from file name to modification time.
  class HashString
    {
  public:
    size_t operator()(const std::string& s) const
      {
      return h(s.c_str());
      }
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
    std::hash<const char*> h;
#else
    cmsys::hash<const char*> h;
#endif
    };
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
  typedef std::unordered_map<std::string,
#else
  typedef cmsys::hash_map<std::string,
#endif
                          cmFileTimeComparison_Type, HashString> FileStatsMap;
  FileStatsMap Files;
#endif

  // Internal methods to lookup and compare modification times.
  inline bool Stat(const char* fname, cmFileTimeComparison_Type* st);
  inline int Compare(cmFileTimeComparison_Type* st1,
                     cmFileTimeComparison_Type* st2);
  inline bool TimesDiffer(cmFileTimeComparison_Type* st1,
                          cmFileTimeComparison_Type* st2);
};

//----------------------------------------------------------------------------
bool cmFileTimeComparisonInternal::Stat(const char* fname,
                                        cmFileTimeComparison_Type* st)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Use the stored time if available.
  cmFileTimeComparisonInternal::FileStatsMap::iterator fit =
    this->Files.find(fname);
  if ( fit != this->Files.end() )
    {
    *st = fit->second;
    return true;
    }
#endif

#if !defined(_WIN32) || defined(__CYGWIN__)
  // POSIX version.  Use the stat function.
  int res = ::stat(fname, st);
  if ( res != 0 )
    {
    return false;
    }
#else
  // Windows version.  Get the modification time from extended file
  // attributes.
  WIN32_FILE_ATTRIBUTE_DATA fdata;
  if(!GetFileAttributesExW(cmsys::Encoding::ToWide(fname).c_str(),
                           GetFileExInfoStandard, &fdata))
    {
    return false;
    }

  // Copy the file time to the output location.
  *st = fdata.ftLastWriteTime;
#endif

#if defined(CMAKE_BUILD_WITH_CMAKE)
  // Store the time for future use.
  this->Files[fname] = *st;
#endif

  return true;
}

//----------------------------------------------------------------------------
cmFileTimeComparison::cmFileTimeComparison()
{
  this->Internals = new cmFileTimeComparisonInternal;
}

//----------------------------------------------------------------------------
cmFileTimeComparison::~cmFileTimeComparison()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
bool cmFileTimeComparison::FileTimeCompare(const char* f1,
                                           const char* f2, int* result)
{
  return this->Internals->FileTimeCompare(f1, f2, result);
}

//----------------------------------------------------------------------------
bool cmFileTimeComparison::FileTimesDiffer(const char* f1, const char* f2)
{
  return this->Internals->FileTimesDiffer(f1, f2);
}

//----------------------------------------------------------------------------
int cmFileTimeComparisonInternal::Compare(cmFileTimeComparison_Type* s1,
                                          cmFileTimeComparison_Type* s2)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
# if cmsys_STAT_HAS_ST_MTIM
  // Compare using nanosecond resolution.
  if(s1->st_mtim.tv_sec < s2->st_mtim.tv_sec)
    {
    return -1;
    }
  else if(s1->st_mtim.tv_sec > s2->st_mtim.tv_sec)
    {
    return 1;
    }
  else if(s1->st_mtim.tv_nsec < s2->st_mtim.tv_nsec)
    {
    return -1;
    }
  else if(s1->st_mtim.tv_nsec > s2->st_mtim.tv_nsec)
    {
    return 1;
    }
# else
  // Compare using 1 second resolution.
  if(s1->st_mtime < s2->st_mtime)
    {
    return -1;
    }
  else if(s1->st_mtime > s2->st_mtime)
    {
    return 1;
    }
# endif
  // Files have the same time.
  return 0;
#else
  // Compare using system-provided function.
  return (int)CompareFileTime(s1, s2);
#endif
}

//----------------------------------------------------------------------------
bool cmFileTimeComparisonInternal::TimesDiffer(cmFileTimeComparison_Type* s1,
                                               cmFileTimeComparison_Type* s2)
{
#if !defined(_WIN32) || defined(__CYGWIN__)
# if cmsys_STAT_HAS_ST_MTIM
  // Times are integers in units of 1ns.
  long long bil = 1000000000;
  long long t1 = s1->st_mtim.tv_sec * bil + s1->st_mtim.tv_nsec;
  long long t2 = s2->st_mtim.tv_sec * bil + s2->st_mtim.tv_nsec;
  if(t1 < t2)
    {
    return (t2 - t1) >= bil;
    }
  else if(t2 < t1)
    {
    return (t1 - t2) >= bil;
    }
  else
    {
    return false;
    }
# else
  // Times are integers in units of 1s.
  if(s1->st_mtime < s2->st_mtime)
    {
    return (s2->st_mtime - s1->st_mtime) >= 1;
    }
  else if(s1->st_mtime > s2->st_mtime)
    {
    return (s1->st_mtime - s2->st_mtime) >= 1;
    }
  else
    {
    return false;
    }
# endif
#else
  // Times are integers in units of 100ns.
  LARGE_INTEGER t1;
  LARGE_INTEGER t2;
  t1.LowPart = s1->dwLowDateTime;
  t1.HighPart = s1->dwHighDateTime;
  t2.LowPart = s2->dwLowDateTime;
  t2.HighPart = s2->dwHighDateTime;
  if(t1.QuadPart < t2.QuadPart)
    {
    return (t2.QuadPart - t1.QuadPart) >= static_cast<LONGLONG>(10000000);
    }
  else if(t2.QuadPart < t1.QuadPart)
    {
    return (t1.QuadPart - t2.QuadPart) >= static_cast<LONGLONG>(10000000);
    }
  else
    {
    return false;
    }
#endif
}

//----------------------------------------------------------------------------
bool cmFileTimeComparisonInternal::FileTimeCompare(const char* f1,
                                                   const char* f2,
                                                   int* result)
{
  // Get the modification time for each file.
  cmFileTimeComparison_Type s1;
  cmFileTimeComparison_Type s2;
  if(this->Stat(f1, &s1) &&
     this->Stat(f2, &s2))
    {
    // Compare the two modification times.
    *result = this->Compare(&s1, &s2);
    return true;
    }
  else
    {
    // No comparison available.  Default to the same time.
    *result = 0;
    return false;
    }
}

//----------------------------------------------------------------------------
bool cmFileTimeComparisonInternal::FileTimesDiffer(const char* f1,
                                                   const char* f2)
{
  // Get the modification time for each file.
  cmFileTimeComparison_Type s1;
  cmFileTimeComparison_Type s2;
  if(this->Stat(f1, &s1) &&
     this->Stat(f2, &s2))
    {
    // Compare the two modification times.
    return this->TimesDiffer(&s1, &s2);
    }
  else
    {
    // No comparison available.  Default to different times.
    return true;
    }
}
