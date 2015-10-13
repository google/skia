/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackComponentGroup.h"
#include "cmSystemTools.h"
#include <vector>
#include <string>

//----------------------------------------------------------------------
unsigned long cmCPackComponent::GetInstalledSize(
    const std::string& installDir) const
{
  if (this->TotalSize != 0)
    {
    return this->TotalSize;
    }

  std::vector<std::string>::const_iterator fileIt;
  for (fileIt = this->Files.begin(); fileIt != this->Files.end(); ++fileIt)
    {
    std::string path = installDir;
    path += '/';
    path += *fileIt;
    this->TotalSize += cmSystemTools::FileLength(path);
    }

  return this->TotalSize;
}

//----------------------------------------------------------------------
unsigned long
cmCPackComponent::GetInstalledSizeInKbytes(const std::string& installDir) const
{
  unsigned long result = (GetInstalledSize(installDir) + 512) / 1024;
  return result? result : 1;
}
