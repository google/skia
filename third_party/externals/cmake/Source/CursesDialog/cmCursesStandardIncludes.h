/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCursesStandardIncludes_h
#define cmCursesStandardIncludes_h
#if defined(__sun__) && defined(__GNUC__)
 #define _MSE_INT_H
#endif

#if defined(__hpux)
# define _BOOL_DEFINED
# include <sys/time.h>
#endif

#include <form.h>

// on some machines move erase and clear conflict with stl
// so remove them from the namespace
inline void curses_move(unsigned int x, unsigned int y)
{
  move(x,y);
}

inline void curses_clear()
{
  erase();
  clearok(stdscr, TRUE);
}

#undef move
#undef erase
#undef clear


#endif // cmCursesStandardIncludes_h
