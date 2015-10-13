/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesStringWidget_h
#define cmCursesStringWidget_h

#include "cmCursesWidget.h"

class cmCursesMainForm;

/** \class cmCursesStringWidget
 * \brief A simple entry widget.
 *
 * cmCursesStringWdiget is a simple text entry widget.
 */

class cmCursesStringWidget : public cmCursesWidget
{
public:
  cmCursesStringWidget(int width, int height, int left, int top);

  /**
   * Handle user input. Called by the container of this widget
   * when this widget has focus. Returns true if the input was
   * handled.
   */
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

  /**
   * Set/Get the string.
   */
  void SetString(const std::string& value);
  const char* GetString();
  virtual const char* GetValue();

  /**
   * Set/Get InEdit flag. Can be used to tell the widget to leave
   * edit mode (in case of a resize for example).
   */
  void SetInEdit(bool inedit) { this->InEdit = inedit; }
  bool GetInEdit() { return this->InEdit; }

  /**
   * This method is called when different keys are pressed. The
   * subclass can have a special implementation handler for this.
   */
  virtual void OnTab(cmCursesMainForm* fm, WINDOW* w);
  virtual void OnReturn(cmCursesMainForm* fm, WINDOW* w);
  virtual void OnType(int& key, cmCursesMainForm* fm, WINDOW* w);

  /**
   * If there are any, print the widget specific commands
   * in the toolbar and return true. Otherwise, return false
   * and the parent widget will print.
   */
  virtual bool PrintKeys();

protected:
  cmCursesStringWidget(const cmCursesStringWidget& from);
  void operator=(const cmCursesStringWidget&);

  // true if the widget is in edit mode
  bool InEdit;
  char* OriginalString;
  bool Done;
};

#endif // cmCursesStringWidget_h
