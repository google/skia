
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkCanvasStateUtils.h"
#include "SkDrawFilter.h"
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

static void test_soft_clips(skiatest::Reporter* reporter) {
    SkBitmapDevice device(SkBitmap::kARGB_8888_Config, 10, 10);
    SkCanvas canvas(&device);

    SkRRect roundRect;
    roundRect.setOval(SkRect::MakeWH(5, 5));

    canvas.clipRRect(roundRect, SkRegion::kIntersect_Op, true);

    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
    REPORTER_ASSERT(reporter, !state);
}

////////////////////////////////////////////////////////////////////////////////

static void test_canvas_state_utils(skiatest::Reporter* reporter) {
    test_complex_layers(reporter);
    test_draw_filters(reporter);
    test_soft_clips(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("CanvasState", TestCanvasStateClass, test_canvas_state_utils)
