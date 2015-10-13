/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesForm.h"

cmsys::ofstream cmCursesForm::DebugFile;
bool cmCursesForm::Debug = false;

cmCursesForm::cmCursesForm()
{
  this->Form = 0;
}

cmCursesForm::~cmCursesForm()
{
  if (this->Form)
    {
    unpost_form(this->Form);
    free_form(this->Form);
    this->Form = 0;
    }
}

void cmCursesForm::DebugStart()
{
  cmCursesForm::Debug = true;
  cmCursesForm::DebugFile.open("ccmakelog.txt");
}

void cmCursesForm::DebugEnd()
{
  if (!cmCursesForm::Debug)
    {
    return;
    }

  cmCursesForm::Debug = false;
  cmCursesForm::DebugFile.close();
}

void cmCursesForm::LogMessage(const char* msg)
{
  if (!cmCursesForm::Debug)
    {
    return;
    }

  cmCursesForm::DebugFile << msg << std::endl;
}
