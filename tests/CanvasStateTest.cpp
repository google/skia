/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "include/utils/SkCanvasStateUtils.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkTLazy.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"

#include <cstring>

class SkCanvasState;

// Uncomment to include tests of CanvasState across a library boundary. This will change how 'dm'
// is built so that the functions defined in CanvasStateHelpers do not exist inside 'dm', and are
// instead compiled as part of the 'canvas_state_lib' build target. This produces a shared library
// that must be passed to 'dm' using the --library flag when running.
// #define SK_TEST_CANVAS_STATE_CROSS_LIBRARY

// Must be included after SK_TEST_CANVAS_STATE_CROSS_LIBRARY is defined
#include "tests/CanvasStateHelpers.h"

// dlopen, the library flag and canvas state helpers are only used for tests which require this flag
#if defined(SK_TEST_CANVAS_STATE_CROSS_LIBRARY)

static DEFINE_string(library, "",
                     "Support library to use for CanvasState test. Must be provided when"
                     " SK_TEST_CANVAS_STATE_CROSS_LIBRARY to specify the dynamically loaded library"
                     " that receives the captured canvas state. Functions from the library will be"
                     " called to test SkCanvasState. The library is built from the canvas_state_lib"
                     " target");

#include "src/ports/SkOSLibrary.h"

// Automatically loads library passed to --library flag and closes it when it goes out of scope.
class OpenLibResult {
public:
    OpenLibResult(skiatest::Reporter* reporter) {
        if (FLAGS_library.count() == 1) {
            fLibrary = SkLoadDynamicLibrary(FLAGS_library[0]);
            REPORTER_ASSERT(reporter, fLibrary != nullptr, "Failed to open library!");
        } else {
            fLibrary = nullptr;
        }
    }

    ~OpenLibResult() {
        if (fLibrary) {
            SkFreeDynamicLibrary(fLibrary);
        }
    }

    // Load a function address from the library object, or null if the library had failed
    void* procAddress(const char* funcName) {
        if (fLibrary) {
            return SkGetProcedureAddress(fLibrary, funcName);
        }
        return nullptr;
    }

private:
    void* fLibrary;
};

#endif

static void write_image(const SkImage* img, const char path[]) {
    auto data = img->encodeToData();
    SkFILEWStream(path).write(data->data(), data->size());
}

static void compare(skiatest::Reporter* reporter, SkImage* img0, SkImage* img1) {
    if (false) {
        static int counter;

        SkDebugf("---- counter %d\n", counter);
        SkString name;
        name.printf("no_capture_%d.png", counter);
        write_image(img0, name.c_str());
        name.printf("capture_%d.png", counter);
        write_image(img1, name.c_str());
        counter++;
    }

    SkPixmap pm[2];
    REPORTER_ASSERT(reporter, img0->peekPixels(&pm[0]));
    REPORTER_ASSERT(reporter, img1->peekPixels(&pm[1]));
    // now we memcmp the two bitmaps
    REPORTER_ASSERT(reporter, pm[0].computeByteSize() == pm[1].computeByteSize());
    REPORTER_ASSERT(reporter, pm[0].rowBytes() == (size_t)pm[0].width() * pm[0].info().bytesPerPixel());
    REPORTER_ASSERT(reporter, pm[1].rowBytes() == (size_t)pm[1].width() * pm[1].info().bytesPerPixel());
    if (memcmp(pm[0].addr(0, 0), pm[1].addr(0, 0), pm[0].computeByteSize()) != 0) {
        REPORTER_ASSERT(reporter, false);
    }
}

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

    bool (*drawFn)(SkCanvasState* state, float l, float t,
                   float r, float b, int32_t s);

#if defined(SK_TEST_CANVAS_STATE_CROSS_LIBRARY)
    OpenLibResult openLibResult(reporter);
    *(void**) (&drawFn) = openLibResult.procAddress("complex_layers_draw_from_canvas_state");
#else
    drawFn = complex_layers_draw_from_canvas_state;
#endif

    REPORTER_ASSERT(reporter, drawFn);
    if (!drawFn) {
        return;
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(colorTypes); ++i) {
        sk_sp<SkImage> images[2];
        for (int j = 0; j < 2; ++j) {
            auto surf = SkSurface::MakeRaster(SkImageInfo::Make(WIDTH, HEIGHT,
                                                                colorTypes[i],
                                                                kPremul_SkAlphaType));
            SkCanvas* canvas = surf->getCanvas();

            canvas->drawColor(SK_ColorRED);

            for (size_t k = 0; k < SK_ARRAY_COUNT(layerAlpha); ++k) {
                SkTLazy<SkPaint> paint;
                if (layerAlpha[k] != 0xFF) {
                    paint.init()->setAlpha(layerAlpha[k]);
                }

                // draw a rect within the layer's bounds and again outside the layer's bounds
                canvas->saveLayer(SkCanvas::SaveLayerRec(&rect, paint.getMaybeNull()));

                if (j) {
                    // Capture from the first Skia.
                    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(canvas);
                    REPORTER_ASSERT(reporter, state);

                    // And draw to it in the second Skia.
                    bool success = complex_layers_draw_from_canvas_state(state,
                            rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, SPACER);
                    REPORTER_ASSERT(reporter, success);

                    // And release it in the *first* Skia.
                    SkCanvasStateUtils::ReleaseCanvasState(state);
                } else {
                    // Draw in the first Skia.
                    complex_layers_draw(canvas, rect.fLeft, rect.fTop,
                                        rect.fRight, rect.fBottom, SPACER);
                }

                canvas->restore();

                // translate the canvas for the next iteration
                canvas->translate(0, 2*(rect.height() + SPACER));
            }
            images[j] = surf->makeImageSnapshot();
        }

        compare(reporter, images[0].get(), images[1].get());
    }
}

////////////////////////////////////////////////////////////////////////////////

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
                                     SkRegion::kDifference_Op,
    };

    bool (*drawFn)(SkCanvasState* state, int32_t l, int32_t t,
                   int32_t r, int32_t b, int32_t clipOp,
                   int32_t regionRects, int32_t* rectCoords);

#if defined(SK_TEST_CANVAS_STATE_CROSS_LIBRARY)
    OpenLibResult openLibResult(reporter);
    *(void**) (&drawFn) = openLibResult.procAddress("complex_clips_draw_from_canvas_state");
#else
    drawFn = complex_clips_draw_from_canvas_state;
#endif

    REPORTER_ASSERT(reporter, drawFn);
    if (!drawFn) {
        return;
    }

    sk_sp<SkImage> images[2];
    for (int i = 0; i < 2; ++i) {
        auto surf = SkSurface::MakeRaster(SkImageInfo::MakeN32Premul(WIDTH, HEIGHT));
        SkCanvas* canvas = surf->getCanvas();

        canvas->drawColor(SK_ColorRED);

        SkRegion localRegion = clipRegion;

        SkPaint paint;
        paint.setAlpha(128);
        for (size_t j = 0; j < SK_ARRAY_COUNT(clipOps); ++j) {
            SkRect layerBounds = SkRect::Make(layerRect);
            canvas->saveLayer(SkCanvas::SaveLayerRec(&layerBounds, &paint));

            if (i) {
                SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(canvas);
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
                complex_clips_draw(canvas, clipRect.fLeft, clipRect.fTop,
                                   clipRect.fRight, clipRect.fBottom, clipOps[j],
                                   localRegion);
            }

            canvas->restore();

            // translate the canvas and region for the next iteration
            canvas->translate(0, SkIntToScalar(2*(layerRect.height() + (SPACER))));
            localRegion.translate(0, 2*(layerRect.height() + SPACER));
        }
        images[i] = surf->makeImageSnapshot();
    }

    compare(reporter, images[0].get(), images[1].get());
}

////////////////////////////////////////////////////////////////////////////////

DEF_TEST(CanvasState_test_soft_clips, reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkCanvas canvas(bitmap);

    SkRRect roundRect;
    roundRect.setOval(SkRect::MakeWH(5, 5));

    canvas.clipRRect(roundRect, SkClipOp::kIntersect, true);

    SkCanvasState* state = SkCanvasStateUtils::CaptureCanvasState(&canvas);
    REPORTER_ASSERT(reporter, !state);
}

DEF_TEST(CanvasState_test_saveLayer_clip, reporter) {
    const int WIDTH = 100;
    const int HEIGHT = 100;
    const int LAYER_WIDTH = 50;
    const int LAYER_HEIGHT = 50;

    SkBitmap bitmap;
    bitmap.allocN32Pixels(WIDTH, HEIGHT);
    SkCanvas canvas(bitmap);

    SkRect bounds = SkRect::MakeWH(SkIntToScalar(LAYER_WIDTH), SkIntToScalar(LAYER_HEIGHT));
    canvas.clipRect(SkRect::MakeWH(SkIntToScalar(WIDTH), SkIntToScalar(HEIGHT)));

    // Check that saveLayer sets the clip stack to the layer bounds.
    canvas.saveLayer(&bounds, nullptr);
    SkIRect devClip = canvas.getDeviceClipBounds();
    REPORTER_ASSERT(reporter, canvas.isClipRect());
    REPORTER_ASSERT(reporter, devClip.width() == LAYER_WIDTH);
    REPORTER_ASSERT(reporter, devClip.height() == LAYER_HEIGHT);
    canvas.restore();
}
