
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkTypeface_win_DEFINED
#define SkTypeface_win_DEFINED

#include "SkTypeface.h"

/**
 *  Like the other Typeface create methods, this returns a new reference to the
 *  corresponding typeface for the specified logfont. The caller is responsible
 *  for calling unref() when it is finished.
 */
SK_API SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT&);

/**
 *  Copy the LOGFONT associated with this typeface into the lf parameter. Note
 *  that the lfHeight will need to be set afterwards, since the typeface does
 *  not track this (the paint does).
 *  typeface may be NULL, in which case we return the logfont for the default font.
 */
SK_API void SkLOGFONTFromTypeface(const SkTypeface* typeface, LOGFONT* lf);

/**
  *  Set an optional callback to ensure that the data behind a LOGFONT is loaded.
  *  This will get called if Skia tries to access the data but hits a failure.
  *  Normally this is null, and is only required if the font data needs to be
  *  remotely (re)loaded.
  */
SK_API void SkTypeface_SetEnsureLOGFONTAccessibleProc(void (*)(const LOGFONT&));

#endif
