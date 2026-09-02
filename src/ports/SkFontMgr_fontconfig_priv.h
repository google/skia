/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_fontconfig_priv_DEFINED
#define SkFontMgr_fontconfig_priv_DEFINED

#include <fontconfig/fontconfig.h>

// Exposing remove_weak for testing
class SkFontMgr_fontconfig_priv {
public:
    static void remove_weak(FcPattern* pattern, const char object[]);
};

#endif  // SkFontMgr_fontconfig_priv_DEFINED
