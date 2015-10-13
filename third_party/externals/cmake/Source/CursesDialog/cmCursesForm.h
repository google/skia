/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesForm_h
#define cmCursesForm_h

#include "../cmStandardIncludes.h"
#include "cmCursesStandardIncludes.h"
#include <cmsys/FStream.hxx>

class cmCursesForm
{
public:
  cmCursesForm();
  virtual ~cmCursesForm();

  // Description:
  // Handle user input.
  virtual void HandleInput() = 0;

  // Description:
  // Display form.
  virtual void Render(int left, int top, int width, int height) = 0;

  // Description:
  // This method should normally  called only by the form.
  // The only exception is during a resize.
  virtual void UpdateStatusBar() = 0;

  // Description:
  // During a CMake run, an error handle should add errors
  // to be displayed afterwards.
  virtual void AddError(const char*, const char*) {}

  // Description:
  // Turn debugging on. This will create ccmakelog.txt.
  static void DebugStart();

  // Description:
  // Turn debugging off. This will close ccmakelog.txt.
  static void DebugEnd();

  // Description:
  // Write a debugging message.
  static void LogMessage(const char* msg);

  // Description:
  // Return the FORM. Should be only used by low-level methods.
  FORM* GetForm()
    {
      return this->Form;
    }

  static cmCursesForm* CurrentForm;


protected:

  static cmsys::ofstream DebugFile;
  static bool Debug;

  cmCursesForm(const cmCursesForm& form);
  void operator=(const cmCursesForm&);

  FORM* Form;
};

#endif // cmCursesForm_h
