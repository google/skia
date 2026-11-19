/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef NearestDecalSubsetTestCases_DEFINED
#define NearestDecalSubsetTestCases_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkAssert.h"
#include "src/shaders/SkImageShader.h"
#include "tests/Test.h"

#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

// Backend-agnostic GPU test cases for nearest-filtered decal image shader subsets. GPU backends
// must clamp texture lookup coordinates to the texel-center-inset subset rect before sampling (so
// no texel outside the subset is ever read) and decide border-vs-texel by comparing the snapped
// texel center against the subset. Shared by Ganesh (tests/GrTextureEffectTest.cpp) and Graphite
// (tests/graphite/ImageShaderTest.cpp): exact-color cases deliberately stay outside the
// [k - 0.001, k) near-integer window where Ganesh's +0.001 snap bias (emulating hardware
// fixed-point rounding on exact subset edges) differs from Graphite's unbiased floor(coord) + 0.5,
// while a dedicated bias-window case checks that neither backend ever leaks out-of-subset texels
// there. These expectations (and SkImageShader::MakeSubset itself) are GPU-specific and must not
// be run against raster backends.
//
// Every border-vs-texel decision edge that is approached from below is kept at a texture
// coordinate <= 2. GPUs whose fragment shaders only have fp16 floats (e.g. Mali-400) can't resolve
// coordinates within ~1/600 texel of larger edges: just below 3 or 4 the +0.001 bias rounds up a
// full fp16 step (1/512) onto the integer, and just below 5 round-to-nearest conversion alone
// lands on 5.0. The magnified and fractional-translation cases therefore use a 3x3 source whose
// single blue center texel [1, 2] x [1, 2] is the subset; it is surrounded by red texels and is its
// own Y-flip, so bottom-left origin sources see the same edges.
namespace skiatest::NearestDecalSubset {

struct Factories {
    // Creates a texturable surface that source content is drawn into. Ganesh uses this to vary the
    // surface origin of the sampled texture.
    std::function<sk_sp<SkSurface>(const SkImageInfo&)> makeSourceSurface;
    // Creates a readable surface that the shader under test is drawn into.
    std::function<sk_sp<SkSurface>(const SkImageInfo&)> makeTargetSurface;
    // Creates an image from a source surface after its content has been drawn. The source surface
    // is not drawn to again after this is called.
    std::function<sk_sp<SkImage>(sk_sp<SkSurface>)> makeImage;
};

namespace detail {

inline SkImageInfo MakeInfo(int w, int h) {
    return SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

inline void DrawSolidRect(SkSurface* surface, const SkRect& rect, SkColor color) {
    SkPaint paint;
    paint.setColor(color);
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawRect(rect, paint);
}

// Builds a `size` x `size` image cleared to red and then filled with each (rect, color) pair in
// order.
inline sk_sp<SkImage> MakeSourceImage(const Factories& factories,
                                      int size,
                                      std::initializer_list<std::pair<SkRect, SkColor>> fills) {
    sk_sp<SkSurface> surface = factories.makeSourceSurface(MakeInfo(size, size));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(SK_ColorRED);
    for (const auto& [rect, color] : fills) {
        DrawSolidRect(surface.get(), rect, color);
    }
    return factories.makeImage(std::move(surface));
}

inline sk_sp<SkShader> MakeNearestSubsetShader(sk_sp<SkImage> image,
                                               const SkRect& subset,
                                               SkTileMode tmx,
                                               SkTileMode tmy,
                                               const SkMatrix* localMatrix) {
    return SkImageShader::MakeSubset(std::move(image),
                                     subset,
                                     tmx,
                                     tmy,
                                     SkSamplingOptions(SkFilterMode::kNearest),
                                     localMatrix);
}

// Draws `shader` over all of `dstSurface` (after translating the canvas by `canvasOffset`), reads
// the result back, and checks every pixel (x, y) against `expectFn`, which may be either an exact
// color `(int x, int y) -> SkColor` or a predicate `(int x, int y, SkColor actual) -> bool`.
template <typename ExpectFn>
void CheckShaderWindow(Reporter* reporter,
                       SkSurface* dstSurface,
                       sk_sp<SkShader> shader,
                       SkPoint canvasOffset,
                       ExpectFn&& expectFn,
                       const char* label) {
    SkCanvas* canvas = dstSurface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    canvas->save();
    canvas->translate(canvasOffset.fX, canvasOffset.fY);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(std::move(shader));
    canvas->drawPaint(paint);
    canvas->restore();

    SkBitmap bm;
    bm.allocPixels(dstSurface->imageInfo());
    if (!dstSurface->readPixels(bm, 0, 0)) {
        ERRORF(reporter, "%s: readPixels failed", label);
        return;
    }
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            const SkColor actual = bm.getColor(x, y);
            if constexpr (std::is_invocable_v<ExpectFn, int, int, SkColor>) {
                REPORTER_ASSERT(reporter,
                                expectFn(x, y, actual),
                                "%s (%d, %d): got unexpected 0x%08x",
                                label,
                                x,
                                y,
                                actual);
            } else {
                const SkColor expected = expectFn(x, y);
                REPORTER_ASSERT(reporter,
                                actual == expected,
                                "%s (%d, %d): got 0x%08x, expected 0x%08x",
                                label,
                                x,
                                y,
                                actual,
                                expected);
            }
        }
    }
}

}  // namespace detail

// Callers must skip protected contexts (readback is unsupported).
inline void Run(Reporter* reporter, const Factories& factories) {
    SkASSERT(factories.makeSourceSurface && factories.makeTargetSurface && factories.makeImage);
    using namespace detail;

    // A 3x3 red image whose single center texel [1, 2] x [1, 2] is blue, so any out-of-subset read
    // produces red. All decision edges are at texture coordinates 1 and 2 for both surface origins
    // (see the comment at the top of this file).
    constexpr SkRect kSubset = {1.f, 1.f, 2.f, 2.f};
    sk_sp<SkImage> srcImage = MakeSourceImage(factories, /*size=*/3, {{kSubset, SK_ColorBLUE}});
    REPORTER_ASSERT(reporter, srcImage);
    if (!srcImage) {
        return;
    }

    // 1. Fractional translations into a 4x4 target:
    // (a) +/-0.0015 sits outside Ganesh's +0.001 near-integer snap bias window while staying inside
    //     8-bit fixed-point hardware rounding (half of the 1/256 LSB, i.e. 1/512 ~= 0.00195):
    //     - 0.5015 places local coords at k - 0.0015, mapping the subset onto pixel 2. For top-left
    //       sources pixel 2 samples 1.9985, which 8-bit rounding samplers would snap onto red
    //       texel 2 without coordinate clamping.
    //     - 0.4985 places local coords at k + 0.0015, mapping the subset onto pixel 1. For
    //       bottom-left sources pixel 1 samples texture-space 1.9985 (3 - 1.0015), with the same
    //       leak.
    // (b) +/-0.0005 sits inside Ganesh's +0.001 bias window (e.g. 0.9995 < 1.0 is treated as
    //     inside by Ganesh's border test, while samplers with more than ~10 bits of sub-texel
    //     precision, or that truncate rather than round, would read red texel 0 without coordinate
    //     clamping). Ganesh and Graphite intentionally disagree on which of pixels 1 and 2 shows
    //     the texel along each axis, so exactly one pixel in [1, 2] x [1, 2] must be blue and every
    //     other pixel must be transparent.
    if (sk_sp<SkSurface> dst4x4 = factories.makeTargetSurface(MakeInfo(4, 4))) {
        for (float offset : {0.5015f, 0.4985f}) {
            const int inside = offset > 0.5f ? 2 : 1;
            const SkMatrix localMatrix = SkMatrix::Translate(offset, offset);
            SkString label = SkStringPrintf("FracTranslate %.4f", offset);
            CheckShaderWindow(
                    reporter,
                    dst4x4.get(),
                    MakeNearestSubsetShader(srcImage,
                                            kSubset,
                                            SkTileMode::kDecal,
                                            SkTileMode::kDecal,
                                            &localMatrix),
                    /*canvasOffset=*/{0.f, 0.f},
                    [inside](int x, int y) {
                        return (x == inside && y == inside) ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    label.c_str());
        }

        for (float offset : {0.5005f, 0.4995f}) {
            const SkMatrix localMatrix = SkMatrix::Translate(offset, offset);
            SkString label = SkStringPrintf("FracTranslateBiasWindow %.4f", offset);
            int blueCount = 0;
            CheckShaderWindow(
                    reporter,
                    dst4x4.get(),
                    MakeNearestSubsetShader(srcImage,
                                            kSubset,
                                            SkTileMode::kDecal,
                                            SkTileMode::kDecal,
                                            &localMatrix),
                    /*canvasOffset=*/{0.f, 0.f},
                    [&blueCount](int x, int y, SkColor actual) {
                        const bool candidate = x >= 1 && x <= 2 && y >= 1 && y <= 2;
                        if (candidate && actual == SK_ColorBLUE) {
                            ++blueCount;
                            return true;
                        }
                        return actual == SK_ColorTRANSPARENT;
                    },
                    label.c_str());
            REPORTER_ASSERT(reporter,
                            blueCount == 1,
                            "%s: expected exactly 1 blue pixel, got %d",
                            label.c_str(),
                            blueCount);
        }
    }

    // 2. High-magnification (300x) nearest upscale around both the top-left (300, 300) and
    // bottom-right (600, 600) subset corners, across Decal paired with Decal, Clamp, Repeat, and
    // Mirror. With 8 bits of sub-texel precision, 300x places the outermost inside fragment center
    // at 2 - 0.5/300 (~= 1.99833), outside Ganesh's 0.001 snap bias window yet within 1/600 texel
    // (< 1/256 sampler LSB, and < 1/512 round-to-nearest threshold) of the subset edge, where
    // hardware fixed-point conversion rounds onto the next texel unless clamped.
    if (sk_sp<SkSurface> winSurface = factories.makeTargetSurface(MakeInfo(16, 16))) {
        constexpr std::pair<SkTileMode, SkTileMode> kTileCombos[] = {
                {SkTileMode::kDecal, SkTileMode::kDecal},
                {SkTileMode::kDecal, SkTileMode::kClamp},
                {SkTileMode::kClamp, SkTileMode::kDecal},
                {SkTileMode::kDecal, SkTileMode::kRepeat},
                {SkTileMode::kRepeat, SkTileMode::kDecal},
                {SkTileMode::kDecal, SkTileMode::kMirror},
                {SkTileMode::kMirror, SkTileMode::kDecal},
        };
        constexpr float kScale = 300.f;
        const SkMatrix localMatrix = SkMatrix::Scale(kScale, kScale);
        for (auto [tmx, tmy] : kTileCombos) {
            sk_sp<SkShader> shader =
                    MakeNearestSubsetShader(srcImage, kSubset, tmx, tmy, &localMatrix);

            // Inspect 16x16 windows centered on the scaled subset's top-left (300, 300) and
            // bottom-right (600, 600) corners.
            for (bool bottomRight : {false, true}) {
                const float edge = bottomRight ? 2.f * kScale : kScale;
                SkString label = SkStringPrintf("scale=%.0f tm=(%d,%d) br=%d",
                                                kScale,
                                                static_cast<int>(tmx),
                                                static_cast<int>(tmy),
                                                bottomRight);
                CheckShaderWindow(
                        reporter,
                        winSurface.get(),
                        shader,
                        /*canvasOffset=*/{-(edge - 8.f), -(edge - 8.f)},
                        [bottomRight, tmx, tmy](int x, int y) {
                            const bool inX = bottomRight ? (x < 8) : (x >= 8);
                            const bool inY = bottomRight ? (y < 8) : (y >= 8);
                            const bool outX = (tmx == SkTileMode::kDecal) && !inX;
                            const bool outY = (tmy == SkTileMode::kDecal) && !inY;
                            return (outX || outY) ? SK_ColorTRANSPARENT : SK_ColorBLUE;
                        },
                        label.c_str());
            }
        }
    }

    // 3. Fractional subsets: on GPU (both Ganesh and Graphite), the clamp uses
    // floor/ceil(subset) while the border test compares the snapped texel center against the
    // fractional subset bounds (an intentional GPU behavior that includes whole texels iff
    // their center lies inside the subset). These draw at 1:1 so every sample is at a texel center
    // (k + 0.5), far from any decision edge. Build a 6x6 image with outer ring [0..6] red, middle
    // ring [1..5] green (texel centers 1.5 and 4.5), and inner [2..4] blue (texel centers 2.5 and
    // 3.5).
    {
        sk_sp<SkImage> ringedImage =
                MakeSourceImage(factories,
                                /*size=*/6,
                                {{SkRect{1.f, 1.f, 5.f, 5.f}, SK_ColorGREEN},
                                 {SkRect{2.f, 2.f, 4.f, 4.f}, SK_ColorBLUE}});
        sk_sp<SkSurface> dst6x6 = factories.makeTargetSurface(MakeInfo(6, 6));
        if (ringedImage && dst6x6) {
            auto ringColor = [](int x, int y) {
                return (x >= 2 && x <= 3 && y >= 2 && y <= 3) ? SK_ColorBLUE : SK_ColorGREEN;
            };
            auto checkFracSubset = [&](const SkRect& subset,
                                       int firstX,
                                       int lastX,
                                       int firstY,
                                       int lastY,
                                       const char* label) {
                CheckShaderWindow(
                        reporter,
                        dst6x6.get(),
                        MakeNearestSubsetShader(ringedImage,
                                                subset,
                                                SkTileMode::kDecal,
                                                SkTileMode::kDecal,
                                                /*localMatrix=*/nullptr),
                        /*canvasOffset=*/{0.f, 0.f},
                        [&](int x, int y) {
                            return (x >= firstX && x <= lastX && y >= firstY && y <= lastY)
                                           ? ringColor(x, y)
                                           : SK_ColorTRANSPARENT;
                        },
                        label);
            };

            // (a) [1.3, 1.3, 4.7, 4.7] includes texel centers 1.5 and 4.5.
            checkFracSubset({1.3f, 1.3f, 4.7f, 4.7f}, 1, 4, 1, 4, "WideFrac");

            // (b) [1.6, 1.6, 4.4, 4.4] excludes texel centers 1.5 and 4.5.
            checkFracSubset({1.6f, 1.6f, 4.4f, 4.4f}, 2, 3, 2, 3, "NarrowFrac");

            // (c) Asymmetric [1.3, 1.6, 4.4, 4.7]: includes x in [1..3] (excludes 4.5 > 4.4)
            // and y in [2..4] (excludes 1.5 < 1.6, includes 4.5 <= 4.7), verifying that
            // asymmetric fractional Y bounds are flipped accurately for bottom-left origins.
            checkFracSubset({1.3f, 1.6f, 4.4f, 4.7f}, 1, 3, 2, 4, "AsymFrac");
        }
    }

    // 4. Sub-texel subsets (< 1 texel wide/tall) of the 3x3 source: [1.2, 1.2, 1.8, 1.8] collapses
    // the clamp rect to the single texel center (1.5, 1.5) while surrounded on all sides by red
    // texels. Because GPU decal compares the snapped texel center (1.5) against [1.2, 1.8], texel 1
    // fills the entire [300, 600) device range at 300x.
    {
        sk_sp<SkSurface> winSurface = factories.makeTargetSurface(MakeInfo(16, 16));
        sk_sp<SkSurface> dst3x3 = factories.makeTargetSurface(MakeInfo(3, 3));
        if (winSurface && dst3x3) {
            constexpr SkRect kSubTexelSubset = {1.2f, 1.2f, 1.8f, 1.8f};
            const SkMatrix scale300 = SkMatrix::Scale(300.f, 300.f);
            sk_sp<SkShader> subTexelShader = MakeNearestSubsetShader(
                    srcImage, kSubTexelSubset, SkTileMode::kDecal, SkTileMode::kDecal, &scale300);

            // Check both the top-left (300, 300) and bottom-right (600, 600) corners of the
            // magnified texel [300, 600) x [300, 600).
            CheckShaderWindow(
                    reporter,
                    winSurface.get(),
                    subTexelShader,
                    /*canvasOffset=*/{-(300.f - 8.f), -(300.f - 8.f)},
                    [](int x, int y) {
                        return (x >= 8 && y >= 8) ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    "SubTexel 300x TopLeft");
            CheckShaderWindow(
                    reporter,
                    winSurface.get(),
                    std::move(subTexelShader),
                    /*canvasOffset=*/{-(600.f - 8.f), -(600.f - 8.f)},
                    [](int x, int y) {
                        return (x < 8 && y < 8) ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    "SubTexel 300x BottomRight");

            // [1.6, 1.6, 2.4, 2.4] straddles a texel boundary without containing any texel
            // center (1.5 < 1.6 and 2.5 > 2.4): every pixel must resolve to border.
            CheckShaderWindow(reporter,
                              dst3x3.get(),
                              MakeNearestSubsetShader(srcImage,
                                                      {1.6f, 1.6f, 2.4f, 2.4f},
                                                      SkTileMode::kDecal,
                                                      SkTileMode::kDecal,
                                                      /*localMatrix=*/nullptr),
                              /*canvasOffset=*/{0.f, 0.f},
                              [](int, int) { return SK_ColorTRANSPARENT; },
                              "EmptyCenterSubTexel");
        }
    }
}

}  // namespace skiatest::NearestDecalSubset

#endif  // NearestDecalSubsetTestCases_DEFINED
