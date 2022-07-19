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

///////////////////////////////////////////////////////////////////////////////

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
    SkRegisterTwoPointConicalGradientShaderFlattenable();
}
