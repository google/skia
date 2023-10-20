/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"  // IWYU pragma: keep
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

DEF_TEST(SkStrikeCache_CachePurge, Reporter) {
    SkStrikeCache cache;

    sk_sp<SkTypeface> typeface =
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());

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
