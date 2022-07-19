/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkGradientShader.h"

#include "include/core/SkColorSpace.h"
#include "include/private/SkTArray.h"
#include "src/shaders/gradients/SkGradientShaderBase.h"
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkTwoPointConicalGradient.h"

///////////////////////////////////////////////////////////////////////////////

// assumes colors is SkColor4f* and pos is SkScalar*
#define EXPAND_1_COLOR(count)                \
     SkColor4f tmp[2];                       \
     do {                                    \
         if (1 == count) {                   \
             tmp[0] = tmp[1] = colors[0];    \
             colors = tmp;                   \
             pos = nullptr;                  \
             count = 2;                      \
         }                                   \
     } while (0)

// TODO: move the ColorConverter into SkGradientShaderBase
struct ColorConverter {
    ColorConverter(const SkColor* colors, int count) {
        const float ONE_OVER_255 = 1.f / 255;
        for (int i = 0; i < count; ++i) {
            fColors4f.push_back({
                SkColorGetR(colors[i]) * ONE_OVER_255,
                SkColorGetG(colors[i]) * ONE_OVER_255,
                SkColorGetB(colors[i]) * ONE_OVER_255,
                SkColorGetA(colors[i]) * ONE_OVER_255 });
        }
    }

    SkSTArray<2, SkColor4f, true> fColors4f;
};

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeLinear(pts, converter.fColors4f.begin(), nullptr, pos, colorCount, mode, flags,
                      localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int count, SkTileMode mode) {
    return MakeLinear(pts, colors, std::move(colorSpace), pos, count, mode, 0, nullptr);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int colorCount,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    if (!pts || !SkScalarIsFinite((pts[1] - pts[0]).length())) {
        return nullptr;
    }
    if (!SkGradientShaderBase::ValidGradient(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShaders::Color(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    if (SkScalarNearlyZero((pts[1] - pts[0]).length(),
                           SkGradientShaderBase::kDegenerateThreshold)) {
        // Degenerate gradient, the only tricky complication is when in clamp mode, the limit of
        // the gradient approaches two half planes of solid color (first and last). However, they
        // are divided by the line perpendicular to the start and end point, which becomes undefined
        // once start and end are exactly the same, so just use the end color for a stable solution.
        return SkGradientShaderBase::MakeDegenerateGradient(colors, pos, colorCount,
                                                            std::move(colorSpace), mode);
    }

    SkGradientShaderBase::ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc(opt.fColors, std::move(colorSpace), opt.fPos,
                                          opt.fCount, mode, flags, localMatrix);
    return sk_make_sp<SkLinearGradient>(pts, desc);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeRadial(center, radius, converter.fColors4f.begin(), nullptr, pos, colorCount, mode,
                      flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int count, SkTileMode mode) {
    return MakeRadial(center, radius, colors, std::move(colorSpace), pos, count, mode, 0, nullptr);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor colors[],
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkTileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeTwoPointConical(start, startRadius, end, endRadius, converter.fColors4f.begin(),
                               nullptr, pos, colorCount, mode, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int count, SkTileMode mode) {
    return MakeTwoPointConical(start, startRadius, end, endRadius, colors,
                               std::move(colorSpace), pos, count, mode, 0, nullptr);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkTileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    if (startRadius < 0 || endRadius < 0) {
        return nullptr;
    }
    if (!SkGradientShaderBase::ValidGradient(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (SkScalarNearlyZero((start - end).length(), SkGradientShaderBase::kDegenerateThreshold)) {
        // If the center positions are the same, then the gradient is the radial variant of a 2 pt
        // conical gradient, an actual radial gradient (startRadius == 0), or it is fully degenerate
        // (startRadius == endRadius).
        if (SkScalarNearlyEqual(startRadius, endRadius,
                                SkGradientShaderBase::kDegenerateThreshold)) {
            // Degenerate case, where the interpolation region area approaches zero. The proper
            // behavior depends on the tile mode, which is consistent with the default degenerate
            // gradient behavior, except when mode = clamp and the radii > 0.
            if (mode == SkTileMode::kClamp &&
                endRadius > SkGradientShaderBase::kDegenerateThreshold) {
                // The interpolation region becomes an infinitely thin ring at the radius, so the
                // final gradient will be the first color repeated from p=0 to 1, and then a hard
                // stop switching to the last color at p=1.
                static constexpr SkScalar circlePos[3] = {0, 1, 1};
                SkColor4f reColors[3] = {colors[0], colors[0], colors[colorCount - 1]};
                return MakeRadial(start, endRadius, reColors, std::move(colorSpace),
                                  circlePos, 3, mode, flags, localMatrix);
            } else {
                // Otherwise use the default degenerate case
                return SkGradientShaderBase::MakeDegenerateGradient(colors, pos, colorCount,
                                                                    std::move(colorSpace), mode);
            }
        } else if (SkScalarNearlyZero(startRadius, SkGradientShaderBase::kDegenerateThreshold)) {
            // We can treat this gradient as radial, which is faster. If we got here, we know
            // that endRadius is not equal to 0, so this produces a meaningful gradient
            return MakeRadial(start, endRadius, colors, std::move(colorSpace), pos, colorCount,
                              mode, flags, localMatrix);
        }
        // Else it's the 2pt conical radial variant with no degenerate radii, so fall through to the
        // regular 2pt constructor.
    }

    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    EXPAND_1_COLOR(colorCount);

    SkGradientShaderBase::ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc(opt.fColors, std::move(colorSpace), opt.fPos,
                                          opt.fCount, mode, flags, localMatrix);
    return SkTwoPointConicalGradient::Create(start, startRadius, end, endRadius, desc);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkTileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeSweep(cx, cy, converter.fColors4f.begin(), nullptr, pos, colorCount,
                     mode, startAngle, endAngle, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[], int count,
                                            uint32_t flags, const SkMatrix* localMatrix) {
    return MakeSweep(cx, cy, colors, std::move(colorSpace), pos, count,
                     SkTileMode::kClamp, 0, 360, flags, localMatrix);
}
sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[], int count) {
    return MakeSweep(cx, cy, colors, std::move(colorSpace), pos, count, 0, nullptr);
}


// TODO: flatten this into SkFlattenable::PrivateInitializer::InitEffects
void SkGradientShader::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkLinearGradient);
    SkRegisterRadialGradientShaderFlattenable();
    SkRegisterSweepGradientShaderFlattenable();
    SK_REGISTER_FLATTENABLE(SkTwoPointConicalGradient);
}
