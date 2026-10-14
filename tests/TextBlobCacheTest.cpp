/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkSpinlock.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/fonts/RandomScalerContext.h"
#include "tools/ganesh/GrAtlasTools.h"

#ifdef SK_BUILD_FOR_WIN
    #include "include/ports/SkTypeface_win.h"
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>

using namespace skia_private;

struct GrContextOptions;

static void draw(SkCanvas* canvas, int redraw, const TArray<sk_sp<SkTextBlob>>& blobs) {
    int yOffset = 0;
    for (int r = 0; r < redraw; r++) {
        for (int i = 0; i < blobs.size(); i++) {
            const auto& blob = blobs[i];
            const SkRect& bounds = blob->bounds();
            yOffset += SkScalarCeilToInt(bounds.height());
            SkPaint paint;
            canvas->drawTextBlob(blob, 0, SkIntToScalar(yOffset), paint);
        }
    }
}

static const int kWidth = 1024;
static const int kHeight = 768;

static void setup_always_evict_atlas(GrDirectContext* dContext) {
    GrAtlasManagerTools::SetAtlasDimensionsToMinimum(dContext->priv().getAtlasManager());
}

class GrTextBlobTestingPeer {
public:
    static void SetBudget(sktext::gpu::TextBlobRedrawCoordinator* cache, size_t budget) {
        SkAutoSpinlock lock{cache->fSpinLock};
        cache->fSizeBudget = budget;
        cache->internalCheckPurge();
    }
};

// This test hammers the GPU textblobcache and font atlas
static void text_blob_cache_inner(skiatest::Reporter* reporter, GrDirectContext* dContext,
                                  int maxTotalText, int maxGlyphID, int maxFamilies, bool normal,
                                  bool stressTest) {
    // setup surface
    uint32_t flags = 0;
    SkSurfaceProps props(flags, kRGB_H_SkPixelGeometry);

    // configure our context for maximum stressing of cache and atlas
    if (stressTest) {
        setup_always_evict_atlas(dContext);
        GrTextBlobTestingPeer::SetBudget(dContext->priv().getTextBlobCache(), 0);
    }

    SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info, 0, &props));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    sk_sp<SkFontMgr> fm(ToolUtils::TestFontMgr());

    int count = std::min(fm->countFamilies(), maxFamilies);

    // make a ton of text
    AutoTArray<uint16_t> text(maxTotalText);
    for (int i = 0; i < maxTotalText; i++) {
        text[i] = i % maxGlyphID;
    }

    // generate textblobs
    TArray<sk_sp<SkTextBlob>> blobs;
    for (int i = 0; i < count; i++) {
        SkFont font;
        font.setSize(48); // draw big glyphs to really stress the atlas

        SkString familyName;
        fm->getFamilyName(i, &familyName);
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        for (int j = 0; j < set->count(); ++j) {
            SkFontStyle fs;
            set->getStyle(j, &fs, nullptr);

            // We use a typeface which randomy returns unexpected mask formats to fuzz
            sk_sp<SkTypeface> orig(set->createTypeface(j));
            if (normal) {
                font.setTypeface(orig);
            } else {
                font.setTypeface(sk_make_sp<SkRandomTypeface>(orig, SkPaint(), true));
            }

            SkTextBlobBuilder builder;
            for (int aa = 0; aa < 2; aa++) {
                for (int subpixel = 0; subpixel < 2; subpixel++) {
                    for (int lcd = 0; lcd < 2; lcd++) {
                        font.setEdging(SkFont::Edging::kAlias);
                        if (aa) {
                            font.setEdging(SkFont::Edging::kAntiAlias);
                            if (lcd) {
                                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                            }
                        }
                        font.setSubpixel(SkToBool(subpixel));
                        if (!SkToBool(lcd)) {
                            font.setSize(160);
                        }
                        const SkTextBlobBuilder::RunBuffer& run = builder.allocRun(font,
                                                                                   maxTotalText,
                                                                                   0, 0,
                                                                                   nullptr);
                        memcpy(run.glyphs, text.get(), maxTotalText * sizeof(uint16_t));
                    }
                }
            }
            blobs.emplace_back(builder.make());
        }
    }

    // create surface where LCD is impossible
    info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkSurfaceProps propsNoLCD(0, kUnknown_SkPixelGeometry);
    auto surfaceNoLCD(canvas->makeSurface(info, &propsNoLCD));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvasNoLCD = surfaceNoLCD->getCanvas();

    // test redraw
    draw(canvas, 2, blobs);
    draw(canvasNoLCD, 2, blobs);

    // test draw after free
    dContext->freeGpuResources();
    draw(canvas, 1, blobs);

    dContext->freeGpuResources();
    draw(canvasNoLCD, 1, blobs);

    // test draw after abandon
    dContext->abandonContext();
    draw(canvas, 1, blobs);
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(TextBlobCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.directContext(), 1024, 256, 30, true, false);
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(TextBlobStressCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.directContext(), 256, 256, 10, true, true);
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(TextBlobAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.directContext(), 256, 256, 10, false, false);
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(TextBlobStressAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.directContext(), 256, 256, 10, false, true);
}

static const int kScreenDim = 160;

static SkBitmap draw_blob(SkTextBlob* blob, SkSurface* surface, SkPoint offset) {

    SkPaint paint;

    SkCanvas* canvas = surface->getCanvas();
    canvas->save();
    canvas->drawColor(SK_ColorWHITE, SkBlendMode::kSrc);
    canvas->translate(offset.fX, offset.fY);
    canvas->drawTextBlob(blob, 0, 0, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(kScreenDim, kScreenDim);
    surface->readPixels(bitmap, 0, 0);
    canvas->restore();
    return bitmap;
}

static bool compare_bitmaps(const SkBitmap& expected, const SkBitmap& actual) {
    SkASSERT(expected.width() == actual.width());
    SkASSERT(expected.height() == actual.height());
    for (int i = 0; i < expected.width(); ++i) {
        for (int j = 0; j < expected.height(); ++j) {
            SkColor expectedColor = expected.getColor(i, j);
            SkColor actualColor = actual.getColor(i, j);
            if (expectedColor != actualColor) {
                return false;
            }
        }
    }
    return true;
}

static sk_sp<SkTextBlob> make_blob() {
    auto tf = ToolUtils::CreateTestTypeface("Roboto2-Regular", SkFontStyle());
    SkFont font;
    font.setTypeface(tf);
    font.setSubpixel(false);
    font.setEdging(SkFont::Edging::kAlias);
    font.setSize(24);

    static char text[] = "HekpqB";
    static const int maxGlyphLen = sizeof(text) * 4;
    std::array<SkGlyphID, maxGlyphLen> glyphs;
    int glyphCount =
            font.textToGlyphs(text, sizeof(text), SkTextEncoding::kUTF8, glyphs);

    SkTextBlobBuilder builder;
    const auto& runBuffer = builder.allocRun(font, glyphCount, 0, 0);
    for (int i = 0; i < glyphCount; i++) {
        runBuffer.glyphs[i] = glyphs[i];
    }
    return builder.make();
}

// Turned off to pass on android and ios devices, which were running out of memory..
#if 0
static sk_sp<SkTextBlob> make_large_blob() {
    auto tf = ToolUtils::CreateTestTypeface("Roboto2-Regular", SkFontStyle());
    SkFont font;
    font.setTypeface(tf);
    font.setSubpixel(false);
    font.setEdging(SkFont::Edging::kAlias);
    font.setSize(24);

    const int mallocSize = 0x3c3c3bd; // x86 size
    std::unique_ptr<char[]> text{new char[mallocSize + 1]};
    if (text == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < mallocSize; i++) {
        text[i] = 'x';
    }
    text[mallocSize] = 0;

    static const int maxGlyphLen = mallocSize;
    std::unique_ptr<SkGlyphID[]> glyphs{new SkGlyphID[maxGlyphLen]};
    int glyphCount =
            font.textToGlyphs(
                    text.get(), mallocSize, SkTextEncoding::kUTF8, glyphs.get(), maxGlyphLen);
    SkTextBlobBuilder builder;
    const auto& runBuffer = builder.allocRun(font, glyphCount, 0, 0);
    for (int i = 0; i < glyphCount; i++) {
        runBuffer.glyphs[i] = glyphs[i];
    }
    return builder.make();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextBlobIntegerOverflowTest, reporter, ctxInfo,
                                   CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    const SkImageInfo info =
            SkImageInfo::Make(kScreenDim, kScreenDim, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);

    auto blob = make_large_blob();
    int y = 40;
    SkBitmap base = draw_blob(blob.get(), surface.get(), {40, y + 0.0f});
}
#endif

static const bool kDumpPngs = true;
// dump pngs needs a "good" and a "bad" directory to put the results in. This allows the use of the
// skdiff tool to visualize the differences.

void write_png(const std::string& filename, const SkBitmap& bitmap) {
    SkFILEWStream w{filename.c_str()};
    SkASSERT_RELEASE(SkPngEncoder::Encode(&w, bitmap.pixmap(), {}));
    w.fsync();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextBlobJaggedGlyph,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();
    const SkImageInfo info =
            SkImageInfo::Make(kScreenDim, kScreenDim, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(direct, skgpu::Budgeted::kNo, info);

    auto blob = make_blob();

    for (int y = 40; y < kScreenDim - 40; y++) {
        SkBitmap base = draw_blob(blob.get(), surface.get(), {40, y + 0.0f});
        SkBitmap half = draw_blob(blob.get(), surface.get(), {40, y + 0.5f});
        SkBitmap unit = draw_blob(blob.get(), surface.get(), {40, y + 1.0f});
        bool isOk = compare_bitmaps(base, half) || compare_bitmaps(unit, half);
        REPORTER_ASSERT(reporter, isOk);
        if (!isOk) {
            if (kDumpPngs) {
                {
                    std::string filename = "bad/half-y" + std::to_string(y) + ".png";
                    write_png(filename, half);
                }
                {
                    std::string filename = "good/half-y" + std::to_string(y) + ".png";
                    write_png(filename, base);
                }
            }
            break;
        }
    }

    // Testing the x direction across all platforms does not workout, because letter spacing can
    // change based on non-integer advance widths, but this has been useful for diagnosing problems.
#if 0
    blob = make_blob();
    for (int x = 40; x < kScreenDim - 40; x++) {
        SkBitmap base = draw_blob(blob.get(), surface.get(), {x + 0.0f, 40});
        SkBitmap half = draw_blob(blob.get(), surface.get(), {x + 0.5f, 40});
        SkBitmap unit = draw_blob(blob.get(), surface.get(), {x + 1.0f, 40});
        bool isOk = compare_bitmaps(base, half) || compare_bitmaps(unit, half);
        REPORTER_ASSERT(reporter, isOk);
        if (!isOk) {
            if (kDumpPngs) {
                {
                    std::string filename = "bad/half-x" + std::to_string(x) + ".png";
                    write_png(filename, half);
                }
                {
                    std::string filename = "good/half-x" + std::to_string(x) + ".png";
                    write_png(filename, base);
                }
            }
            break;
        }
    }
#endif
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextBlobSmoothScroll,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto direct = ctxInfo.directContext();
    const SkImageInfo info =
            SkImageInfo::Make(kScreenDim, kScreenDim, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(direct, skgpu::Budgeted::kNo, info);

    auto movingBlob = make_blob();

    for (SkScalar y = 40; y < 50; y += 1.0/8.0) {
        auto expectedBlob = make_blob();
        auto expectedBitMap = draw_blob(expectedBlob.get(), surface.get(), {40, y});
        auto movingBitmap = draw_blob(movingBlob.get(), surface.get(), {40, y});
        bool isOk = compare_bitmaps(expectedBitMap, movingBitmap);
        REPORTER_ASSERT(reporter, isOk);
        if (!isOk) {
            if (kDumpPngs) {
                {
                    std::string filename = "bad/scroll-y" + std::to_string(y) + ".png";
                    write_png(filename, movingBitmap);
                }
                {
                    std::string filename = "good/scroll-y" + std::to_string(y) + ".png";
                    write_png(filename, expectedBitMap);
                }
            }
            break;
        }
    }
}

namespace {
sk_sp<SkTypeface> make_path_typeface() {
    SkCustomTypefaceBuilder builder;
    constexpr SkRect kGlyphRect = SkRect::MakeLTRB(0, -1, 1, 0);
    builder.setGlyph(1, 1.0f, SkPath::Rect(kGlyphRect));
    return builder.detach();
}

sk_sp<SkTextBlob> make_single_glyph_blob(sk_sp<SkTypeface> tf, SkScalar size) {
    SkTextBlobBuilder builder;
    builder.allocRun(SkFont(std::move(tf), size), 1, 0, 0).glyphs[0] = 1;
    return builder.make();
}
}  // namespace

// SkCanvas::drawTextBlob caches sktext::gpu::TextBlob instances in TextBlobRedrawCoordinator.
// Cached TextBlobs are never serialized after drawing and must release their SkStrike references
// once the initial draw is flushed so SkStrikeCache::purgeAll() can evict the strikes.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TextBlob_draw_releases_strike_from_cache,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto surface = SkSurfaces::RenderTarget(
            dContext, skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(64, 64));
    REPORTER_ASSERT(reporter, surface);
    auto canvas = surface->getCanvas();

    // Use a dedicated typeface so no other cache entry shares its strikes
    sk_sp<SkTypeface> customTypeface = make_path_typeface();

    // 400pt exercises PathSubRun (PathOpSubmitter); 16pt exercises DirectMaskSubRun (GlyphVector)
    auto pathBlob = make_single_glyph_blob(customTypeface, /*size=*/400);
    auto maskBlob = make_single_glyph_blob(customTypeface, /*size=*/16);

    SkPaint paint;
    paint.setAntiAlias(true);

    // Drawing and flushing populates TextBlobRedrawCoordinator and converts glyph IDs to SkPaths
    // (in PathOpSubmitter::submitDraws) and atlas Glyph* pointers (in GlyphVector::regenAtlas)
    canvas->drawTextBlob(pathBlob, 0, 64, paint);
    canvas->drawTextBlob(maskBlob, 0, 32, paint);
    dContext->flushAndSubmit();

    // Grab external references to both strikes while they are still in SkStrikeCache
    auto [pathSpec, _] = SkStrikeSpec::MakePath(
            SkFont(customTypeface, 400),
            paint,
            surface->props(),
            SkScalerContextFlags::kFakeGammaAndBoostContrast);
    sk_sp<SkStrike> pathStrike =
            SkStrikeCache::GlobalStrikeCache()->findStrike(pathSpec.descriptor());
    REPORTER_ASSERT(reporter, pathStrike != nullptr);

    SkStrikeSpec maskSpec = SkStrikeSpec::MakeMask(
            SkFont(customTypeface, 16),
            paint,
            surface->props(),
            SkScalerContextFlags::kFakeGammaAndBoostContrast,
            SkMatrix::I());
    sk_sp<SkStrike> maskStrike =
            SkStrikeCache::GlobalStrikeCache()->findStrike(maskSpec.descriptor());
    REPORTER_ASSERT(reporter, maskStrike != nullptr);

    // Purging SkStrikeCache drops its references; if TextBlobRedrawCoordinator also released its
    // fStrikePromise references after the flushed draw, pathStrike and maskStrike are now unique()
    SkStrikeCache::GlobalStrikeCache()->purgeAll();
    REPORTER_ASSERT(reporter, pathStrike->unique());
    REPORTER_ASSERT(reporter, maskStrike->unique());
}

