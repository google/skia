/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrFixedClip.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

#include <array>
#include <vector>

static void only_allow_software(GrContextOptions* options) {
    options->fGpuPathRenderers = GpuPathRenderers::kNone;
}

static constexpr int kSurfaceSize = 64;

// Concave and volatile, so it goes to the software path renderer without a cached exact-fit
// mask
static SkPath notched_square(float left, float size) {
    float right = left + size;
    return SkPathBuilder()
            .moveTo(left, left)
            .lineTo(right, left)
            .lineTo(right, right)
            .lineTo(left + size / 2, left + size / 2)
            .lineTo(left, right)
            .close()
            .setIsVolatile(true)
            .detach();
}

// Fills the whole approx-fit 64x64 mask texture with full or almost no coverage, then releases
// it so the next smaller mask reuses it with these texels outside its subset
static void draw_stale_mask(GrDirectContext* dContext,
                            skgpu::ganesh::SurfaceDrawContext* sdc,
                            bool fullCoverage) {
    SkPath path = fullCoverage ? notched_square(0, kSurfaceSize)
                               : SkPathBuilder()
                                         .moveTo(0, 0)
                                         .lineTo(0.25f, 0)
                                         .lineTo(0, 0.25f)
                                         .close()
                                         .moveTo(64, 64)
                                         .lineTo(63.75f, 64)
                                         .lineTo(64, 63.75f)
                                         .close()
                                         .setIsVolatile(true)
                                         .detach();
    sdc->drawPath(nullptr,
                  GrPaint(),
                  GrAA::kYes,
                  SkMatrix::I(),
                  path,
                  GrStyle(SkStrokeRec::kFill_InitStyle));
    dContext->flushAndSubmit(GrSyncCpu::kYes);
}

static std::vector<uint32_t> draw_clipped(skiatest::Reporter* reporter,
                                          GrDirectContext* dContext,
                                          skgpu::ganesh::SurfaceDrawContext* sdc,
                                          float scale,
                                          float translate,
                                          int left,
                                          int size) {
    sdc->clear(SK_PMColor4fTRANSPARENT);
    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    SkMatrix viewMatrix = SkMatrix::Scale(scale, scale).preTranslate(translate, translate);
    // The scissor, not the path, defines the mask bounds, so the mask edges do not lie on the
    // float grid of the path coordinates
    GrFixedClip clip(sdc->dimensions(), SkIRect::MakeXYWH(left, left, size, size));
    sdc->drawPath(&clip,
                  std::move(paint),
                  GrAA::kYes,
                  viewMatrix,
                  notched_square((left - 3) / scale - translate, (size + 6) / scale),
                  GrStyle(SkStrokeRec::kFill_InitStyle));
    std::vector<uint32_t> pixels(kSurfaceSize * kSurfaceSize);
    GrImageInfo info(
            GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr, kSurfaceSize, kSurfaceSize);
    bool ok = sdc->readPixels(dContext, GrPixmap(info, pixels.data(), kSurfaceSize * 4), {0, 0});
    REPORTER_ASSERT(reporter, ok);
    return pixels;
}

// Contract: software path masks use approx-fit textures whose texels outside the mask hold stale
// data from prior draws. Even when a large view translation cancelled by path coords loses float
// precision in mask-space mapping, rendered coverage must depend only on the mask subset itself.
// Zero translation and unit scale serve as precision-preserving controls
DEF_GANESH_TEST_FOR_CONTEXTS(SoftwarePathRenderer_HugeTranslateStaysInMaskSubset,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             only_allow_software,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    if (dContext->supportsProtectedContent()) {
        return;
    }
    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kExact,
                                                       {kSurfaceSize, kSurfaceSize},
                                                       SkSurfaceProps(),
                                                       /*label=*/{});
    if (!sdc) {
        return;
    }

    dContext->freeGpuResources();
    static constexpr std::array<float, 3> kScales = {1.f, 1.25f, 1.5f};
    static constexpr std::array<float, 3> kTranslates = {0.f, 1e7f, 3e7f};
    for (float scale : kScales) {
        for (float translate : kTranslates) {
            for (int left = 1; left <= 8; ++left) {
                for (int size = 33; size <= 40; ++size) {
                    std::array<std::vector<uint32_t>, 2> results;
                    for (int stale = 0; stale < 2; ++stale) {
                        draw_stale_mask(dContext, sdc.get(), stale == 1);
                        results[stale] = draw_clipped(
                                reporter, dContext, sdc.get(), scale, translate, left, size);
                    }
                    uint32_t interior = results[0][(left + 1) * kSurfaceSize + (left + 1)];
                    REPORTER_ASSERT(reporter,
                                    interior == 0xFFFFFFFF,
                                    "scale %g, translate %g, left %d, size %d: "
                                    "interior pixel 0x%08x != 0xFFFFFFFF",
                                    scale,
                                    translate,
                                    left,
                                    size,
                                    interior);
                    int diffs = 0;
                    for (int i = 0; i < kSurfaceSize * kSurfaceSize; ++i) {
                        diffs += results[0][i] != results[1][i];
                    }
                    REPORTER_ASSERT(reporter,
                                    diffs == 0,
                                    "scale %g, translate %g, left %d, size %d: "
                                    "%d pixels depend on stale texels",
                                    scale,
                                    translate,
                                    left,
                                    size,
                                    diffs);
                }
            }
        }
    }
}
