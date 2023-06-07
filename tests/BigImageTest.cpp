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
#include "include/core/SkImageInfo.h"
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

class SkImage;

extern int gOverrideMaxTextureSize;
extern std::atomic<int> gNumTilesDrawn;

namespace {

constexpr bool kWriteOutImages = false;

sk_sp<SkImage> make_big_bitmap_image(int size, int desiredLineWidth, int desiredDepth) {
    SkBitmap bm;

    bm.allocN32Pixels(size, size, /* isOpaque= */ true);

    SkCanvas canvas(bm);

    canvas.clear(SK_ColorDKGRAY);

    ToolUtils::HilbertGenerator gen(/* desiredSize= */ size - 2 * desiredLineWidth,
                                    desiredLineWidth,
                                    desiredDepth);

    canvas.translate(desiredLineWidth, size - desiredLineWidth);
    gen.draw(&canvas);

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

void potentially_write_to_png(const char* directory, const SkSamplingOptions& sampling,
                              int scale, const SkBitmap& bm) {
    if constexpr(kWriteOutImages) {
        SkString filename;
        filename.appendf("//%s//%s-%d.png",
                         directory, get_sampling_str(sampling), scale);

        SkFILEWStream file(filename.c_str());
        SkAssertResult(file.isValid());

        SkAssertResult(SkPngEncoder::Encode(&file, bm.pixmap(), {}));
    }
}

bool check_pixels(skiatest::Reporter* reporter,
                  const SkBitmap& expected,
                  const SkBitmap& actual,
                  const SkSamplingOptions& sampling,
                  int scale) {
    static const float kTols[4] = { 0.004f, 0.004f, 0.004f, 0.004f };

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter, "%s %d: mismatch at %d, %d (%f, %f, %f %f)",
                       get_sampling_str(sampling), scale,
                       x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });

    return ComparePixels(expected.pixmap(), actual.pixmap(), kTols, error);
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

    sk_sp<SkImage> img = make_big_bitmap_image(kImageSize,
                                               /* desiredLineWidth= */ 16,
                                               /* desiredDepth= */ 7);

    const SkSamplingOptions kSamplingOptions[] = {
        SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
        SkSamplingOptions(SkCubicResampler::CatmullRom()),
    };

    for (auto sampling : kSamplingOptions) {
        for (int scale : { 1, 4, 8 }) {

            if (scale > 1 && !sampling.useCubic && (sampling.filter == SkFilterMode::kNearest ||
                                                    sampling.mipmap == SkMipmapMode::kLinear)) {
                // Currently NN and Mipmapping only match up for 1-to-1 draws
                continue;
            }

            auto destII = SkImageInfo::Make(kImageSize/scale, kImageSize/scale,
                                            kRGBA_8888_SkColorType, kPremul_SkAlphaType);

            SkBitmap expected, actual;
            expected.allocPixels(destII);
            actual.allocPixels(destII);

            SkRect destRect = SkRect::MakeWH(destII.width(), destII.height());
            sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo,
                                                                destII);
            SkCanvas* canvas = surface->getCanvas();

            // First, draw w/o tiling
            gOverrideMaxTextureSize = 0;
            gNumTilesDrawn.store(0, std::memory_order_relaxed);

            canvas->drawImageRect(img, destRect, sampling);
            SkAssertResult(surface->readPixels(expected, 0, 0));
            REPORTER_ASSERT(reporter, gNumTilesDrawn.load(std::memory_order_acquire) == 0);

            potentially_write_to_png("expected", sampling, scale, expected);

            // Then, force 4x4 tiling
            gOverrideMaxTextureSize = kOverrideMaxTextureSize;

            canvas->drawImageRect(img, destRect, sampling);
            SkAssertResult(surface->readPixels(actual, 0, 0));
            REPORTER_ASSERT(reporter,
                            gNumTilesDrawn.load(std::memory_order_acquire) == kExpectedNumTiles);

            potentially_write_to_png("actual", sampling, scale, actual);

            REPORTER_ASSERT(reporter, check_pixels(reporter, expected, actual, sampling, scale));
        }
    }
}

#endif // SK_GANESH
