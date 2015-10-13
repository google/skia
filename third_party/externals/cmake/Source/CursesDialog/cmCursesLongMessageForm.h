/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesLongMessageForm_h
#define cmCursesLongMessageForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesForm.h"
#include "cmCursesStandardIncludes.h"

class cmCursesCacheEntryComposite;

class cmCursesLongMessageForm : public cmCursesForm
{
public:
  cmCursesLongMessageForm(std::vector<std::string> const& messages,
                          const char* title);
  virtual ~cmCursesLongMessageForm();

  // Description:
  // Handle user input.
  virtual void HandleInput();

  // Description:
  // Display form. Use a window of size width x height, starting
  // at top, left.
  virtual void Render(int left, int top, int width, int height);

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  void PrintKeys();

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar();

protected:
  cmCursesLongMessageForm(const cmCursesLongMessageForm& from);
  void operator=(const cmCursesLongMessageForm&);

  std::string Messages;
  std::string Title;

  FIELD* Fields[2];

};

#endif // cmCursesLongMessageForm_h
