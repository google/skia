/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkSweepGradient.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <cstdint>
#include <tuple>
#include <utility>

class SkArenaAlloc;

SkSweepGradient::SkSweepGradient(const SkPoint& center,
                                 SkScalar t0,
                                 SkScalar t1,
                                 const SkGradient& desc)
        : SkGradientBaseShader(desc, SkMatrix::Translate(-center.x(), -center.y()))
        , fCenter(center)
        , fTBias(-t0)
        , fTScale(1 / (t1 - t0)) {
    SkASSERT(t0 < t1);
}

SkShaderBase::GradientType SkSweepGradient::asGradient(GradientInfo* info,
                                                       SkMatrix* localMatrix) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
        info->fPoint[1].fX = fTScale;
        info->fPoint[1].fY = fTBias;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kSweep;
}

static std::tuple<SkScalar, SkScalar> angles_from_t_coeff(SkScalar tBias, SkScalar tScale) {
    return std::make_tuple(-tBias * 360, (sk_ieee_float_divide(1, tScale) - tBias) * 360);
}

sk_sp<SkFlattenable> SkSweepGradient::CreateProc(SkReadBuffer& buffer) {
    SkGradientScope scope;
    SkMatrix legacyLocalMatrix, *lmPtr = nullptr;
    auto grad = scope.unflatten(buffer, &legacyLocalMatrix);
    if (!grad) {
        return nullptr;
    }
    if (!legacyLocalMatrix.isIdentity()) {
        lmPtr = &legacyLocalMatrix;
    }
    const SkPoint center = buffer.readPoint();

    const auto tBias  = buffer.readScalar(),
               tScale = buffer.readScalar();
    auto [startAngle, endAngle] = angles_from_t_coeff(tBias, tScale);

    return SkShaders::SweepGradient(center, startAngle, endAngle, *grad, lmPtr);
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->SkGradientBaseShader::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fTBias);
    buffer.writeScalar(fTScale);
}

void SkSweepGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                           SkRasterPipeline*) const {
    p->append(SkRasterPipelineOp::xy_to_unit_angle);
    p->appendMatrix(alloc, SkMatrix::Scale(fTScale, 1) * SkMatrix::Translate(fTBias, 0));
}

sk_sp<SkShader> SkShaders::SweepGradient(SkPoint center, float startAngle, float endAngle,
                                         const SkGradient& grad, const SkMatrix* lm) {
    if (!SkIsFinite(startAngle, endAngle) || startAngle > endAngle) {
        return nullptr;
    }

    GRADIENT_FACTORY_EARLY_EXIT(grad, lm);

    const auto& colors = grad.colors();
    const auto& interp = grad.interpolation();
    SkTileMode mode = colors.tileMode();

    if (SkScalarNearlyEqual(startAngle, endAngle, SkGradientBaseShader::kDegenerateThreshold)) {
        // Degenerate gradient, which should follow default degenerate behavior unless it is
        // clamped and the angle is greater than 0.
        if (mode == SkTileMode::kClamp && endAngle > SkGradientBaseShader::kDegenerateThreshold) {
            // In this case, the first color is repeated from 0 to the angle, then a hardstop
            // switches to the last color (all other colors are compressed to the infinitely thin
            // interpolation region).
            static constexpr float clampPos[3] = {0, 1, 1};
            SkSpan<const SkColor4f> srcColors = colors.colors();
            const SkColor4f reColors[3] = {
                srcColors.front(), srcColors.front(), srcColors.back()
            };
            SkGradient::Colors newColors = {
                reColors, clampPos, colors.tileMode(), colors.colorSpace()
            };
            return SkShaders::SweepGradient(center, 0, endAngle, {newColors, interp}, lm);
        } else {
            return SkGradientBaseShader::MakeDegenerateGradient(colors);
        }
    }

    if (startAngle <= 0 && endAngle >= 360) {
        // If the t-range includes [0,1], then we can always use clamping (presumably faster).
        mode = SkTileMode::kClamp;
    }

    const SkGradient newGrad {
        {colors.colors(), colors.positions(), mode, colors.colorSpace()},
        interp
    };

    const float t0 = startAngle / 360,
                t1 =   endAngle / 360;

    sk_sp<SkShader> s = sk_make_sp<SkSweepGradient>(center, t0, t1, newGrad);
    return s->makeWithLocalMatrix(lm ? *lm : SkMatrix::I());
}

void SkRegisterSweepGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkSweepGradient);
}

#ifdef SK_SUPPORT_LEGACY_UNSPANNED_GRADIENTS
sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colorsPtr[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar posPtr[],
                                            int colorsCount,
                                            SkTileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            const Interpolation& interp,
                                            const SkMatrix* lm) {
    MAKE_COLORS_POS_SPANS(colorsPtr, posPtr, colorsCount);

    return SkShaders::SweepGradient({cx, cy}, startAngle, endAngle,
                                    {{colors, pos, mode, std::move(colorSpace)}, interp}, lm);
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
    SkColorConverter converter({colors, SkToSizeT(colorCount)});
    return MakeSweep(cx, cy, converter.colors4f().data(), nullptr, pos, colorCount,
                     mode, startAngle, endAngle, flags, localMatrix);
}
#endif
