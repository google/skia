/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ToolUtils.h"

#include <string>

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkGlyphRun.h"
#include "tools/fonts/RandomScalerContext.h"

#ifdef SK_BUILD_FOR_WIN
    #include "include/ports/SkTypeface_win.h"
#endif

#include "tests/Test.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"

static void draw(SkCanvas* canvas, int redraw, const SkTArray<sk_sp<SkTextBlob>>& blobs) {
    int yOffset = 0;
    for (int r = 0; r < redraw; r++) {
        for (int i = 0; i < blobs.count(); i++) {
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

static void setup_always_evict_atlas(GrContext* context) {
    context->priv().getAtlasManager()->setAtlasSizesToMinimum_ForTesting();
}

// This test hammers the GPU textblobcache and font atlas
static void text_blob_cache_inner(skiatest::Reporter* reporter, GrContext* context,
                                  int maxTotalText, int maxGlyphID, int maxFamilies, bool normal,
                                  bool stressTest) {
    // setup surface
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);

    // configure our context for maximum stressing of cache and atlas
    if (stressTest) {
        setup_always_evict_atlas(context);
        context->priv().testingOnly_setTextBlobCacheLimit(0);
    }

    SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);
    auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0, &props));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());

    int count = SkMin32(fm->countFamilies(), maxFamilies);

    // make a ton of text
    SkAutoTArray<uint16_t> text(maxTotalText);
    for (int i = 0; i < maxTotalText; i++) {
        text[i] = i % maxGlyphID;
    }

    // generate textblobs
    SkTArray<sk_sp<SkTextBlob>> blobs;
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
    context->freeGpuResources();
    draw(canvas, 1, blobs);

    context->freeGpuResources();
    draw(canvasNoLCD, 1, blobs);

    // test draw after abandon
    context->abandonContext();
    draw(canvas, 1, blobs);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(TextBlobCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 1024, 256, 30, true, false);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(TextBlobStressCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, true, true);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(TextBlobAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, false, false);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(TextBlobStressAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, false, true);
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
    auto tf = SkTypeface::MakeFromName("Roboto2-Regular", SkFontStyle());
    SkFont font;
    font.setTypeface(tf);
    font.setSubpixel(false);
    font.setEdging(SkFont::Edging::kAlias);
    font.setSize(24);

    static char text[] = "HekpqB";
    static const int maxGlyphLen = sizeof(text) * 4;
    SkGlyphID glyphs[maxGlyphLen];
    int glyphCount =
            font.textToGlyphs(text, sizeof(text), SkTextEncoding::kUTF8, glyphs, maxGlyphLen);

    SkTextBlobBuilder builder;
    const auto& runBuffer = builder.allocRun(font, glyphCount, 0, 0);
    for (int i = 0; i < glyphCount; i++) {
        runBuffer.glyphs[i] = glyphs[i];
    }
    return builder.make();
}

static const bool kDumpPngs = true;
// dump pngs needs a "good" and a "bad" directory to put the results in. This allows the use of the
// skdiff tool to visualize the differences.

void write_png(const std::string& filename, const SkBitmap& bitmap) {
    auto data = SkEncodeBitmap(bitmap, SkEncodedImageFormat::kPNG, 0);
    SkFILEWStream w{filename.c_str()};
    w.write(data->data(), data->size());
    w.fsync();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextBlobJaggedGlyph, reporter, ctxInfo) {
    auto grContext = ctxInfo.grContext();
    const SkImageInfo info =
            SkImageInfo::Make(kScreenDim, kScreenDim, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(grContext, SkBudgeted::kNo, info);

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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(TextBlobSmoothScroll, reporter, ctxInfo) {
    auto grContext = ctxInfo.grContext();
    const SkImageInfo info =
            SkImageInfo::Make(kScreenDim, kScreenDim, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(grContext, SkBudgeted::kNo, info);

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
