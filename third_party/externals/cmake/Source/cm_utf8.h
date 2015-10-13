/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cm_utf8_h
#define cm_utf8_h

#ifdef __cplusplus
extern "C" {
#endif

/** Decode one UTF-8 character from the input byte range.  On success,
    stores the unicode character number in *pc and returns the first
    position not extracted.  On failure, returns 0.  */
const char* cm_utf8_decode_character(const char* first, const char* last,
                                     unsigned int* pc);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
