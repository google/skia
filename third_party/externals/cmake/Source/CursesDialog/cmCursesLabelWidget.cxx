/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesLabelWidget.h"

cmCursesLabelWidget::cmCursesLabelWidget(int width, int height,
                                         int left, int top,
                                         const std::string& name) :
  cmCursesWidget(width, height, left, top)
{
  field_opts_off(this->Field,  O_EDIT);
  field_opts_off(this->Field,  O_ACTIVE);
  field_opts_off(this->Field,  O_STATIC);
  this->SetValue(name);
}

cmCursesLabelWidget::~cmCursesLabelWidget()
{
}

bool cmCursesLabelWidget::HandleInput(int&, cmCursesMainForm*, WINDOW* )
{
  // Static text. No input is handled here.
  return false;
}
