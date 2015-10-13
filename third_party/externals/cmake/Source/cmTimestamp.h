/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTimestamp_h
#define cmTimestamp_h

#include <string>
#include <time.h>

/** \class cmTimestamp
 * \brief Utility class to generate sting representation of a timestamp
 *
 */
class cmTimestamp
{
public:
  cmTimestamp() {}

  std::string CurrentTime(const std::string& formatString, bool utcFlag);

  std::string FileModificationTime(const char* path,
    const std::string& formatString, bool utcFlag);

private:
  std::string CreateTimestampFromTimeT(time_t timeT,
      std::string formatString, bool utcFlag);

  std::string AddTimestampComponent(char flag, struct tm& timeStruct);
};


#endif
