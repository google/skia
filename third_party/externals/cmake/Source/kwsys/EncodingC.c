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
#include KWSYS_HEADER(Encoding.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Encoding.h.in"
#endif

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

size_t kwsysEncoding_mbstowcs(wchar_t* dest, const char* str, size_t n)
{
  if(str == 0)
    {
    return (size_t)-1;
    }
#ifdef _WIN32
  return MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0,
                             str, -1, dest, (int)n) - 1;
#else
  return mbstowcs(dest, str, n);
#endif
}

wchar_t* kwsysEncoding_DupToWide(const char* str)
{
  wchar_t* ret = NULL;
  size_t length = kwsysEncoding_mbstowcs(NULL, str, 0) + 1;
  if(length > 0)
    {
    ret = (wchar_t*)malloc((length)*sizeof(wchar_t));
    ret[0] = 0;
    kwsysEncoding_mbstowcs(ret, str, length);
    }
  return ret;
}

size_t kwsysEncoding_wcstombs(char* dest, const wchar_t* str, size_t n)
{
  if(str == 0)
    {
    return (size_t)-1;
    }
#ifdef _WIN32
  return WideCharToMultiByte(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str, -1,
                             dest, (int)n, NULL, NULL) - 1;
#else
  return wcstombs(dest, str, n);
#endif
}

char* kwsysEncoding_DupToNarrow(const wchar_t* str)
{
  char* ret = NULL;
  size_t length = kwsysEncoding_wcstombs(0, str, 0) + 1;
  if(length > 0)
    {
    ret = (char*)malloc(length);
    ret[0] = 0;
    kwsysEncoding_wcstombs(ret, str, length);
    }
  return ret;
}
