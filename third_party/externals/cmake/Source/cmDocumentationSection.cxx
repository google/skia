/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationSection.h"


//----------------------------------------------------------------------------
void cmDocumentationSection::Append(const char *data[][2])
{
  int i = 0;
  while(data[i][1])
    {
    this->Entries.push_back(cmDocumentationEntry(data[i][0],
                                                 data[i][1]));
    data += 1;
    }
}

//----------------------------------------------------------------------------
void cmDocumentationSection::Prepend(const char *data[][2])
{
  std::vector<cmDocumentationEntry> tmp;
  int i = 0;
  while(data[i][1])
    {
    tmp.push_back(cmDocumentationEntry(data[i][0],
                                       data[i][1]));
    data += 1;
    }
  this->Entries.insert(this->Entries.begin(),tmp.begin(),tmp.end());
}

//----------------------------------------------------------------------------
void cmDocumentationSection::Append(const char *n, const char *b)
{
  this->Entries.push_back(cmDocumentationEntry(n,b));
}
