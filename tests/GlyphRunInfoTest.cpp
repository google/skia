/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRunInfo.h"

#include "Test.h"

DEF_TEST(GlyphRunInfo, reporter) {
    SkGlyphID glyphs[] = {100, 3, 240, 3, 234, 111, 3, 4, 10, 11};
    uint16_t count = SK_ARRAY_COUNT(glyphs);

    SkGlyphRunInfo::Make(count, glyphs);
}