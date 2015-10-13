/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesCacheEntryComposite.h"
#include "cmCursesOptionsWidget.h"
#include "cmCursesStringWidget.h"
#include "cmCursesLabelWidget.h"
#include "cmCursesBoolWidget.h"
#include "cmCursesPathWidget.h"
#include "cmCursesFilePathWidget.h"
#include "cmCursesDummyWidget.h"
#include "../cmSystemTools.h"
#include "../cmake.h"
#include "../cmState.h"

#include <assert.h>

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
                                                        const std::string& key,
                                                        int labelwidth,
                                                        int entrywidth) :
  Key(key), LabelWidth(labelwidth), EntryWidth(entrywidth)
{
  this->Label = new cmCursesLabelWidget(this->LabelWidth, 1, 1, 1, key);
  this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
  this->Entry = 0;
  this->Entry = new cmCursesStringWidget(this->EntryWidth, 1, 1, 1);
}

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const std::string& key, cmake *cm, bool isNew,
  int labelwidth, int entrywidth)
  : Key(key), LabelWidth(labelwidth), EntryWidth(entrywidth)
{
  this->Label = new cmCursesLabelWidget(this->LabelWidth, 1, 1, 1, key);
  if (isNew)
    {
    this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, "*");
    }
  else
    {
    this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
    }

  this->Entry = 0;
  const char* value = cm->GetState()->GetCacheEntryValue(key);
  assert(value);
  switch (cm->GetState()->GetCacheEntryType(key))
    {
    case cmState::BOOL:
      this->Entry = new cmCursesBoolWidget(this->EntryWidth, 1, 1, 1);
      if (cmSystemTools::IsOn(value))
        {
        static_cast<cmCursesBoolWidget*>(this->Entry)->SetValueAsBool(true);
        }
      else
        {
        static_cast<cmCursesBoolWidget*>(this->Entry)->SetValueAsBool(false);
        }
      break;
    case cmState::PATH:
      this->Entry = new cmCursesPathWidget(this->EntryWidth, 1, 1, 1);
      static_cast<cmCursesPathWidget*>(this->Entry)->SetString(value);
      break;
    case cmState::FILEPATH:
      this->Entry = new cmCursesFilePathWidget(this->EntryWidth, 1, 1, 1);
      static_cast<cmCursesFilePathWidget*>(this->Entry)->SetString(value);
      break;
    case cmState::STRING:
      {
      const char* stringsProp = cm->GetState()
                                  ->GetCacheEntryProperty(key, "STRINGS");
      if(stringsProp)
        {
        cmCursesOptionsWidget* ow =
          new cmCursesOptionsWidget(this->EntryWidth, 1, 1, 1);
        this->Entry = ow;
        std::vector<std::string> options;
        cmSystemTools::ExpandListArgument(stringsProp, options);
        for(std::vector<std::string>::iterator
              si = options.begin(); si != options.end(); ++si)
          {
          ow->AddOption(*si);
          }
        ow->SetOption(value);
        }
      else
        {
        this->Entry = new cmCursesStringWidget(this->EntryWidth, 1, 1, 1);
        static_cast<cmCursesStringWidget*>(this->Entry)->SetString(value);
        }
      break;
      }
    case cmState::UNINITIALIZED:
      cmSystemTools::Error("Found an undefined variable: ",
                           key.c_str());
      break;
    default:
      // TODO : put warning message here
      break;
    }

}

cmCursesCacheEntryComposite::~cmCursesCacheEntryComposite()
{
  delete this->Label;
  delete this->IsNewLabel;
  delete this->Entry;
}

const char* cmCursesCacheEntryComposite::GetValue()
{
  if (this->Label)
    {
    return this->Label->GetValue();
    }
  else
    {
    return 0;
    }
}
