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
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkSamplingPriv.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
struct GrContextOptions;
#endif

#include <atomic>
#include <functional>
#include <initializer_list>

extern int gOverrideMaxTextureSize;
extern std::atomic<int> gNumTilesDrawn;

namespace {

constexpr bool kWriteOutImages = false;

// Create a bitmap w/ a white border around the edge (to test strict constraints) and
// a Hilbert curve inside of that (so the effects of (mis) sampling are evident).
sk_sp<SkImage> make_big_bitmap_image(int imgSize, int whiteBandWidth,
                                     int desiredLineWidth, int desiredDepth) {
    const int kPad = desiredLineWidth;

    SkBitmap bm;

    bm.allocN32Pixels(imgSize, imgSize, /* isOpaque= */ true);

    bm.eraseColor(SK_ColorWHITE);
    bm.erase(SK_ColorDKGRAY, { whiteBandWidth, whiteBandWidth,
                               imgSize-whiteBandWidth, imgSize-whiteBandWidth });

    SkCanvas canvas(bm);
    int desiredDrawSize = imgSize - 2 * kPad - 2 * whiteBandWidth;
    ToolUtils::HilbertGenerator gen(desiredDrawSize, desiredLineWidth, desiredDepth);

    canvas.translate(kPad + whiteBandWidth, imgSize - kPad - whiteBandWidth);
    gen.draw(&canvas);

    bm.setImmutable();
    return bm.asImage();
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

const char* get_constraint_name(SkCanvas::SrcRectConstraint constraint) {
    switch (constraint) {
        case SkCanvas::kFast_SrcRectConstraint:   return "fast";
        case SkCanvas::kStrict_SrcRectConstraint: return "strict";
    }

    SkUNREACHABLE;
}

void potentially_write_to_png(const char* directory, const SkSamplingOptions& sampling,
                              int scale, int rot, SkCanvas::SrcRectConstraint constraint,
                              const SkBitmap& bm) {
    if constexpr(kWriteOutImages) {
        SkString filename;
        filename.appendf("//%s//%s-%d-%d-%s.png",
                         directory,
                         get_sampling_str(sampling),
                         scale,
                         rot,
                         get_constraint_name(constraint));

        SkFILEWStream file(filename.c_str());
        SkAssertResult(file.isValid());

        SkAssertResult(SkPngEncoder::Encode(&file, bm.pixmap(), {}));
    }
}

bool check_pixels(skiatest::Reporter* reporter,
                  const SkBitmap& expected,
                  const SkBitmap& actual,
                  const SkSamplingOptions& sampling,
                  int scale,
                  int rot,
                  SkCanvas::SrcRectConstraint constraint) {
    static const float kTols[4]    = { 0.004f, 0.004f, 0.004f, 0.004f };   // ~ 1/255
    static const float kRotTols[4] = { 0.024f, 0.024f, 0.024f, 0.024f };   // ~ 6/255

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "%s %d %d %s: mismatch at %d, %d (%f, %f, %f %f)",
                       get_sampling_str(sampling), scale, rot,
                       get_constraint_name(constraint),
                       x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });

    return ComparePixels(expected.pixmap(), actual.pixmap(), rot ? kRotTols : kTols, error);
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

} // anonymous namespace


#if defined(SK_GANESH)

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BigImageTest_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    // We're using the knowledge that the internal tile size is 1024. By creating kImageSize
    // sized images we know we'll get a 4x4 tiling regardless of the sampling.
    static const int kImageSize = 4096 - 4 * 2 * kBicubicFilterTexelPad;
    static const int kOverrideMaxTextureSize = 1024;
    static const int kExpectedNumTiles = 4*4;

    if (dContext->maxTextureSize() < kImageSize) {
        // For the expected images we need to be able to draw w/o tiling
        return;
    }

    static const int kWhiteBandWidth = 4;
    sk_sp<SkImage> img = make_big_bitmap_image(kImageSize,
                                               kWhiteBandWidth,
                                               /* desiredLineWidth= */ 16,
                                               /* desiredDepth= */ 7);
    const SkRect srcRect = SkRect::MakeIWH(img->width(), img->height()).makeInset(kWhiteBandWidth,
                                                                                  kWhiteBandWidth);

    const SkSamplingOptions kSamplingOptions[] = {
        SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
        // Note that Mipmapping gets auto-disabled with a strict-constraint
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
        SkSamplingOptions(SkCubicResampler::CatmullRom()),
    };

    for (auto sampling : kSamplingOptions) {
        for (int scale : { 1, 4, 8 }) {
            for (int rot : { 0, 45 }) {
                for (auto constraint : { SkCanvas::kStrict_SrcRectConstraint,
                                         SkCanvas::kFast_SrcRectConstraint }) {
                    if (difficult_case(sampling, scale, rot, constraint)) {
                        continue;
                    }
                    SkRect destRect = SkRect::MakeWH(srcRect.width()/scale, srcRect.height()/scale);

                    SkMatrix m = SkMatrix::RotateDeg(rot, destRect.center());
                    SkIRect rotatedRect = m.mapRect(destRect).roundOut();
                    rotatedRect.outset(2, 2);   // outset to capture the constraint's effect

                    auto destII = SkImageInfo::Make(rotatedRect.width(), rotatedRect.height(),
                                                    kRGBA_8888_SkColorType, kPremul_SkAlphaType);

                    SkBitmap expected, actual;
                    expected.allocPixels(destII);
                    actual.allocPixels(destII);

                    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext,
                                                                        skgpu::Budgeted::kNo,
                                                                        destII);
                    SkCanvas* canvas = surface->getCanvas();

                    canvas->translate(-rotatedRect.fLeft, -rotatedRect.fTop);
                    if (sampling.useCubic || sampling.filter != SkFilterMode::kNearest) {
                        // NN sampling doesn't deal well w/ the (0.5, 0.5) offset but the other
                        // sampling modes need it to exercise strict vs. fast constraint
                        // in non-rotated draws
                        canvas->translate(0.5f, 0.5f);
                    }
                    canvas->concat(m);

                    // First, draw w/o tiling
                    gOverrideMaxTextureSize = 0;
                    gNumTilesDrawn.store(0, std::memory_order_relaxed);

                    canvas->clear(SK_ColorBLACK);
                    canvas->drawImageRect(img, srcRect, destRect, sampling, nullptr, constraint);
                    SkAssertResult(surface->readPixels(expected, 0, 0));
                    REPORTER_ASSERT(reporter, gNumTilesDrawn.load(std::memory_order_acquire) == 0);

                    // Then, force 4x4 tiling
                    gOverrideMaxTextureSize = kOverrideMaxTextureSize;

                    canvas->clear(SK_ColorBLACK);
                    canvas->drawImageRect(img, srcRect, destRect, sampling, nullptr, constraint);
                    SkAssertResult(surface->readPixels(actual, 0, 0));
                    REPORTER_ASSERT(reporter,
                                    gNumTilesDrawn.load(std::memory_order_acquire) ==
                                                                                kExpectedNumTiles);

                    REPORTER_ASSERT(reporter, check_pixels(reporter, expected, actual,
                                                           sampling, scale, rot, constraint));

                    potentially_write_to_png("expected", sampling, scale, rot, constraint,
                                             expected);
                    potentially_write_to_png("actual", sampling, scale, rot, constraint,
                                             actual);
                }
            }
        }
    }
}


#endif // SK_GANESH
