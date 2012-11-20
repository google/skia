/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTUtils_DEFINED
#define SkOTUtils_DEFINED

#include "SkOTTableTypes.h"
class SkData;
class SkStream;

struct SkOTUtils {
    /**
      *  Calculates the OpenType checksum for data.
      */
    static uint32_t CalcTableChecksum(SK_OT_ULONG *data, size_t length);

    /**
      *  Renames an sfnt font. On failure (invalid data or not an sfnt font)
      *  returns NULL.
      *
      *  Essentially, this removes any existing 'name' table and replaces it
      *  with a new one in which FontFamilyName, FontSubfamilyName,
      *  UniqueFontIdentifier, FullFontName, and PostscriptName are fontName.
      *
      *  The new 'name' table records will be written with the Windows,
      *  UnicodeBMPUCS2, and English_UnitedStates settings.
      *
      *  fontName and fontNameLen must be specified in terms of ASCII chars.
      */
    static SkData* RenameFont(SkStream* fontData, const char* fontName, int fontNameLen);
};

#endif
