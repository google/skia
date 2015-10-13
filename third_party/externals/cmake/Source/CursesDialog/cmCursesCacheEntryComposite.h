/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesCacheEntryComposite_h
#define cmCursesCacheEntryComposite_h

#include "cmCursesLabelWidget.h"

class cmCursesCacheEntryComposite
{
public:
  cmCursesCacheEntryComposite(const std::string& key, int labelwidth,
                              int entrywidth);
  cmCursesCacheEntryComposite(const std::string& key,
                              cmake *cm,
                              bool isNew, int labelwidth, int entrywidth);
  ~cmCursesCacheEntryComposite();
  const char* GetValue();

  friend class cmCursesMainForm;

protected:
  cmCursesCacheEntryComposite(const cmCursesCacheEntryComposite& from);
  void operator=(const cmCursesCacheEntryComposite&);

  cmCursesLabelWidget* Label;
  cmCursesLabelWidget* IsNewLabel;
  cmCursesWidget* Entry;
  std::string Key;
  int LabelWidth;
  int EntryWidth;
};

#endif // cmCursesCacheEntryComposite_h
