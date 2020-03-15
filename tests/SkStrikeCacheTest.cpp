/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

DEF_TEST(SkStrikeCache_CachePurge, Reporter) {
    SkStrikeCache cache;

    sk_sp<SkTypeface> typeface =
            ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic());

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    SkPaint defaultPaint;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());

    // Initially empty cache
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);

    {
        sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike(&cache);
    }

    // Stuff in cache.
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() > 0);

    cache.purgeAll();

    // Purged cache.
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);

    // Smallest cache.
    cache.setCacheSizeLimit(0);
    {
        sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike(&cache);
        REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);
    }
    REPORTER_ASSERT(Reporter, cache.getTotalMemoryUsed() == 0);


}
