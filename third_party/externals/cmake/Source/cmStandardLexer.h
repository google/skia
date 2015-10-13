/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmStandardLexer_h
#define cmStandardLexer_h

/* Disable some warnings.  */
#if defined(_MSC_VER)
# pragma warning ( disable : 4127 )
# pragma warning ( disable : 4131 )
# pragma warning ( disable : 4244 )
# pragma warning ( disable : 4251 )
# pragma warning ( disable : 4267 )
# pragma warning ( disable : 4305 )
# pragma warning ( disable : 4309 )
# pragma warning ( disable : 4706 )
# pragma warning ( disable : 4786 )
#endif

/* Define isatty on windows.  */
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h>
# if defined( _MSC_VER )
#  define isatty _isatty
# endif
# define YY_NO_UNISTD_H 1
#endif

/* Make sure malloc and free are available on QNX.  */
#ifdef __QNX__
# include <malloc.h>
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1
#define ECHO

#endif
