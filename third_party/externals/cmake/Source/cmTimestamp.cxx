/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTimestamp.h"

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

//----------------------------------------------------------------------------
std::string cmTimestamp::CurrentTime(
  const std::string& formatString, bool utcFlag)
{
  time_t currentTimeT = time(0);
  if(currentTimeT == time_t(-1))
    {
    return std::string();
    }

  return CreateTimestampFromTimeT(currentTimeT, formatString, utcFlag);
}

//----------------------------------------------------------------------------
std::string cmTimestamp::FileModificationTime(const char* path,
  const std::string& formatString, bool utcFlag)
{
  if(!cmsys::SystemTools::FileExists(path))
    {
    return std::string();
    }

  time_t mtime = cmsys::SystemTools::ModifiedTime(path);
  return CreateTimestampFromTimeT(mtime, formatString, utcFlag);
}

//----------------------------------------------------------------------------
std::string cmTimestamp::CreateTimestampFromTimeT(time_t timeT,
    std::string formatString, bool utcFlag)
{
  if(formatString.empty())
    {
    formatString = "%Y-%m-%dT%H:%M:%S";
    if(utcFlag)
      {
      formatString += "Z";
      }
    }

  struct tm timeStruct;
  memset(&timeStruct, 0, sizeof(timeStruct));

  struct tm* ptr = (struct tm*) 0;
  if(utcFlag)
    {
    ptr = gmtime(&timeT);
    }
  else
    {
    ptr = localtime(&timeT);
    }

  if(ptr == 0)
    {
    return std::string();
    }

  timeStruct = *ptr;

  std::string result;
  for(std::string::size_type i = 0; i < formatString.size(); ++i)
    {
    char c1 = formatString[i];
    char c2 = (i+1 < formatString.size()) ?
      formatString[i+1] : static_cast<char>(0);

    if(c1 == '%' && c2 != 0)
      {
      result += AddTimestampComponent(c2, timeStruct);
      ++i;
      }
    else
      {
      result += c1;
      }
    }

  return result;
}

//----------------------------------------------------------------------------
std::string cmTimestamp::AddTimestampComponent(
  char flag, struct tm& timeStruct)
{
  std::string formatString = "%";
  formatString += flag;

  switch(flag)
    {
    case 'd':
    case 'H':
    case 'I':
    case 'j':
    case 'm':
    case 'M':
    case 'S':
    case 'U':
    case 'w':
    case 'y':
    case 'Y':
      break;
    default:
      {
      return formatString;
      }
    }

  char buffer[16];

  size_t size = strftime(buffer, sizeof(buffer),
    formatString.c_str(), &timeStruct);

  return std::string(buffer, size);
}
