/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesLabelWidget_h
#define cmCursesLabelWidget_h

#include "cmCursesWidget.h"
#include "cmCursesStandardIncludes.h"

class cmCursesMainForm;

class cmCursesLabelWidget : public cmCursesWidget
{
public:
  cmCursesLabelWidget(int width, int height, int left, int top,
                      const std::string& name);
  virtual ~cmCursesLabelWidget();

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

protected:
  cmCursesLabelWidget(const cmCursesLabelWidget& from);
  void operator=(const cmCursesLabelWidget&);
};

#endif // cmCursesLabelWidget_h
