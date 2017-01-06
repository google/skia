/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTextBlob.h"
#include "SkFontMgr.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "SkTypeface.h"
#include "../src/fonts/SkRandomScalerContext.h"

#ifdef SK_BUILD_FOR_WIN
    #include "SkTypeface_win.h"
#endif

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTest.h"

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

// This test hammers the GPU textblobcache and font atlas
static void text_blob_cache_inner(skiatest::Reporter* reporter, GrContext* context,
                                  int maxTotalText, int maxGlyphID, int maxFamilies, bool normal,
                                  bool stressTest) {
    // setup surface
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);

    // configure our context for maximum stressing of cache and atlas
    if (stressTest) {
        GrTest::SetupAlwaysEvictAtlas(context);
        context->setTextBlobCacheLimit_ForTesting(0);
    }

    SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kN32_SkColorType, kPremul_SkAlphaType);
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
        SkPaint paint;
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint.setTextSize(48); // draw big glyphs to really stress the atlas

        SkString familyName;
        fm->getFamilyName(i, &familyName);
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        for (int j = 0; j < set->count(); ++j) {
            SkFontStyle fs;
            set->getStyle(j, &fs, nullptr);

            // We use a typeface which randomy returns unexpected mask formats to fuzz
            sk_sp<SkTypeface> orig(set->createTypeface(j));
            if (normal) {
                paint.setTypeface(orig);
            } else {
                paint.setTypeface(sk_make_sp<SkRandomTypeface>(orig, paint, true));
            }

            SkTextBlobBuilder builder;
            for (int aa = 0; aa < 2; aa++) {
                for (int subpixel = 0; subpixel < 2; subpixel++) {
                    for (int lcd = 0; lcd < 2; lcd++) {
                        paint.setAntiAlias(SkToBool(aa));
                        paint.setSubpixelText(SkToBool(subpixel));
                        paint.setLCDRenderText(SkToBool(lcd));
                        if (!SkToBool(lcd)) {
                            paint.setTextSize(160);
                        }
                        const SkTextBlobBuilder::RunBuffer& run = builder.allocRun(paint,
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
    info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
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

DEF_GPUTEST_FOR_NULLGL_CONTEXT(TextBlobCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 1024, 256, 30, true, false);
}

DEF_GPUTEST_FOR_NULLGL_CONTEXT(TextBlobStressCache, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, true, true);
}

DEF_GPUTEST_FOR_NULLGL_CONTEXT(TextBlobAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, false, false);
}

DEF_GPUTEST_FOR_NULLGL_CONTEXT(TextBlobStressAbnormal, reporter, ctxInfo) {
    text_blob_cache_inner(reporter, ctxInfo.grContext(), 256, 256, 10, false, true);
}
#endif
