/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_android_ndk_DEFINED
#define SkFontMgr_android_ndk_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include <memory>

class SkFontMgr;
class SkFontScanner;

/** Create a font manager for Android NDK. May return nullptr if unavailable (API < 29).

    The Android NDK Font API only works correctly in an Android Application process.
    If running in a bare executable (for testing or the like) it will use the legacy fonts.xml data.
 */
SK_API sk_sp<SkFontMgr> SkFontMgr_New_AndroidNDK(bool cacheFontFiles,
                                                 std::unique_ptr<SkFontScanner> scanner);

#endif // SkFontMgr_android_ndk_DEFINED
