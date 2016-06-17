/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_custom_DEFINED
#define SkFontMgr_custom_DEFINED

#include "SkTypes.h"

class SkFontMgr;

/** Create a custom font manager which scans a given directory for font files. */
SK_API SkFontMgr* SkFontMgr_New_Custom_Directory(const char* dir);

/** Create a custom font manager that contains no built-in fonts. */
SK_API SkFontMgr* SkFontMgr_New_Custom_Empty();

#endif // SkFontMgr_custom_DEFINED
