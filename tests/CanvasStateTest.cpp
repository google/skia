/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CanvasStateHelpers.h"
#include "CommandLineFlags.h"
#include "SkBitmap.h"
#include "SkCanvasPriv.h"
#include "SkCanvasStateUtils.h"
#include "SkClipOpPriv.h"
#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkPaint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkRegion.h"
#include "SkScalar.h"
#include "SkTDArray.h"
#include "SkTLazy.h"
#include "SkTypes.h"
#include "Test.h"

#include <cstring>
#include <memory>

class SkCanvasState;

// dlopen and the library flag are only used for tests which require this flag.
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
#include <dlfcn.h>

static DEFINE_string(library, "",
                     "Support library to use for CanvasState test. If empty (the default), "
                     "the test will be run without crossing a library boundary. Otherwise, "
                     "it is expected to be a full path to a shared library file, which will"
                     " be dynamically loaded. Functions from the library will be called to "
                     "test SkCanvasState. Instructions for generating the library are in "
                     "gyp/canvas_state_lib.gyp");


// This class calls dlopen on the library passed in to the command line flag library, and handles
// calling dlclose when it goes out of scope.
class OpenLibResult {
public:
    // If the flag library was passed to this run of the test, attempt to open it using dlopen and
    // report whether it succeeded.
    OpenLibResult(skiatest::Reporter* reporter) {
        if (FLAGS_library.count() == 1) {
            fHandle = dlopen(FLAGS_library[0], RTLD_LAZY | RTLD_LOCAL);
            REPORTER_ASSERT(reporter, fHandle != nullptr, "Failed to open library!");
        } else {
            fHandle = nullptr;
        }
    }

    // Automatically call dlclose when going out of scope.
    ~OpenLibResult() {
        if (fHandle) {
            dlclose(fHandle);
        }
    }

    // Pointer to the shared library object.
    void* handle() { return fHandle; }

private:
    void* fHandle;
};

DEF_TEST(CanvasState_test_complex_layers, reporter) {
    const int WIDTH = 400;
    const int HEIGHT = 400;
    const int SPACER = 10;

    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(SPACER), SkIntToScalar(SPACER),
                                   SkIntToScalar(WIDTH-(2*SPACER)),
                                   SkIntToScalar((HEIGHT-(2*SPACER)) / 7));

    const SkColorType colorTypes[] = {
        kRGB_565_SkColorType, kN32_SkColorType
    };

    const int layerAlpha[] = { 255, 255, 0 };
    const SkCanvas::SaveLayerFlags flags[] = {
        static_cast<SkCanvas::SaveLayerFlags>(SkCanvasPriv::kDontClipToLayer_SaveLayerFlag),
        0,
        static_cast<SkCanvas::SaveLayerFlags>(SkCanvasPriv::kDontClipToLayer_SaveLayerFlag),
    };
    REPORTER_ASSERT(reporter, sizeof(layerAlpha) == sizeof(flags));

    bool (*drawFn)(SkCanvasState* state, float l, float t,
                   float r, float b, int32_t s);

    OpenLibResult openLibResult(reporter);
    if (openLibResult.handle() != nullptr) {
        *(void**) (&drawFn) = dlsym(openLibResult.handle(),
                                    "complex_layers_draw_from_canvas_state");
    } else {
        drawFn = complex_layers_draw_from_canvas_state;
    }

    REPORTER_ASSERT(reporter, drawFn);
    if (!drawFn) {
        return;
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(colorTypes); ++i) {
        SkBitmap bitmaps[2];
        for (int j = 0; j < 2; ++j) {
            bitmaps[j].allocPixels(SkImageInfo::Make(WIDTH, HEIGHT,
                                                     colorTypes[i],
                                                     kPremul_SkAlphaType));

            SkCanvas canvas(bitmaps[j]);

            canvas.drawColor(SK_ColorRED);

            for (size_t k = 0; k < SK_ARRAY_COUNT(layerAlpha); ++k) {
                SkTLazy<SkPaint> paint;
                if (layerAlpha[k] != 0xFF) {
                    paint.init()->setAlpha(layerAlpha[k]);
                }

                // draw a rect within the layer's bounds and again outside the layer's bounds
                canvas.saveLayer(SkCanvas::SaveLayerRec(&rect, paint.getMaybeNull(), flags[k]));

                if (j) {
                    // Capture from the first Skia.
                    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
                    REPORTER_ASSERT(reporter, state);

                    // And draw to it in the second Skia.
                    bool success = complex_layers_draw_from_canvas_state(state,
                            rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, SPACER);
                    REPORTER_ASSERT(reporter, success);

                    // And release it in the *first* Skia.
                    SkCanvasStateUtils::ReleaseCanvasState(state);
                } else {
                    // Draw in the first Skia.
                    complex_layers_draw(&canvas, rect.fLeft, rect.fTop,
                                        rect.fRight, rect.fBottom, SPACER);
                }

                canvas.restore();

                // translate the canvas for the next iteration
                canvas.translate(0, 2*(rect.height() + SPACER));
            }
        }

        // now we memcmp the two bitmaps
        REPORTER_ASSERT(reporter, bitmaps[0].computeByteSize() == bitmaps[1].computeByteSize());
        REPORTER_ASSERT(reporter, !memcmp(bitmaps[0].getPixels(),
                                          bitmaps[1].getPixels(),
                                          bitmaps[0].computeByteSize()));
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
DEF_TEST(CanvasState_test_complex_clips, reporter) {
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
    const SkCanvas::SaveLayerFlags flags[] = {
        static_cast<SkCanvas::SaveLayerFlags>(SkCanvasPriv::kDontClipToLayer_SaveLayerFlag),
        0,
        static_cast<SkCanvas::SaveLayerFlags>(SkCanvasPriv::kDontClipToLayer_SaveLayerFlag),
    };
    REPORTER_ASSERT(reporter, sizeof(clipOps) == sizeof(flags));

    bool (*drawFn)(SkCanvasState* state, int32_t l, int32_t t,
                   int32_t r, int32_t b, int32_t clipOp,
                   int32_t regionRects, int32_t* rectCoords);

    OpenLibResult openLibResult(reporter);
    if (openLibResult.handle() != nullptr) {
        *(void**) (&drawFn) = dlsym(openLibResult.handle(),
                                    "complex_clips_draw_from_canvas_state");
    } else {
        drawFn = complex_clips_draw_from_canvas_state;
    }

    REPORTER_ASSERT(reporter, drawFn);
    if (!drawFn) {
        return;
    }

    SkBitmap bitmaps[2];
    for (int i = 0; i < 2; ++i) {
        bitmaps[i].allocN32Pixels(WIDTH, HEIGHT);

        SkCanvas canvas(bitmaps[i]);

        canvas.drawColor(SK_ColorRED);

        SkRegion localRegion = clipRegion;

        SkPaint paint;
        paint.setAlpha(128);
        for (size_t j = 0; j < SK_ARRAY_COUNT(flags); ++j) {
            SkRect layerBounds = SkRect::Make(layerRect);
            canvas.saveLayer(SkCanvas::SaveLayerRec(&layerBounds, &paint, flags[j]));

            if (i) {
                SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
                REPORTER_ASSERT(reporter, state);

                SkRegion::Iterator iter(localRegion);
                SkTDArray<int32_t> rectCoords;
                for (; !iter.done(); iter.next()) {
                    const SkIRect& rect = iter.rect();
                    *rectCoords.append() = rect.fLeft;
                    *rectCoords.append() = rect.fTop;
                    *rectCoords.append() = rect.fRight;
                    *rectCoords.append() = rect.fBottom;
                }
                bool success = drawFn(state, clipRect.fLeft, clipRect.fTop,
                                      clipRect.fRight, clipRect.fBottom, clipOps[j],
                                      rectCoords.count() / 4, rectCoords.begin());
                REPORTER_ASSERT(reporter, success);

                SkCanvasStateUtils::ReleaseCanvasState(state);
            } else {
                complex_clips_draw(&canvas, clipRect.fLeft, clipRect.fTop,
                                   clipRect.fRight, clipRect.fBottom, clipOps[j],
                                   localRegion);
            }

            canvas.restore();

            // translate the canvas and region for the next iteration
            canvas.translate(0, SkIntToScalar(2*(layerRect.height() + (SPACER))));
            localRegion.translate(0, 2*(layerRect.height() + SPACER));
        }
    }

    // now we memcmp the two bitmaps
    REPORTER_ASSERT(reporter, bitmaps[0].computeByteSize() == bitmaps[1].computeByteSize());
    REPORTER_ASSERT(reporter, !memcmp(bitmaps[0].getPixels(),
                                      bitmaps[1].getPixels(),
                                      bitmaps[0].computeByteSize()));
}
#endif

////////////////////////////////////////////////////////////////////////////////

DEF_TEST(CanvasState_test_soft_clips, reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkCanvas canvas(bitmap);

    SkRRect roundRect;
    roundRect.setOval(SkRect::MakeWH(5, 5));

    canvas.clipRRect(roundRect, kIntersect_SkClipOp, true);

    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
    REPORTER_ASSERT(reporter, !state);
}

DEF_TEST(CanvasState_test_saveLayer_clip, reporter) {
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
    static_assert(SkCanvas::kDontClipToLayer_Legacy_SaveLayerFlag ==
                  SkCanvasPriv::kDontClipToLayer_SaveLayerFlag, "");
#endif
    const int WIDTH = 100;
    const int HEIGHT = 100;
    const int LAYER_WIDTH = 50;
    const int LAYER_HEIGHT = 50;

    SkBitmap bitmap;
    bitmap.allocN32Pixels(WIDTH, HEIGHT);
    SkCanvas canvas(bitmap);

    SkRect bounds = SkRect::MakeWH(SkIntToScalar(LAYER_WIDTH), SkIntToScalar(LAYER_HEIGHT));
    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(WIDTH), SkIntToScalar(HEIGHT)));

    SkIRect devClip;
    // Check that saveLayer without the kClipToLayer_SaveFlag leaves the clip unchanged.
    canvas.saveLayer(SkCanvas::SaveLayerRec(&bounds, nullptr,
            (SkCanvas::SaveLayerFlags) SkCanvasPriv::kDontClipToLayer_SaveLayerFlag));
    devClip = canvas.getDeviceClipBounds();
    REPORTER_ASSERT(reporter, canvas.isClipRect());
    REPORTER_ASSERT(reporter, devClip.width() == WIDTH);
    REPORTER_ASSERT(reporter, devClip.height() == HEIGHT);
    canvas.restore();

    // Check that saveLayer with the kClipToLayer_SaveFlag sets the clip
    // stack to the layer bounds.
    canvas.saveLayer(&bounds, nullptr);
    devClip = canvas.getDeviceClipBounds();
    REPORTER_ASSERT(reporter, canvas.isClipRect());
    REPORTER_ASSERT(reporter, devClip.width() == LAYER_WIDTH);
    REPORTER_ASSERT(reporter, devClip.height() == LAYER_HEIGHT);
    canvas.restore();
}
