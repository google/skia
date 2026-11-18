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

// Builds a 6x6 image cleared to red and then filled with each (rect, color) pair in order.
inline sk_sp<SkImage> MakeSourceImage(const Factories& factories,
                                      std::initializer_list<std::pair<SkRect, SkColor>> fills) {
    sk_sp<SkSurface> surface = factories.makeSourceSurface(MakeInfo(6, 6));
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

    constexpr SkRect kSubset = {1.f, 1.f, 5.f, 5.f};

    // Surround the [1, 1, 5, 5] blue subset with a 1px red border so any out-of-subset read
    // produces red.
    sk_sp<SkImage> srcImage = MakeSourceImage(factories, {{kSubset, SK_ColorBLUE}});
    REPORTER_ASSERT(reporter, srcImage);
    if (!srcImage) {
        return;
    }

    // 1. Fractional translations into an 8x8 target:
    // (a) ±0.0015 sits outside Ganesh's +0.001 near-integer snap bias window while staying inside
    //     8-bit fixed-point hardware rounding (1/512 ≈ 0.00195):
    //     - 0.5015 places local coords at k - 0.0015 (texture-space coords just below integers for
    //       top-left sources and just above for bottom-left sources), mapping [1, 5] onto [2, 5].
    //     - 0.4985 places local coords at k + 0.0015 (texture-space coords just above integers for
    //       top-left sources and just below for bottom-left sources), mapping [1, 5] onto [1, 4].
    // (b) ±0.0005 sits inside Ganesh's +0.001 bias window (e.g. texture-space 0.9995 < 1.0 is
    //     treated as inside by Ganesh's border test while samplers with more than ~10 bits of
    //     sub-texel precision, or that truncate rather than round, would read red texel 0 without
    //     coordinate clamping). Because +0.001 shifts both integer subset edges (0.9995 at index 1
    //     and 4.9995 at index 5, mapping [1, 5] to [1, 4] vs. Graphite's [2, 5]), the 1px boundary
    //     ring of [1, 5] x [1, 5] (for 0.4995 only the Y boundary rows, since X is at k + 0.0005)
    //     may be blue or transparent depending on backend and origin, while the remaining interior
    //     must be blue and everything outside must be transparent.
    if (sk_sp<SkSurface> dst8x8 = factories.makeTargetSurface(MakeInfo(8, 8))) {
        for (float offset : {0.5015f, 0.4985f}) {
            const int firstInside = offset > 0.5f ? 2 : 1;
            const SkMatrix localMatrix = SkMatrix::Translate(offset, offset);
            SkString label = SkStringPrintf("FracTranslate %.4f", offset);
            CheckShaderWindow(
                    reporter,
                    dst8x8.get(),
                    MakeNearestSubsetShader(
                            srcImage, kSubset, SkTileMode::kDecal, SkTileMode::kDecal, &localMatrix),
                    /*canvasOffset=*/{0.f, 0.f},
                    [firstInside](int x, int y) {
                        const bool inside = x >= firstInside && x <= firstInside + 3 &&
                                            y >= firstInside && y <= firstInside + 3;
                        return inside ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    label.c_str());
        }

        for (float offset : {0.5005f, 0.4995f}) {
            const SkMatrix localMatrix = SkMatrix::Translate(offset, offset);
            SkString label = SkStringPrintf("FracTranslateBiasWindow %.4f", offset);
            CheckShaderWindow(
                    reporter,
                    dst8x8.get(),
                    MakeNearestSubsetShader(
                            srcImage, kSubset, SkTileMode::kDecal, SkTileMode::kDecal, &localMatrix),
                    /*canvasOffset=*/{0.f, 0.f},
                    [offset](int x, int y, SkColor actual) {
                        // X is only in the [k - 0.001, k) bias window for 0.5005; for 0.4995 local
                        // X is k + 0.0005, so both backends agree X is inside [1, 4]. Y can hit
                        // the bias window for either offset (top-left on 0.5005, bottom-left on
                        // 0.4995).
                        const int minX = 1;
                        const int maxX = offset > 0.5f ? 5 : 4;
                        if (x < minX || x > maxX || y < 1 || y > 5) {
                            return actual == SK_ColorTRANSPARENT;
                        }
                        const int strictMinX = offset > 0.5f ? 2 : 1;
                        if (x >= strictMinX && x <= 4 && y >= 2 && y <= 4) {
                            return actual == SK_ColorBLUE;
                        }
                        return actual == SK_ColorBLUE || actual == SK_ColorTRANSPARENT;
                    },
                    label.c_str());
        }
    }

    // 2. High-magnification (300x) nearest upscale around both the top-left (300, 300) and
    // bottom-right (1500, 1500) subset corners, across Decal paired with Decal, Clamp, Repeat, and
    // Mirror. With 8 bits of sub-texel precision, 300x places the outermost fragment center at
    // 5 - 0.5/300 (≈ 4.99833), outside Ganesh's 0.001 snap bias window yet within 1/600 texel
    // (< 1/256 sampler LSB, and < 1/512 round-to-nearest threshold) of the subset edge, where
    // hardware fixed-point conversion rounds onto the next texel unless clamped.
    if (sk_sp<SkSurface> winSurface = factories.makeTargetSurface(MakeInfo(16, 16))) {
        constexpr std::pair<SkTileMode, SkTileMode> kTileCombos[] = {
                {SkTileMode::kDecal,  SkTileMode::kDecal},
                {SkTileMode::kDecal,  SkTileMode::kClamp},
                {SkTileMode::kClamp,  SkTileMode::kDecal},
                {SkTileMode::kDecal,  SkTileMode::kRepeat},
                {SkTileMode::kRepeat, SkTileMode::kDecal},
                {SkTileMode::kDecal,  SkTileMode::kMirror},
                {SkTileMode::kMirror, SkTileMode::kDecal},
        };
        constexpr float kScale = 300.f;
        const SkMatrix localMatrix = SkMatrix::Scale(kScale, kScale);
        for (auto [tmx, tmy] : kTileCombos) {
            sk_sp<SkShader> shader =
                    MakeNearestSubsetShader(srcImage, kSubset, tmx, tmy, &localMatrix);

            // Inspect 16x16 windows centered on the scaled subset's top-left (300, 300) and
            // bottom-right (1500, 1500) corners.
            for (bool bottomRight : {false, true}) {
                const float edge = bottomRight ? 5.f * kScale : kScale;
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
    // their center lies inside the subset). Build a 6x6 image with outer ring [0..6] red,
    // middle ring [1..5] green (texel centers 1.5 and 4.5), and inner [2..4] blue (texel
    // centers 2.5 and 3.5).
    {
        sk_sp<SkImage> ringedImage =
                MakeSourceImage(factories,
                                {{SkRect{1.f, 1.f, 5.f, 5.f}, SK_ColorGREEN},
                                 {SkRect{2.f, 2.f, 4.f, 4.f}, SK_ColorBLUE}});
        sk_sp<SkSurface> dst6x6 = factories.makeTargetSurface(MakeInfo(6, 6));
        if (ringedImage && dst6x6) {
            auto ringColor = [](int x, int y) {
                return (x >= 2 && x <= 3 && y >= 2 && y <= 3) ? SK_ColorBLUE : SK_ColorGREEN;
            };
            auto checkFracSubset = [&](const SkRect& subset,
                                       int firstX, int lastX,
                                       int firstY, int lastY,
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

    // 4. Sub-texel subsets (< 1 texel wide/tall): [2.2, 2.2, 2.8, 2.8] collapses the clamp
    // rect to the single texel center (2.5, 2.5) while surrounded on all sides by red texels.
    // Because GPU decal compares the snapped texel center (2.5) against [2.2, 2.8], texel 2
    // fills the entire [600, 900) device range at 300x.
    {
        sk_sp<SkImage> singleTexelImage =
                MakeSourceImage(factories, {{SkRect{2.f, 2.f, 3.f, 3.f}, SK_ColorBLUE}});
        sk_sp<SkSurface> winSurface = factories.makeTargetSurface(MakeInfo(16, 16));
        sk_sp<SkSurface> dst6x6 = factories.makeTargetSurface(MakeInfo(6, 6));
        if (singleTexelImage && winSurface && dst6x6) {
            constexpr SkRect kSubTexelSubset = {2.2f, 2.2f, 2.8f, 2.8f};
            const SkMatrix scale300 = SkMatrix::Scale(300.f, 300.f);
            sk_sp<SkShader> subTexelShader = MakeNearestSubsetShader(singleTexelImage,
                                                                     kSubTexelSubset,
                                                                     SkTileMode::kDecal,
                                                                     SkTileMode::kDecal,
                                                                     &scale300);

            // Check both the top-left (600, 600) and bottom-right (900, 900) corners of the
            // magnified texel [600, 900) x [600, 900).
            CheckShaderWindow(
                    reporter,
                    winSurface.get(),
                    subTexelShader,
                    /*canvasOffset=*/{-(600.f - 8.f), -(600.f - 8.f)},
                    [](int x, int y) {
                        return (x >= 8 && y >= 8) ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    "SubTexel 300x TopLeft");
            CheckShaderWindow(
                    reporter,
                    winSurface.get(),
                    std::move(subTexelShader),
                    /*canvasOffset=*/{-(900.f - 8.f), -(900.f - 8.f)},
                    [](int x, int y) {
                        return (x < 8 && y < 8) ? SK_ColorBLUE : SK_ColorTRANSPARENT;
                    },
                    "SubTexel 300x BottomRight");

            // [2.6, 2.6, 3.4, 3.4] straddles a texel boundary without containing any texel
            // center (2.5 < 2.6 and 3.5 > 3.4): every pixel must resolve to border.
            CheckShaderWindow(reporter,
                              dst6x6.get(),
                              MakeNearestSubsetShader(singleTexelImage,
                                                      {2.6f, 2.6f, 3.4f, 3.4f},
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
