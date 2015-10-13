/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesOptionsWidget.h"
#include "cmCursesMainForm.h"

inline int ctrl(int z)
{
    return (z&037);
}

cmCursesOptionsWidget::cmCursesOptionsWidget(int width, int height,
                                       int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  this->Type = cmState::BOOL; // this is a bit of a hack
  // there is no option type, and string type causes ccmake to cast
  // the widget into a string widget at some point.  BOOL is safe for
  // now.
  set_field_fore(this->Field,  A_NORMAL);
  set_field_back(this->Field,  A_STANDOUT);
  field_opts_off(this->Field,  O_STATIC);
}

bool cmCursesOptionsWidget::HandleInput(int& key, cmCursesMainForm*, WINDOW* w)
{

  // 10 == enter
  if (key == 10 || key == KEY_ENTER)
    {
    this->NextOption();
    touchwin(w);
    wrefresh(w);
    return true;
    }
  else if (key == KEY_LEFT || key == ctrl('b'))
    {
    touchwin(w);
    wrefresh(w);
    this->PreviousOption();
    return true;
    }
  else if (key == KEY_RIGHT || key == ctrl('f'))
    {
    this->NextOption();
    touchwin(w);
    wrefresh(w);
    return true;
    }
  else
    {
    return false;
    }
}

void cmCursesOptionsWidget::AddOption(std::string const & option )
{
  this->Options.push_back(option);
}

void cmCursesOptionsWidget::NextOption()
{
  this->CurrentOption++;
  if(this->CurrentOption > this->Options.size()-1)
    {
    this->CurrentOption = 0;
    }
  this->SetValue(this->Options[this->CurrentOption]);
}
void cmCursesOptionsWidget::PreviousOption()
{
  if(this->CurrentOption == 0)
    {
    this->CurrentOption = this->Options.size()-1;
    }
  else
    {
    this->CurrentOption--;
    }
  this->SetValue(this->Options[this->CurrentOption]);
}

void cmCursesOptionsWidget::SetOption(const std::string& value)
{
  this->CurrentOption = 0; // default to 0 index
  this->SetValue(value);
  int index = 0;
  for(std::vector<std::string>::iterator i = this->Options.begin();
      i != this->Options.end(); ++i)
    {
    if(*i == value)
      {
      this->CurrentOption = index;
      }
    index++;
    }
}
