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

#ifdef SK_BUILD_FOR_WIN
    #include "SkTypeface_win.h"
#endif

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"

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

// limit this just so we don't take too long to draw
#define MAX_TOTAL_TEXT 4096
#define MAX_CHAR 256
#define MAX_FAMILIES 30

static const int kWidth = 1024;
static const int kHeight = 768;

// This test hammers the GPU textblobcache and font atlas
DEF_GPUTEST(TextBlobCache, reporter, factory) {
    // setup surface
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);

    GrContext* ctx = factory->get(GrContextFactory::kNative_GLContextType);
    SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kN32_SkColorType, kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info,
                                                               0, &props));
    REPORTER_ASSERT(reporter, surface);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

    SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());

    int count = SkMin32(fm->countFamilies(), MAX_FAMILIES);

    // make a ton of text
    uint16_t text[MAX_TOTAL_TEXT];
    for (int i = 0; i < MAX_TOTAL_TEXT; i++) {
        text[i] = i % MAX_CHAR;
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
            set->getStyle(j, &fs, NULL);

            SkSafeUnref(paint.setTypeface(set->createTypeface(j)));

            SkTextBlobBuilder builder;
            for (int aa = 0; aa < 2; aa++) {
                for (int subpixel = 0; subpixel < 2; subpixel++) {
                    for (int lcd = 0; lcd < 2; lcd++) {
                        paint.setAntiAlias(SkToBool(aa));
                        paint.setSubpixelText(SkToBool(subpixel));
                        paint.setLCDRenderText(SkToBool(lcd));
                        const SkTextBlobBuilder::RunBuffer& run = builder.allocRun(paint,
                                                                                   MAX_TOTAL_TEXT,
                                                                                   0, 0,
                                                                                   NULL);
                        memcpy(run.glyphs, text, MAX_TOTAL_TEXT * sizeof(uint16_t));
                    }
                }
            }
            SkNEW_APPEND_TO_TARRAY(&blobs, TextBlobWrapper, (builder.build()));
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
    ctx->freeGpuResources();
    draw(canvas, 1, blobs);

    ctx->freeGpuResources();
    draw(canvasNoLCD, 1, blobs);

    // test draw after abandon
    ctx->abandonContext();
    draw(canvas, 1, blobs);
}
#endif
