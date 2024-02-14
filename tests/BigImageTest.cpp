/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTiledImageUtils.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkSamplingPriv.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "tests/CtsEnforcement.h"
struct GrContextOptions;
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Texture.h"
#include "tools/GpuToolUtils.h"
#else
namespace skgpu { namespace graphite { class Recorder; } }
#endif

#include <atomic>
#include <functional>
#include <initializer_list>
#include <string.h>
#include <utility>

#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
extern int gOverrideMaxTextureSizeGanesh;
extern std::atomic<int> gNumTilesDrawnGanesh;
#endif

#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
extern int gOverrideMaxTextureSizeGraphite;
extern std::atomic<int> gNumTilesDrawnGraphite;
#endif

namespace {

// Draw a white border around the edge (to test strict constraints) and
// a Hilbert curve inside of that (so the effects of (mis) sampling are evident).
 void draw(SkCanvas* canvas, int imgSize, int whiteBandWidth,
           int desiredLineWidth, int desiredDepth) {
    const int kPad = desiredLineWidth;

    canvas->clear(SK_ColorWHITE);

    SkPaint innerRect;
    innerRect.setColor(SK_ColorDKGRAY);
    canvas->drawRect(SkRect::MakeIWH(imgSize, imgSize).makeInset(whiteBandWidth, whiteBandWidth),
                     innerRect);

    int desiredDrawSize = imgSize - 2 * kPad - 2 * whiteBandWidth;
    ToolUtils::HilbertGenerator gen(desiredDrawSize, desiredLineWidth, desiredDepth);

    canvas->translate(kPad + whiteBandWidth, imgSize - kPad - whiteBandWidth);
    gen.draw(canvas);
}


sk_sp<SkImage> make_big_bitmap_image(int imgSize, int whiteBandWidth,
                                     int desiredLineWidth, int desiredDepth) {
    SkBitmap bm;

    bm.allocN32Pixels(imgSize, imgSize, /* isOpaque= */ true);
    SkCanvas canvas(bm);

    draw(&canvas, imgSize, whiteBandWidth, desiredLineWidth, desiredDepth);

    bm.setImmutable();
    return bm.asImage();
}

sk_sp<SkImage> make_big_picture_image(int imgSize, int whiteBandWidth,
                                      int desiredLineWidth, int desiredDepth) {
    sk_sp<SkPicture> pic;

    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeIWH(imgSize, imgSize));
        draw(canvas, imgSize, whiteBandWidth, desiredLineWidth, desiredDepth);
        pic = recorder.finishRecordingAsPicture();
    }

    return SkImages::DeferredFromPicture(std::move(pic),
                                         { imgSize, imgSize },
                                         /* matrix= */ nullptr,
                                         /* paint= */ nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}


const char* get_sampling_str(const SkSamplingOptions& sampling) {
    if (sampling.isAniso()) {
        return "Aniso";
    } else if (sampling.useCubic) {
        return "Cubic";
    } else if (sampling.mipmap != SkMipmapMode::kNone) {
        return "Mipmap";
    } else if (sampling.filter == SkFilterMode::kLinear) {
        return "Linear";
    } else {
        return "NN";
    }
}

SkString create_label(GrDirectContext* dContext,
                      const char* generator,
                      const SkSamplingOptions& sampling,
                      int scale,
                      int rot,
                      SkCanvas::SrcRectConstraint constraint,
                      int numTiles) {
    SkString label;
    label.appendf("%s-%s-%s-%d-%d-%s-%d",
                  dContext ? "ganesh" : "graphite",
                  generator,
                  get_sampling_str(sampling),
                  scale,
                  rot,
                  constraint == SkCanvas::kFast_SrcRectConstraint ? "fast" : "strict",
                  numTiles);
    return label;
 }

void potentially_write_to_png(const char* directory,
                              const SkString& label,
                              const SkBitmap& bm) {
    constexpr bool kWriteOutImages = false;

    if constexpr(kWriteOutImages) {
        SkString filename;
        filename.appendf("//%s//%s.png", directory, label.c_str());

        SkFILEWStream file(filename.c_str());
        SkAssertResult(file.isValid());

        SkAssertResult(SkPngEncoder::Encode(&file, bm.pixmap(), {}));
    }
}

bool check_pixels(skiatest::Reporter* reporter,
                  const SkBitmap& expected,
                  const SkBitmap& actual,
                  const SkString& label,
                  int rot) {
    static const float kTols[4]    = { 0.008f, 0.008f, 0.008f, 0.008f };   // ~ 2/255
    static const float kRotTols[4] = { 0.024f, 0.024f, 0.024f, 0.024f };   // ~ 6/255

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "%s: mismatch at %d, %d (%f, %f, %f %f)",
                       label.c_str(), x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });

    return ComparePixels(expected.pixmap(), actual.pixmap(), rot ? kRotTols : kTols, error);
}

// Return a clip rect that will result in the number of desired tiles being used. The trick
// is that the clip rect also has to work when rotated.
SkRect clip_rect(SkRect dstRect, int numDesiredTiles) {
    dstRect.outset(5, 5);

    switch (numDesiredTiles) {
        case 0:
            return { dstRect.fLeft-64, dstRect.fTop-64, dstRect.fLeft-63, dstRect.fTop-63 };
        case 4: {
            // Upper left 4x4
            float outset = 0.125f * dstRect.width() * SK_ScalarRoot2Over2;
            SkPoint center = dstRect.center();
            return { center.fX - outset, center.fY - outset,
                     center.fX + outset, center.fY + outset };
        }
        case 9: {
            // Upper left 3x3
            float outset = 0.25f * dstRect.width() * SK_ScalarRoot2Over2;
            SkPoint center = dstRect.center();
            center.offset(-dstRect.width()/8.0f, -dstRect.height()/8.0f);
            return { center.fX - outset, center.fY - outset,
                     center.fX + outset, center.fY + outset };
        }
    }

    return dstRect; // all 16 tiles
}

bool difficult_case(const SkSamplingOptions& sampling,
                    int scale,
                    int rot,
                    SkCanvas::SrcRectConstraint constraint) {
    if (sampling.useCubic) {
        return false;  // cubic never causes any issues
    }

    if (constraint == SkCanvas::kStrict_SrcRectConstraint &&
            (sampling.mipmap != SkMipmapMode::kNone || sampling.filter == SkFilterMode::kLinear)) {
        // linear-filtered strict big image drawing is currently broken (b/286239467). The issue
        // is that the strict constraint is propagated to the child tiles which breaks the
        // interpolation expected in the middle of the large image.
        // Note that strict mipmapping is auto-downgraded to strict linear sampling.
        return true;
    }

    if (sampling.mipmap == SkMipmapMode::kLinear) {
        // Mipmapping is broken for anything other that 1-to-1 draws (b/286256104). The issue
        // is that the mipmaps are created for each tile individually so the higher levels differ
        // from what would be generated with the entire image. Mipmapped draws are off by ~20/255
        // at 4x and ~64/255 at 8x)
        return scale > 1;
    }

    if (sampling.filter == SkFilterMode::kNearest) {
        // Perhaps unsurprisingly, NN only passes on un-rotated 1-to-1 draws (off by ~187/255 at
        // different scales).
        return scale > 1 || rot > 0;
    }

    return false;
}

// compare tiled and untiled draws - varying the parameters (e.g., sampling, rotation, fast vs.
// strict, etc).
void tiling_comparison_test(GrDirectContext* dContext,
                            skgpu::graphite::Recorder* recorder,
                            skiatest::Reporter* reporter) {
    // We're using the knowledge that the internal tile size is 1024. By creating kImageSize
    // sized images we know we'll get a 4x4 tiling regardless of the sampling.
    static const int kImageSize = 4096 - 4 * 2 * kBicubicFilterTexelPad;
    static const int kOverrideMaxTextureSize = 1024;

#if defined(SK_GANESH)
    if (dContext && dContext->maxTextureSize() < kImageSize) {
        // For the expected images we need to be able to draw w/o tiling
        return;
    }
#endif

#if defined(SK_GRAPHITE)
    if (recorder) {
        const skgpu::graphite::Caps* caps = recorder->priv().caps();
        if (caps->maxTextureSize() < kImageSize) {
            return;
        }
    }
#endif

    static const int kWhiteBandWidth = 4;
    const SkRect srcRect = SkRect::MakeIWH(kImageSize, kImageSize).makeInset(kWhiteBandWidth,
                                                                             kWhiteBandWidth);

    using GeneratorT = sk_sp<SkImage>(*)(int imgSize, int whiteBandWidth,
                                         int desiredLineWidth, int desiredDepth);

    static const struct {
        GeneratorT fGen;
        const char* fTag;
    } kGenerators[] = { { make_big_bitmap_image,  "BM" },
                        { make_big_picture_image, "Picture" } };

    static const SkSamplingOptions kSamplingOptions[] = {
        SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
        // Note that Mipmapping gets auto-disabled with a strict-constraint
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
        SkSamplingOptions(SkCubicResampler::CatmullRom()),
    };

    int numClippedTiles = 9;
    for (auto gen : kGenerators) {
        if (recorder && !strcmp(gen.fTag, "Picture")) {
            // In the picture-image case, the non-tiled code path draws the picture directly into a
            // gpu-backed surface while the tiled code path the picture is draws the picture into
            // a raster-backed surface. For Ganesh this works out, since both Ganesh and Raster
            // support non-AA rect draws. For Graphite the results are very different (since
            // Graphite always anti-aliases. Forcing all the rect draws to be AA doesn't work out
            // since AA introduces too much variance between both of the gpu backends and Raster -
            // which would obscure any errors introduced by tiling.
            continue;
        }

        sk_sp<SkImage> img = (*gen.fGen)(kImageSize,
                                         kWhiteBandWidth,
                                         /* desiredLineWidth= */ 16,
                                         /* desiredDepth= */ 7);
        numClippedTiles = (numClippedTiles == 9) ? 4 : 9;  // alternate to reduce the combinatorics

        for (int scale : { 1, 4, 8 }) {
            for (int rot : { 0, 45 }) {
                for (int numDesiredTiles : { numClippedTiles, 16 }) {
                    SkRect destRect = SkRect::MakeWH(srcRect.width()/scale,
                                                     srcRect.height()/scale);

                    SkMatrix m = SkMatrix::RotateDeg(rot, destRect.center());
                    SkIRect rotatedRect = m.mapRect(destRect).roundOut();
                    rotatedRect.outset(2, 2);   // outset to capture the constraint's effect

                    SkRect clipRect = clip_rect(destRect, numDesiredTiles);

                    auto destII = SkImageInfo::Make(rotatedRect.width(),
                                                    rotatedRect.height(),
                                                    kRGBA_8888_SkColorType,
                                                    kPremul_SkAlphaType);

                    SkBitmap expected, actual;
                    expected.allocPixels(destII);
                    actual.allocPixels(destII);

                    sk_sp<SkSurface> surface;

#if defined(SK_GANESH)
                    if (dContext) {
                        surface = SkSurfaces::RenderTarget(dContext,
                                                           skgpu::Budgeted::kNo,
                                                           destII);
                    }
#endif

#if defined(SK_GRAPHITE)
                    if (recorder) {
                        surface = SkSurfaces::RenderTarget(recorder, destII);
                    }
#endif

                    for (auto sampling : kSamplingOptions) {
                        for (auto constraint : { SkCanvas::kStrict_SrcRectConstraint,
                                                 SkCanvas::kFast_SrcRectConstraint }) {
                            if (difficult_case(sampling, scale, rot, constraint)) {
                                continue;
                            }

                            SkString label = create_label(dContext, gen.fTag, sampling, scale, rot,
                                                          constraint, numDesiredTiles);

                            SkCanvas* canvas = surface->getCanvas();

                            SkAutoCanvasRestore acr(canvas, /* doSave= */ true);

                            canvas->translate(-rotatedRect.fLeft, -rotatedRect.fTop);
                            if (sampling.useCubic || sampling.filter != SkFilterMode::kNearest) {
                                // NN sampling doesn't deal well w/ the (0.5, 0.5) offset but the
                                // other sampling modes need it to exercise strict vs. fast
                                // constraint in non-rotated draws
                                canvas->translate(0.5f, 0.5f);
                            }
                            canvas->concat(m);

                            // First, draw w/o tiling
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
                            gOverrideMaxTextureSizeGanesh = 0;
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
                            gOverrideMaxTextureSizeGraphite = 0;
#endif
                            canvas->clear(SK_ColorBLACK);
                            canvas->save();
                            canvas->clipRect(clipRect);

                            SkTiledImageUtils::DrawImageRect(canvas, img, srcRect, destRect,
                                                             sampling, /* paint= */ nullptr,
                                                             constraint);
                            SkAssertResult(surface->readPixels(expected, 0, 0));
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
                            int actualNumTiles =
                                    gNumTilesDrawnGanesh.load(std::memory_order_acquire);
                            REPORTER_ASSERT(reporter, actualNumTiles == 0);
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
                            int actualNumTiles2 =
                                    gNumTilesDrawnGraphite.load(std::memory_order_acquire);
                            REPORTER_ASSERT(reporter, actualNumTiles2 == 0);
#endif
                            canvas->restore();

                            // Then, force 4x4 tiling
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
                            gOverrideMaxTextureSizeGanesh = kOverrideMaxTextureSize;
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
                            gOverrideMaxTextureSizeGraphite = kOverrideMaxTextureSize;
#endif

                            canvas->clear(SK_ColorBLACK);
                            canvas->save();
                            canvas->clipRect(clipRect);

                            SkTiledImageUtils::DrawImageRect(canvas, img, srcRect, destRect,
                                                             sampling, /* paint= */ nullptr,
                                                             constraint);
                            SkAssertResult(surface->readPixels(actual, 0, 0));
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
                            if (canvas->recordingContext()) {
                                actualNumTiles =
                                        gNumTilesDrawnGanesh.load(std::memory_order_acquire);
                                REPORTER_ASSERT(reporter,
                                                numDesiredTiles == actualNumTiles,
                                                "mismatch expected: %d actual: %d\n",
                                                numDesiredTiles,
                                                actualNumTiles);
                            }
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
                            if (canvas->recorder()) {
                                actualNumTiles2 =
                                        gNumTilesDrawnGraphite.load(std::memory_order_acquire);
                                REPORTER_ASSERT(reporter,
                                                numDesiredTiles == actualNumTiles2,
                                                "mismatch expected: %d actual: %d\n",
                                                numDesiredTiles,
                                                actualNumTiles2);
                            }
#endif

                            canvas->restore();

                            REPORTER_ASSERT(reporter, check_pixels(reporter, expected, actual,
                                                                   label, rot));

                            potentially_write_to_png("expected", label, expected);
                            potentially_write_to_png("actual", label, actual);
                        }
                    }
                }
            }
        }
    }
    // Reset tiling behavior
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
    gOverrideMaxTextureSizeGanesh = 0;
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
    gOverrideMaxTextureSizeGraphite = 0;
#endif
}

// In this test we draw the same bitmap-backed image twice and check that we only upload it once.
// Everything is set up for the bitmap-backed image to be split into 16 1024x1024 tiles.
void tiled_image_caching_test(GrDirectContext* dContext,
                              skgpu::graphite::Recorder* recorder,
                              skiatest::Reporter* reporter) {
    static const int kImageSize = 4096;
    static const int kOverrideMaxTextureSize = 1024;
    static const SkISize kExpectedTileSize { kOverrideMaxTextureSize, kOverrideMaxTextureSize };

    sk_sp<SkImage> img = make_big_bitmap_image(kImageSize,
                                               /* whiteBandWidth= */ 0,
                                               /* desiredLineWidth= */ 16,
                                               /* desiredDepth= */ 7);

    auto destII = SkImageInfo::Make(kImageSize, kImageSize,
                                    kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType);

    SkBitmap readback;
    readback.allocPixels(destII);

    sk_sp<SkSurface> surface;

#if defined(SK_GANESH)
    if (dContext) {
        surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, destII);
    }
#endif

#if defined(SK_GRAPHITE)
    if (recorder) {
        surface = SkSurfaces::RenderTarget(recorder, destII);
    }
#endif

    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();

#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
    gOverrideMaxTextureSizeGanesh = kOverrideMaxTextureSize;
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
    gOverrideMaxTextureSizeGraphite = kOverrideMaxTextureSize;
#endif
    for (int i = 0; i < 2; ++i) {
        canvas->clear(SK_ColorBLACK);

        SkTiledImageUtils::DrawImage(canvas, img,
                                     /* x= */ 0, /* y= */ 0,
                                     SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
                                     /* paint= */ nullptr,
                                     SkCanvas::kFast_SrcRectConstraint);
        SkAssertResult(surface->readPixels(readback, 0, 0));
    }

    int numFound = 0;

#if defined(SK_GANESH)
    if (dContext) {
        GrResourceCache* cache = dContext->priv().getResourceCache();

        cache->visitSurfaces([&](const GrSurface* surf, bool /* purgeable */) {
            const GrTexture* tex = surf->asTexture();
            if (tex && tex->dimensions() == kExpectedTileSize) {
                ++numFound;
            }
        });
    }
#endif

#if defined(SK_GRAPHITE)
    if (recorder) {
        skgpu::graphite::ResourceCache* cache = recorder->priv().resourceCache();

        cache->visitTextures([&](const skgpu::graphite::Texture* tex, bool /* purgeable */) {
            if (tex->dimensions() == kExpectedTileSize) {
                ++numFound;
            }
        });
    }
#endif

    REPORTER_ASSERT(reporter, numFound == 16, "Expected: 16 Actual: %d", numFound);

    // reset to default behavior
#if defined(SK_GANESH) && defined(GR_TEST_UTILS)
    gOverrideMaxTextureSizeGanesh = 0;
#endif
#if defined(SK_GRAPHITE) && defined(GRAPHITE_TEST_UTILS)
    gOverrideMaxTextureSizeGraphite = 0;
#endif
}

} // anonymous namespace

#if defined(SK_GANESH)

// TODO(b/306005622): fix in SkQP and move to CtsEnforcement::kNextRelease
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BigImageTest_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    tiling_comparison_test(dContext, /* recorder= */ nullptr, reporter);
}

// TODO(b/306005622): fix in SkQP and move to CtsEnforcement::kNextRelease
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(TiledDrawCacheTest_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    tiled_image_caching_test(dContext, /* recorder= */ nullptr, reporter);
}

#endif // SK_GANESH

#if defined(SK_GRAPHITE)

// TODO(b/306005622): fix in SkQP and move to CtsEnforcement::kNextRelease
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BigImageTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    tiling_comparison_test(/* dContext= */ nullptr, recorder.get(), reporter);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(TiledDrawCacheTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    tiled_image_caching_test(/* dContext= */ nullptr, recorder.get(), reporter);
}

#endif // SK_GRAPHITE
