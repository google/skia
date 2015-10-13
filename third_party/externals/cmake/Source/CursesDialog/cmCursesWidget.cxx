/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesWidget.h"

cmCursesWidget::cmCursesWidget(int width, int height, int left, int top)
{
  this->Field = new_field(height, width, top, left, 0, 0);
  set_field_userptr(this->Field, reinterpret_cast<char*>(this));
  field_opts_off(this->Field,  O_AUTOSKIP);
  this->Page = 0;
}

cmCursesWidget::~cmCursesWidget()
{
  if (this->Field)
    {
    free_field(this->Field);
    this->Field = 0;
    }
}

void cmCursesWidget::Move(int x, int y, bool isNewPage)
{
  if (!this->Field)
    {
    return;
    }

  move_field(this->Field, y, x);
  if (isNewPage)
    {
    set_new_page(this->Field, TRUE);
    }
  else
    {
    set_new_page(this->Field, FALSE);
    }
}

void cmCursesWidget::SetValue(const std::string& value)
{
  this->Value = value;
  set_field_buffer(this->Field, 0, value.c_str());
}

const char* cmCursesWidget::GetValue()
{
  return this->Value.c_str();
}
