/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Terminal.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Terminal.h.in"
#endif

int testTerminal(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  kwsysTerminal_cfprintf(kwsysTerminal_Color_ForegroundYellow |
                         kwsysTerminal_Color_BackgroundBlue |
                         kwsysTerminal_Color_AssumeTTY,
                         stdout, "Hello %s!", "World");
  fprintf(stdout, "\n");
  return 0;
}
