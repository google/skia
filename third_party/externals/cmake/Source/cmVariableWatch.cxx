/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVariableWatch.h"

#include "cmAlgorithms.h"

static const char* const cmVariableWatchAccessStrings[] =
{
    "READ_ACCESS",
    "UNKNOWN_READ_ACCESS",
    "UNKNOWN_DEFINED_ACCESS",
    "MODIFIED_ACCESS",
    "REMOVED_ACCESS",
    "NO_ACCESS"
};

const char* cmVariableWatch::GetAccessAsString(int access_type)
{
  if ( access_type < 0 || access_type >= cmVariableWatch::NO_ACCESS )
    {
    return "NO_ACCESS";
    }
  return cmVariableWatchAccessStrings[access_type];
}

cmVariableWatch::cmVariableWatch()
{
}

template<typename C>
void deleteAllSecond(typename C::value_type it)
{
  cmDeleteAll(it.second);
}

cmVariableWatch::~cmVariableWatch()
{
  std::for_each(this->WatchMap.begin(), this->WatchMap.end(),
                deleteAllSecond<cmVariableWatch::StringToVectorOfPairs>);
}

bool cmVariableWatch::AddWatch(const std::string& variable,
                               WatchMethod method, void* client_data /*=0*/,
                               DeleteData delete_data /*=0*/)
{
  cmVariableWatch::Pair* p = new cmVariableWatch::Pair;
  p->Method = method;
  p->ClientData = client_data;
  p->DeleteDataCall = delete_data;
  cmVariableWatch::VectorOfPairs* vp = &this->WatchMap[variable];
  cmVariableWatch::VectorOfPairs::size_type cc;
  for ( cc = 0; cc < vp->size(); cc ++ )
    {
    cmVariableWatch::Pair* pair = (*vp)[cc];
    if ( pair->Method == method &&
         client_data && client_data == pair->ClientData)
      {
      // Callback already exists
      return false;
      }
    }
  vp->push_back(p);
  return true;
}

void cmVariableWatch::RemoveWatch(const std::string& variable,
                                  WatchMethod method,
                                  void* client_data /*=0*/)
{
  if ( !this->WatchMap.count(variable) )
    {
    return;
    }
  cmVariableWatch::VectorOfPairs* vp = &this->WatchMap[variable];
  cmVariableWatch::VectorOfPairs::iterator it;
  for ( it = vp->begin(); it != vp->end(); ++it )
    {
    if ( (*it)->Method == method &&
         // If client_data is NULL, we want to disconnect all watches against
         // the given method; otherwise match ClientData as well.
         (!client_data || (client_data == (*it)->ClientData)))
      {
      delete *it;
      vp->erase(it);
      return;
      }
    }
}

void  cmVariableWatch::VariableAccessed(const std::string& variable,
                                        int access_type,
                                        const char* newValue,
                                        const cmMakefile* mf) const
{
  cmVariableWatch::StringToVectorOfPairs::const_iterator mit =
    this->WatchMap.find(variable);
  if ( mit  != this->WatchMap.end() )
    {
    const cmVariableWatch::VectorOfPairs* vp = &mit->second;
    cmVariableWatch::VectorOfPairs::const_iterator it;
    for ( it = vp->begin(); it != vp->end(); it ++ )
      {
      (*it)->Method(variable, access_type, (*it)->ClientData,
        newValue, mf);
      }
    }
}
