/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDWriteNTDDI_VERSION_DEFINED
#define SkDWriteNTDDI_VERSION_DEFINED

// More strictly, this header should be the first thing in a translation unit,
// since it is effectively negating build flags.
#if defined(_WINDOWS_) || defined(DWRITE_3_H_INCLUDED)
#error Must include SkDWriteNTDDI_VERSION.h before any Windows or DWrite headers.
#endif

// If the build defines NTDDI_VERSION, pretend it didn't.
// This also requires resetting _WIN32_WINNT and WINVER.
// dwrite_3.h guards enum, macro, and interface declarations behind NTDDI_VERSION,
// but it is not clear this is correct since these are all immutable.
#if defined(NTDDI_VERSION)
#  undef NTDDI_VERSION
#  if defined(_WIN32_WINNT)
#    undef _WIN32_WINNT
#  endif
#  if defined(WINVER)
#    undef WINVER
#  endif
#endif

#endif
