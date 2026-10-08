/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/RasterContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/cpu/Recorder.h"
#include "src/core/SkContextPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"  // IWYU pragma: keep
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <cstddef>
#include <memory>

namespace {

SkFont make_test_font(float size = 16.0f) {
    sk_sp<SkTypeface> typeface =
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());
    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);
    font.setSize(size);
    return font;
}

SkStrikeSpec make_test_strike_spec(const SkFont& font) {
    SkPaint defaultPaint;
    return SkStrikeSpec::MakeMask(font,
                                  defaultPaint,
                                  SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                  SkScalerContextFlags::kNone,
                                  SkMatrix::I());
}

}  // namespace

DEF_TEST(SkStrikeCache_CachePurge, Reporter) {
    SkStrikeCache cache;

    SkFont font = make_test_font();
    SkStrikeSpec strikeSpec = make_test_strike_spec(font);

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

// TODO(alexisdavidc): Re-enable once SkContext local strike cache support is implemented.
#if 0
DEF_TEST(SkStrikeCache_ContextOptionsAndLimits, reporter) {
    // 1. Default options
    SkContextOptions defaultOptions;
    std::unique_ptr<SkContext> defaultCtx = SkContexts::MakeRaster(defaultOptions);
    REPORTER_ASSERT(reporter, defaultCtx != nullptr);
    REPORTER_ASSERT(reporter, defaultCtx->fontCacheLimit() == SK_DEFAULT_FONT_CACHE_LIMIT);
    REPORTER_ASSERT(reporter,
                    defaultCtx->fontCacheCountLimit() == SK_DEFAULT_FONT_CACHE_COUNT_LIMIT);
    REPORTER_ASSERT(reporter, defaultCtx->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, defaultCtx->fontCacheCountUsed() == 0);

    // 2. Custom options
    SkContextOptions customOptions;
    customOptions.fFontCacheLimit = 4 * 1024 * 1024;
    customOptions.fFontCacheCountLimit = 512;

    std::unique_ptr<SkContext> customCtx = SkContexts::MakeRaster(customOptions);
    REPORTER_ASSERT(reporter, customCtx != nullptr);
    REPORTER_ASSERT(reporter, customCtx->fontCacheLimit() == customOptions.fFontCacheLimit);
    REPORTER_ASSERT(reporter,
                    customCtx->fontCacheCountLimit() == customOptions.fFontCacheCountLimit);
    REPORTER_ASSERT(reporter, customCtx->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, customCtx->fontCacheCountUsed() == 0);

    // 3. Dynamic limit updates return previous value and apply new limit
    size_t prevByteLimit = customCtx->setFontCacheLimit(1024 * 1024);
    REPORTER_ASSERT(reporter, prevByteLimit == customOptions.fFontCacheLimit);
    REPORTER_ASSERT(reporter, customCtx->fontCacheLimit() == 1024 * 1024);

    int prevCountLimit = customCtx->setFontCacheCountLimit(128);
    REPORTER_ASSERT(reporter, prevCountLimit == customOptions.fFontCacheCountLimit);
    REPORTER_ASSERT(reporter, customCtx->fontCacheCountLimit() == 128);
}

DEF_TEST(SkStrikeCache_ContextPurgeAndEviction, reporter) {
    std::unique_ptr<SkContext> context = SkContexts::MakeRaster({});
    REPORTER_ASSERT(reporter, context != nullptr);

    SkFont font16 = make_test_font(16.0f);
    SkFont font32 = make_test_font(32.0f);
    SkStrikeSpec spec16 = make_test_strike_spec(font16);
    SkStrikeSpec spec32 = make_test_strike_spec(font32);

    REPORTER_ASSERT(reporter, context->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 0);

    // Populate the context's strike cache.
    {
        sk_sp<SkStrike> strike = spec16.findOrCreateStrike(context->priv().fontCache());
        REPORTER_ASSERT(reporter, strike != nullptr);
    }
    REPORTER_ASSERT(reporter, context->fontCacheUsed() > 0);
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);

    // Re-querying the same strike spec hits the existing cached entry.
    size_t usedAfterFirstStrike = context->fontCacheUsed();
    {
        sk_sp<SkStrike> strike = spec16.findOrCreateStrike(context->priv().fontCache());
        REPORTER_ASSERT(reporter, strike != nullptr);
    }
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, context->fontCacheUsed() == usedAfterFirstStrike);

    // Purging the font cache clears entries without altering the configured limits.
    size_t limitBeforePurge = context->fontCacheLimit();
    context->purgeFontCache();
    REPORTER_ASSERT(reporter, context->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 0);
    REPORTER_ASSERT(reporter, context->fontCacheLimit() == limitBeforePurge);

    // Setting count limit to 1 evicts the least-recently-used strike when a 2nd strike is added.
    context->setFontCacheCountLimit(1);
    {
        sk_sp<SkStrike> strike16 = spec16.findOrCreateStrike(context->priv().fontCache());
        REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);

        sk_sp<SkStrike> strike32 = spec32.findOrCreateStrike(context->priv().fontCache());
        REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);
    }

    // Setting byte limit to 0 immediately evicts cached strikes.
    context->setFontCacheLimit(0);
    REPORTER_ASSERT(reporter, context->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 0);
    {
        sk_sp<SkStrike> strike = spec16.findOrCreateStrike(context->priv().fontCache());
        REPORTER_ASSERT(reporter, context->fontCacheUsed() == 0);
        REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 0);
    }
}

DEF_TEST(SkStrikeCache_ContextPurgePinned, reporter) {
    std::unique_ptr<SkContext> context = SkContexts::MakeRaster({});
    REPORTER_ASSERT(reporter, context != nullptr);

    struct TogglePinner : public SkStrikePinner {
        bool fCanDelete = false;
        bool canDelete() override { return fCanDelete; }
    };

    auto pinner = std::make_unique<TogglePinner>();
    TogglePinner* pinnerPtr = pinner.get();

    SkFont font = make_test_font(20.0f);
    SkStrikeSpec strikeSpec = make_test_strike_spec(font);

    {
        sk_sp<SkStrike> strike = context->priv().fontCache()->createStrike(
                strikeSpec, nullptr, std::move(pinner));
        REPORTER_ASSERT(reporter, strike != nullptr);
    }

    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, context->fontCacheUsed() > 0);

    // Drop byte limit to 0 while the strike is pinned (fCanDelete == false).
    context->setFontCacheLimit(0);
    context->purgePinnedFontCache();
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, context->fontCacheUsed() > 0);

    // Once the client unpins the strike, purgePinnedFontCache() evicts it.
    pinnerPtr->fCanDelete = true;
    context->purgePinnedFontCache();
    REPORTER_ASSERT(reporter, context->fontCacheCountUsed() == 0);
    REPORTER_ASSERT(reporter, context->fontCacheUsed() == 0);
}

DEF_TEST(SkStrikeCache_IndependentContextsIsolation, reporter) {
    std::unique_ptr<SkContext> contextA = SkContexts::MakeRaster({});
    std::unique_ptr<SkContext> contextB = SkContexts::MakeRaster({});
    REPORTER_ASSERT(reporter, contextA != nullptr);
    REPORTER_ASSERT(reporter, contextB != nullptr);
    REPORTER_ASSERT(reporter, contextA->priv().fontCache() != contextB->priv().fontCache());

    SkFont font = make_test_font(18.0f);
    SkStrikeSpec strikeSpec = make_test_strike_spec(font);

    // Populating contextA's strike cache does not affect contextB.
    {
        sk_sp<SkStrike> strikeA = strikeSpec.findOrCreateStrike(contextA->priv().fontCache());
        REPORTER_ASSERT(reporter, strikeA != nullptr);
    }
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() > 0);
    REPORTER_ASSERT(reporter, contextB->fontCacheCountUsed() == 0);
    REPORTER_ASSERT(reporter, contextB->fontCacheUsed() == 0);

    // Populate contextB's strike cache as well.
    {
        sk_sp<SkStrike> strikeB = strikeSpec.findOrCreateStrike(contextB->priv().fontCache());
        REPORTER_ASSERT(reporter, strikeB != nullptr);
    }
    REPORTER_ASSERT(reporter, contextB->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, contextB->fontCacheUsed() > 0);

    // Purging contextA leaves contextB's cached strikes intact.
    contextA->purgeFontCache();
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 0);
    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, contextB->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, contextB->fontCacheUsed() > 0);

    // Changing limits on contextA does not affect contextB.
    contextA->setFontCacheLimit(1024);
    contextA->setFontCacheCountLimit(4);
    REPORTER_ASSERT(reporter, contextB->fontCacheLimit() == SK_DEFAULT_FONT_CACHE_LIMIT);
    REPORTER_ASSERT(reporter, contextB->fontCacheCountLimit() == SK_DEFAULT_FONT_CACHE_COUNT_LIMIT);
}

DEF_TEST(SkStrikeCache_ClientDrawTextPopulatesContextCache, reporter) {
    std::unique_ptr<SkContext> contextA = SkContexts::MakeRaster({});
    std::unique_ptr<SkContext> contextB = SkContexts::MakeRaster({});
    REPORTER_ASSERT(reporter, contextA != nullptr);
    REPORTER_ASSERT(reporter, contextB != nullptr);

    std::unique_ptr<skcpu::Recorder> recorderA = contextA->makeCPURecorder();
    REPORTER_ASSERT(reporter, recorderA != nullptr);

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surfaceA = recorderA->makeBitmapSurface(imageInfo);
    REPORTER_ASSERT(reporter, surfaceA != nullptr);

    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 0);

    SkFont font16 = make_test_font(16.0f);
    SkPaint paint;

    // Drawing text on surfaceA populates contextA's strike cache, leaving contextB untouched.
    surfaceA->getCanvas()->drawString("Hello SkContext", 10.0f, 30.0f, font16, paint);
    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() > 0);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 1);
    REPORTER_ASSERT(reporter, contextB->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, contextB->fontCacheCountUsed() == 0);

    // Drawing another string with the same font & matrix reuses the cached strike in contextA.
    surfaceA->getCanvas()->drawString("More glyphs", 10.0f, 50.0f, font16, paint);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 1);

    // Drawing a TextBlob with a different font size adds a second strike to contextA's cache.
    SkFont font32 = make_test_font(32.0f);
    sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("Blob Text", font32);
    surfaceA->getCanvas()->drawTextBlob(blob, 10.0f, 85.0f, paint);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 2);

    // Purging contextA's font cache resets its memory and count usage to 0.
    contextA->purgeFontCache();
    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() == 0);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 0);

    // Re-drawing after purge repopulates contextA's strike cache.
    surfaceA->getCanvas()->drawString("After purge", 10.0f, 30.0f, font16, paint);
    REPORTER_ASSERT(reporter, contextA->fontCacheUsed() > 0);
    REPORTER_ASSERT(reporter, contextA->fontCacheCountUsed() == 1);
}
#endif
