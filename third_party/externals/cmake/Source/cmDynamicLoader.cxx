/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDynamicLoader.h"

class cmDynamicLoaderCache
{
public:
  ~cmDynamicLoaderCache();
  void CacheFile(const char* path,
    const cmsys::DynamicLoader::LibraryHandle&);
  bool GetCacheFile(const char* path, cmsys::DynamicLoader::LibraryHandle&);
  bool FlushCache(const char* path);
  void FlushCache();
  static cmDynamicLoaderCache* GetInstance();

private:
  std::map<std::string, cmsys::DynamicLoader::LibraryHandle> CacheMap;
  static cmDynamicLoaderCache* Instance;
};

cmDynamicLoaderCache* cmDynamicLoaderCache::Instance = 0;

cmDynamicLoaderCache::~cmDynamicLoaderCache()
{
}

void cmDynamicLoaderCache::CacheFile(const char* path,
  const cmsys::DynamicLoader::LibraryHandle& p)
{
  cmsys::DynamicLoader::LibraryHandle h;
  if ( this->GetCacheFile(path, h) )
    {
    this->FlushCache(path);
    }
  this->CacheMap[path] = p;
}

bool cmDynamicLoaderCache::GetCacheFile(const char* path,
  cmsys::DynamicLoader::LibraryHandle& p)
{
  std::map<std::string, cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.find(path);
  if ( it != this->CacheMap.end() )
    {
    p = it->second;
    return true;
    }
  return false;
}

bool cmDynamicLoaderCache::FlushCache(const char* path)
{
  std::map<std::string, cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.find(path);
  bool ret = false;
  if ( it != this->CacheMap.end() )
    {
    cmsys::DynamicLoader::CloseLibrary(it->second);
    this->CacheMap.erase(it);
    ret = true;
    }
  return ret;
}

void cmDynamicLoaderCache::FlushCache()
{
  for ( std::map<std::string,
    cmsys::DynamicLoader::LibraryHandle>::iterator it
    = this->CacheMap.begin();
        it != this->CacheMap.end(); it++ )
    {
    cmsys::DynamicLoader::CloseLibrary(it->second);
    }
  delete cmDynamicLoaderCache::Instance;
  cmDynamicLoaderCache::Instance = 0;
}

cmDynamicLoaderCache* cmDynamicLoaderCache::GetInstance()
{
  if ( !cmDynamicLoaderCache::Instance )
    {
    cmDynamicLoaderCache::Instance = new cmDynamicLoaderCache;
    }
  return cmDynamicLoaderCache::Instance;
}

cmsys::DynamicLoader::LibraryHandle cmDynamicLoader::OpenLibrary(
  const char* libname )
{
  cmsys::DynamicLoader::LibraryHandle lh;
  if ( cmDynamicLoaderCache::GetInstance()->GetCacheFile(libname, lh) )
    {
    return lh;
    }
  lh = cmsys::DynamicLoader::OpenLibrary(libname);
  cmDynamicLoaderCache::GetInstance()->CacheFile(libname, lh);
  return lh;
}

void cmDynamicLoader::FlushCache()
{
  cmDynamicLoaderCache::GetInstance()->FlushCache();
}
