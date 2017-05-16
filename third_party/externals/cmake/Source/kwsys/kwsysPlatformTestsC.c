/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/*
  Macros to define main() in a cross-platform way.

  Usage:

    int KWSYS_PLATFORM_TEST_C_MAIN()
    {
      return 0;
    }

    int KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv)
    {
      (void)argc; (void)argv;
      return 0;
    }
*/
#if defined(__CLASSIC_C__)
# define KWSYS_PLATFORM_TEST_C_MAIN() \
  main()
# define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv) \
  main(argc,argv) int argc; char* argv[];
#else
# define KWSYS_PLATFORM_TEST_C_MAIN() \
  main(void)
# define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv) \
  main(int argc, char* argv[])
#endif

/*--------------------------------------------------------------------------*/
#ifdef TEST_KWSYS_C_HAS_PTRDIFF_T
#include <stddef.h>
int f(ptrdiff_t n) { return n > 0; }
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  char* p = 0;
  ptrdiff_t d = p - p;
  (void)d;
  return f(p - p);
}
#endif

/*--------------------------------------------------------------------------*/
#ifdef TEST_KWSYS_C_HAS_SSIZE_T
#include <unistd.h>
int f(ssize_t n) { return (int)n; }
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  ssize_t n = 0;
  return f(n);
}
#endif

/*--------------------------------------------------------------------------*/
#ifdef TEST_KWSYS_C_TYPE_MACROS
char* info_macros =
#if defined(__SIZEOF_SHORT__)
"INFO:macro[__SIZEOF_SHORT__]\n"
#endif
#if defined(__SIZEOF_INT__)
"INFO:macro[__SIZEOF_INT__]\n"
#endif
#if defined(__SIZEOF_LONG__)
"INFO:macro[__SIZEOF_LONG__]\n"
#endif
#if defined(__SIZEOF_LONG_LONG__)
"INFO:macro[__SIZEOF_LONG_LONG__]\n"
#endif
#if defined(__SHORT_MAX__)
"INFO:macro[__SHORT_MAX__]\n"
#endif
#if defined(__INT_MAX__)
"INFO:macro[__INT_MAX__]\n"
#endif
#if defined(__LONG_MAX__)
"INFO:macro[__LONG_MAX__]\n"
#endif
#if defined(__LONG_LONG_MAX__)
"INFO:macro[__LONG_LONG_MAX__]\n"
#endif
  "";

int KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv)
{
  int require = 0;
  require += info_macros[argc];
  (void)argv;
  return require;
}
#endif
