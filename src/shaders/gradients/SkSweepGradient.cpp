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
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <cstdint>
#include <tuple>
#include <utility>

class SkArenaAlloc;

SkSweepGradient::SkSweepGradient(const SkPoint& center,
                                 SkScalar t0,
                                 SkScalar t1,
                                 const Descriptor& desc)
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
    DescriptorScope desc;
    SkMatrix legacyLocalMatrix;
    if (!desc.unflatten(buffer, &legacyLocalMatrix)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();

    const auto tBias  = buffer.readScalar(),
               tScale = buffer.readScalar();
    auto [startAngle, endAngle] = angles_from_t_coeff(tBias, tScale);

    return SkGradientShader::MakeSweep(center.x(), center.y(),
                                       desc.fColors,
                                       std::move(desc.fColorSpace),
                                       desc.fPositions,
                                       desc.fColorCount,
                                       desc.fTileMode,
                                       startAngle,
                                       endAngle,
                                       desc.fInterpolation,
                                       &legacyLocalMatrix);
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

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkTileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            const Interpolation& interpolation,
                                            const SkMatrix* localMatrix) {
    if (!SkGradientBaseShader::ValidGradient(colors, colorCount, mode, interpolation)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShaders::Color(colors[0], std::move(colorSpace));
    }
    if (!SkScalarIsFinite(startAngle) || !SkScalarIsFinite(endAngle) || startAngle > endAngle) {
        return nullptr;
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    if (SkScalarNearlyEqual(startAngle, endAngle, SkGradientBaseShader::kDegenerateThreshold)) {
        // Degenerate gradient, which should follow default degenerate behavior unless it is
        // clamped and the angle is greater than 0.
        if (mode == SkTileMode::kClamp && endAngle > SkGradientBaseShader::kDegenerateThreshold) {
            // In this case, the first color is repeated from 0 to the angle, then a hardstop
            // switches to the last color (all other colors are compressed to the infinitely thin
            // interpolation region).
            static constexpr SkScalar clampPos[3] = {0, 1, 1};
            SkColor4f reColors[3] = {colors[0], colors[0], colors[colorCount - 1]};
            return MakeSweep(cx, cy, reColors, std::move(colorSpace), clampPos, 3, mode, 0,
                             endAngle, interpolation, localMatrix);
        } else {
            return SkGradientBaseShader::MakeDegenerateGradient(
                    colors, pos, colorCount, std::move(colorSpace), mode);
        }
    }

    if (startAngle <= 0 && endAngle >= 360) {
        // If the t-range includes [0,1], then we can always use clamping (presumably faster).
        mode = SkTileMode::kClamp;
    }

    SkGradientBaseShader::Descriptor desc(
            colors, std::move(colorSpace), pos, colorCount, mode, interpolation);

    const SkScalar t0 = startAngle / 360,
                   t1 =   endAngle / 360;

    return SkLocalMatrixShader::MakeWrapped<SkSweepGradient>(localMatrix,
                                                             SkPoint::Make(cx, cy),
                                                             t0, t1,
                                                             desc);
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
    SkColorConverter converter(colors, colorCount);
    return MakeSweep(cx, cy, converter.fColors4f.begin(), nullptr, pos, colorCount,
                     mode, startAngle, endAngle, flags, localMatrix);
}

void SkRegisterSweepGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkSweepGradient);
}
