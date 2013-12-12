/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCanvasStateUtils.h"
#include "SkDrawFilter.h"
#include "SkError.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRRect.h"

static void test_complex_layers(skiatest::Reporter* reporter) {
    const int WIDTH = 400;
    const int HEIGHT = 400;
    const int SPACER = 10;

    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(SPACER), SkIntToScalar(SPACER),
                                   SkIntToScalar(WIDTH-(2*SPACER)),
                                   SkIntToScalar((HEIGHT-(2*SPACER)) / 7));

    const SkBitmap::Config configs[] = { SkBitmap::kRGB_565_Config,
                                         SkBitmap::kARGB_8888_Config
    };
    const int configCount = sizeof(configs) / sizeof(SkBitmap::Config);

    const int layerAlpha[] = { 255, 255, 0 };
    const SkCanvas::SaveFlags flags[] = { SkCanvas::kARGB_NoClipLayer_SaveFlag,
                                          SkCanvas::kARGB_ClipLayer_SaveFlag,
                                          SkCanvas::kARGB_NoClipLayer_SaveFlag
    };
    REPORTER_ASSERT(reporter, sizeof(layerAlpha) == sizeof(flags));
    const int layerCombinations = sizeof(layerAlpha) / sizeof(int);

    for (int i = 0; i < configCount; ++i) {
        SkBitmap bitmaps[2];
        for (int j = 0; j < 2; ++j) {
            bitmaps[j].setConfig(configs[i], WIDTH, HEIGHT);
            bitmaps[j].allocPixels();

            SkCanvas canvas(bitmaps[j]);

            canvas.drawColor(SK_ColorRED);

            for (int k = 0; k < layerCombinations; ++k) {
                // draw a rect within the layer's bounds and again outside the layer's bounds
                canvas.saveLayerAlpha(&rect, layerAlpha[k], flags[k]);

                SkCanvasState* state = NULL;
                SkCanvas* tmpCanvas = NULL;
                if (j) {
                    state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
                    REPORTER_ASSERT(reporter, state);
                    tmpCanvas = SkCanvasStateUtils::CreateFromCanvasState(state);
                    REPORTER_ASSERT(reporter, tmpCanvas);
                } else {
                    tmpCanvas = SkRef(&canvas);
                }

                SkPaint bluePaint;
                bluePaint.setColor(SK_ColorBLUE);
                bluePaint.setStyle(SkPaint::kFill_Style);

                tmpCanvas->drawRect(rect, bluePaint);
                tmpCanvas->translate(0, rect.height() + SPACER);
                tmpCanvas->drawRect(rect, bluePaint);

                tmpCanvas->unref();
                SkCanvasStateUtils::ReleaseCanvasState(state);

                canvas.restore();

                // translate the canvas for the next iteration
                canvas.translate(0, 2*(rect.height() + SPACER));
            }
        }

        // now we memcmp the two bitmaps
        REPORTER_ASSERT(reporter, bitmaps[0].getSize() == bitmaps[1].getSize());
        REPORTER_ASSERT(reporter, !memcmp(bitmaps[0].getPixels(),
                                          bitmaps[1].getPixels(),
                                          bitmaps[0].getSize()));
    }
}

////////////////////////////////////////////////////////////////////////////////

static void test_complex_clips(skiatest::Reporter* reporter) {

    const int WIDTH = 400;
    const int HEIGHT = 400;
    const int SPACER = 10;

    SkIRect layerRect = SkIRect::MakeWH(WIDTH, HEIGHT / 4);
    layerRect.inset(2*SPACER, 2*SPACER);

    SkIRect clipRect = layerRect;
    clipRect.fRight = clipRect.fLeft + (clipRect.width() / 2) - (2*SPACER);
    clipRect.outset(SPACER, SPACER);

    SkIRect regionBounds = clipRect;
    regionBounds.offset(clipRect.width() + (2*SPACER), 0);

    SkIRect regionInterior = regionBounds;
    regionInterior.inset(SPACER*3, SPACER*3);

    SkRegion clipRegion;
    clipRegion.setRect(regionBounds);
    clipRegion.op(regionInterior, SkRegion::kDifference_Op);


    const SkRegion::Op clipOps[] = { SkRegion::kIntersect_Op,
                                     SkRegion::kIntersect_Op,
                                     SkRegion::kReplace_Op,
    };
    const SkCanvas::SaveFlags flags[] = { SkCanvas::kARGB_NoClipLayer_SaveFlag,
                                          SkCanvas::kARGB_ClipLayer_SaveFlag,
                                          SkCanvas::kARGB_NoClipLayer_SaveFlag,
    };
    REPORTER_ASSERT(reporter, sizeof(clipOps) == sizeof(flags));
    const int layerCombinations = sizeof(flags) / sizeof(SkCanvas::SaveFlags);

    SkBitmap bitmaps[2];
    for (int i = 0; i < 2; ++i) {
        bitmaps[i].setConfig(SkBitmap::kARGB_8888_Config, WIDTH, HEIGHT);
        bitmaps[i].allocPixels();

        SkCanvas canvas(bitmaps[i]);

        canvas.drawColor(SK_ColorRED);

        SkRegion localRegion = clipRegion;

        for (int j = 0; j < layerCombinations; ++j) {
            SkRect layerBounds = SkRect::Make(layerRect);
            canvas.saveLayerAlpha(&layerBounds, 128, flags[j]);

            SkCanvasState* state = NULL;
            SkCanvas* tmpCanvas = NULL;
            if (i) {
                state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
                REPORTER_ASSERT(reporter, state);
                tmpCanvas = SkCanvasStateUtils::CreateFromCanvasState(state);
                REPORTER_ASSERT(reporter, tmpCanvas);
            } else {
                tmpCanvas = SkRef(&canvas);
            }

            tmpCanvas->save();
            tmpCanvas->clipRect(SkRect::Make(clipRect), clipOps[j]);
            tmpCanvas->drawColor(SK_ColorBLUE);
            tmpCanvas->restore();

            tmpCanvas->clipRegion(localRegion, clipOps[j]);
            tmpCanvas->drawColor(SK_ColorBLUE);

            tmpCanvas->unref();
            SkCanvasStateUtils::ReleaseCanvasState(state);

            canvas.restore();

            // translate the canvas and region for the next iteration
            canvas.translate(0, SkIntToScalar(2*(layerRect.height() + (SPACER))));
            localRegion.translate(0, 2*(layerRect.height() + SPACER));
        }
    }

    // now we memcmp the two bitmaps
    REPORTER_ASSERT(reporter, bitmaps[0].getSize() == bitmaps[1].getSize());
    REPORTER_ASSERT(reporter, !memcmp(bitmaps[0].getPixels(),
                                      bitmaps[1].getPixels(),
                                      bitmaps[0].getSize()));
}

////////////////////////////////////////////////////////////////////////////////

class TestDrawFilter : public SkDrawFilter {
public:
    virtual bool filter(SkPaint*, Type) SK_OVERRIDE { return true; }
};

static void test_draw_filters(skiatest::Reporter* reporter) {
    TestDrawFilter drawFilter;
    SkBitmapDevice device(SkBitmap::kARGB_8888_Config, 10, 10);
    SkCanvas canvas(&device);

    canvas.setDrawFilter(&drawFilter);

    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
    REPORTER_ASSERT(reporter, state);
    SkCanvas* tmpCanvas = SkCanvasStateUtils::CreateFromCanvasState(state);
    REPORTER_ASSERT(reporter, tmpCanvas);

    REPORTER_ASSERT(reporter, NULL != canvas.getDrawFilter());
    REPORTER_ASSERT(reporter, NULL == tmpCanvas->getDrawFilter());

    tmpCanvas->unref();
    SkCanvasStateUtils::ReleaseCanvasState(state);
}

////////////////////////////////////////////////////////////////////////////////

// we need this function to prevent SkError from printing to stdout
static void error_callback(SkError code, void* ctx) {}

static void test_soft_clips(skiatest::Reporter* reporter) {
    SkBitmapDevice device(SkBitmap::kARGB_8888_Config, 10, 10);
    SkCanvas canvas(&device);

    SkRRect roundRect;
    roundRect.setOval(SkRect::MakeWH(5, 5));

    canvas.clipRRect(roundRect, SkRegion::kIntersect_Op, true);

    SkSetErrorCallback(error_callback, NULL);

    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
    REPORTER_ASSERT(reporter, !state);

    REPORTER_ASSERT(reporter, kInvalidOperation_SkError == SkGetLastError());
    SkClearLastError();
}

DEF_TEST(CanvasState, reporter) {
    test_complex_layers(reporter);
    test_complex_clips(reporter);
    test_draw_filters(reporter);
    test_soft_clips(reporter);
}
