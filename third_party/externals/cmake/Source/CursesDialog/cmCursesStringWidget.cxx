/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesStringWidget.h"
#include "cmCursesMainForm.h"

inline int ctrl(int z)
{
    return (z&037);
}

cmCursesStringWidget::cmCursesStringWidget(int width, int height,
                                           int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  this->InEdit = false;
  this->Type = cmState::STRING;
  set_field_fore(this->Field,  A_NORMAL);
  set_field_back(this->Field,  A_STANDOUT);
  field_opts_off(this->Field,  O_STATIC);
}

void cmCursesStringWidget::OnTab(cmCursesMainForm*, WINDOW*)
{
  //FORM* form = fm->GetForm();
}

void cmCursesStringWidget::OnReturn(cmCursesMainForm* fm, WINDOW*)
{
  FORM* form = fm->GetForm();
  if (this->InEdit)
    {
    cmCursesForm::LogMessage("String widget leaving edit.");
    this->InEdit = false;
    fm->PrintKeys();
    delete[] this->OriginalString;
    // trick to force forms to update the field buffer
    form_driver(form, REQ_NEXT_FIELD);
    form_driver(form, REQ_PREV_FIELD);
    this->Done = true;
    }
  else
    {
    cmCursesForm::LogMessage("String widget entering edit.");
    this->InEdit = true;
    fm->PrintKeys();
    char* buf = field_buffer(this->Field, 0);
    this->OriginalString = new char[strlen(buf)+1];
    strcpy(this->OriginalString, buf);
    }
}

void cmCursesStringWidget::OnType(int& key, cmCursesMainForm* fm, WINDOW*)
{
  form_driver(fm->GetForm(), key);
}

bool cmCursesStringWidget::HandleInput(int& key, cmCursesMainForm* fm,
                                       WINDOW* w)
{
  int x,y;

  FORM* form = fm->GetForm();
  // 10 == enter
  if (!this->InEdit && ( key != 10 && key != KEY_ENTER ) )
    {
    return false;
    }

  this->OriginalString=0;
  this->Done = false;

  char debugMessage[128];

  // <Enter> is used to change edit mode (like <Esc> in vi).
  while(!this->Done)
    {
    sprintf(debugMessage, "String widget handling input, key: %d", key);
    cmCursesForm::LogMessage(debugMessage);

    fm->PrintKeys();

    getmaxyx(stdscr, y, x);
    // If window too small, handle 'q' only
    if ( x < cmCursesMainForm::MIN_WIDTH  ||
         y < cmCursesMainForm::MIN_HEIGHT )
      {
      // quit
      if ( key == 'q' )
        {
        return false;
        }
      else
        {
        key=getch();
        continue;
        }
      }

    // If resize occured during edit, move out of edit mode
    if (!this->InEdit && ( key != 10 && key != KEY_ENTER ) )
      {
      return false;
      }
    // 10 == enter
    if (key == 10 || key == KEY_ENTER)
      {
      this->OnReturn(fm, w);
      }
    else if ( key == KEY_DOWN || key == ctrl('n') ||
              key == KEY_UP || key == ctrl('p') ||
              key == KEY_NPAGE || key == ctrl('d') ||
              key == KEY_PPAGE || key == ctrl('u'))
      {
      this->InEdit = false;
      delete[] this->OriginalString;
      // trick to force forms to update the field buffer
      form_driver(form, REQ_NEXT_FIELD);
      form_driver(form, REQ_PREV_FIELD);
      return false;
      }
    // esc
    else if (key == 27)
      {
      if (this->InEdit)
        {
        this->InEdit = false;
        fm->PrintKeys();
        this->SetString(this->OriginalString);
        delete[] this->OriginalString;
        touchwin(w);
        wrefresh(w);
        return true;
        }
      }
    else if ( key == 9 )
      {
      this->OnTab(fm, w);
      }
    else if ( key == KEY_LEFT || key == ctrl('b') )
      {
      form_driver(form, REQ_PREV_CHAR);
      }
    else if ( key == KEY_RIGHT || key == ctrl('f') )
      {
      form_driver(form, REQ_NEXT_CHAR);
      }
    else if ( key == ctrl('k') )
      {
      form_driver(form, REQ_CLR_EOL);
      }
    else if ( key == ctrl('a') || key == KEY_HOME )
      {
      form_driver(form, REQ_BEG_FIELD);
      }
    else if ( key == ctrl('e') || key == KEY_END )
      {
      form_driver(form, REQ_END_FIELD);
      }
    else if ( key == 127 ||
              key == KEY_BACKSPACE )
      {
      if ( form->curcol > 0 )
        {
        form_driver(form, REQ_DEL_PREV);
        }
      }
    else if ( key == ctrl('d') ||key == KEY_DC )
      {
      if ( form->curcol >= 0 )
        {
        form_driver(form, REQ_DEL_CHAR);
        }
      }
    else
      {
      this->OnType(key, fm, w);
      }
    if ( !this->Done )
      {
      touchwin(w);
      wrefresh(w);

      key=getch();
      }
    }
  return true;
}

void cmCursesStringWidget::SetString(const std::string& value)
{
  this->SetValue(value);
}

const char* cmCursesStringWidget::GetString()
{
  return this->GetValue();
}

const char* cmCursesStringWidget::GetValue()
{
  return field_buffer(this->Field, 0);
}

bool cmCursesStringWidget::PrintKeys()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  ||
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return false;
    }
  if (this->InEdit)
    {
    char firstLine[512];
    // Clean the toolbar
    for(int i=0; i<512; i++)
      {
      firstLine[i] = ' ';
      }
    firstLine[511] = '\0';
    curses_move(y-4,0);
    printw(firstLine);
    curses_move(y-3,0);
    printw(firstLine);
    curses_move(y-2,0);
    printw(firstLine);
    curses_move(y-1,0);
    printw(firstLine);

    sprintf(firstLine,  "Editing option, press [enter] to leave edit.");
    curses_move(y-3,0);
    printw(firstLine);
    return true;
    }
  else
    {
    return false;
    }
}
