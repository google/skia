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

struct TextBlobWrapper {
    // This class assumes it 'owns' the textblob it wraps, and thus does not need to take a ref
    explicit TextBlobWrapper(const SkTextBlob* blob) : fBlob(blob) {}
    TextBlobWrapper(const TextBlobWrapper& blob) : fBlob(SkRef(blob.fBlob.get())) {}

    SkAutoTUnref<const SkTextBlob> fBlob;
};

static void draw(SkCanvas* canvas, int redraw, const SkTArray<TextBlobWrapper>& blobs) {
    int yOffset = 0;
    for (int r = 0; r < redraw; r++) {
        for (int i = 0; i < blobs.count(); i++) {
            const SkTextBlob* blob = blobs[i].fBlob.get();
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
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(context, SkBudgeted::kNo, info,
                                                               0, &props));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());

    int count = SkMin32(fm->countFamilies(), maxFamilies);

    // make a ton of text
    SkAutoTArray<uint16_t> text(maxTotalText);
    for (int i = 0; i < maxTotalText; i++) {
        text[i] = i % maxGlyphID;
    }

    // generate textblobs
    SkTArray<TextBlobWrapper> blobs;
    for (int i = 0; i < count; i++) {
        SkPaint paint;
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint.setTextSize(48); // draw big glyphs to really stress the atlas

        SkString familyName;
        fm->getFamilyName(i, &familyName);
        SkAutoTUnref<SkFontStyleSet> set(fm->createStyleSet(i));
        for (int j = 0; j < set->count(); ++j) {
            SkFontStyle fs;
            set->getStyle(j, &fs, nullptr);

            // We use a typeface which randomy returns unexpected mask formats to fuzz
            SkAutoTUnref<SkTypeface> orig(set->createTypeface(j));
            if (normal) {
                paint.setTypeface(orig);
            } else {
                SkAutoTUnref<SkTypeface> typeface(new SkRandomTypeface(orig, paint, true));
                paint.setTypeface(typeface);
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
            blobs.emplace_back(builder.build());
        }
    }

    // create surface where LCD is impossible
    info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    SkSurfaceProps propsNoLCD(0, kUnknown_SkPixelGeometry);
    SkAutoTUnref<SkSurface> surfaceNoLCD(canvas->newSurface(info, &propsNoLCD));
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

DEF_GPUTEST_FOR_NULL_CONTEXT(TextBlobCache, reporter, context) {
    text_blob_cache_inner(reporter, context, 1024, 256, 30, true, false);
}

DEF_GPUTEST_FOR_NULL_CONTEXT(TextBlobStressCache, reporter, context) {
    text_blob_cache_inner(reporter, context, 256, 256, 10, true, true);
}

DEF_GPUTEST_FOR_NULL_CONTEXT(TextBlobAbnormal, reporter, context) {
    text_blob_cache_inner(reporter, context, 256, 256, 10, false, false);
}

DEF_GPUTEST_FOR_NULL_CONTEXT(TextBlobStressAbnormal, reporter, context) {
    text_blob_cache_inner(reporter, context, 256, 256, 10, false, true);
}
#endif
