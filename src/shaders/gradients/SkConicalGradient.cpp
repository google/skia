/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/shaders/gradients/SkConicalGradient.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

bool SkConicalGradient::FocalData::set(SkScalar r0, SkScalar r1, SkMatrix* matrix) {
    fIsSwapped = false;
    fFocalX = sk_ieee_float_divide(r0, (r0 - r1));
    if (SkScalarNearlyZero(fFocalX - 1)) {
        // swap r0, r1
        matrix->postTranslate(-1, 0);
        matrix->postScale(-1, 1);
        std::swap(r0, r1);
        fFocalX = 0;  // because r0 is now 0
        fIsSwapped = true;
    }

    // Map {focal point, (1, 0)} to {(0, 0), (1, 0)}
    const SkPoint from[2]   = { {fFocalX, 0}, {1, 0} };
    const SkPoint to[2]     = { {0, 0}, {1, 0} };
    SkMatrix focalMatrix;
    if (!focalMatrix.setPolyToPoly(from, to, 2)) {
        return false;
    }
    matrix->postConcat(focalMatrix);
    fR1 = r1 / SkScalarAbs(1 - fFocalX);  // focalMatrix has a scale of 1/(1-f)

    // The following transformations are just to accelerate the shader computation by saving
    // some arithmatic operations.
    if (this->isFocalOnCircle()) {
        matrix->postScale(0.5, 0.5);
    } else {
        matrix->postScale(fR1 / (fR1 * fR1 - 1), 1 / sqrt(SkScalarAbs(fR1 * fR1 - 1)));
    }
    matrix->postScale(SkScalarAbs(1 - fFocalX), SkScalarAbs(1 - fFocalX));  // scale |1 - f|
    return true;
}

bool SkConicalGradient::MapToUnitX(const SkPoint &startCenter,
                                   const SkPoint &endCenter,
                                   SkMatrix* dstMatrix) {
    const SkPoint centers[2] = { startCenter, endCenter };
    const SkPoint unitvec[2] = { {0, 0}, {1, 0} };

    return dstMatrix->setPolyToPoly(centers, unitvec, 2);
}

sk_sp<SkShader> SkConicalGradient::Create(const SkPoint& c0,
                                          SkScalar r0,
                                          const SkPoint& c1,
                                          SkScalar r1,
                                          const Descriptor& desc,
                                          const SkMatrix* localMatrix) {
    SkMatrix gradientMatrix;
    Type gradientType;

    if (SkScalarNearlyZero((c0 - c1).length())) {
        if (SkScalarNearlyZero(std::max(r0, r1)) || SkScalarNearlyEqual(r0, r1)) {
            // Degenerate case; avoid dividing by zero. Should have been caught by caller but
            // just in case, recheck here.
            return nullptr;
        }
        // Concentric case: we can pretend we're radial (with a tiny twist).
        const SkScalar scale = sk_ieee_float_divide(1, std::max(r0, r1));
        gradientMatrix = SkMatrix::Translate(-c1.x(), -c1.y());
        gradientMatrix.postScale(scale, scale);

        gradientType = Type::kRadial;
    } else {
        if (!MapToUnitX(c0, c1, &gradientMatrix)) {
            // Degenerate case.
            return nullptr;
        }

        gradientType = SkScalarNearlyZero(r1 - r0) ? Type::kStrip : Type::kFocal;
    }

    FocalData focalData;
    if (gradientType == Type::kFocal) {
        const auto dCenter = (c0 - c1).length();
        if (!focalData.set(r0 / dCenter, r1 / dCenter, &gradientMatrix)) {
            return nullptr;
        }
    }

    sk_sp<SkShader> s = sk_make_sp<SkConicalGradient>(
            c0, r0, c1, r1, desc, gradientType, gradientMatrix, focalData);
    return s->makeWithLocalMatrix(localMatrix ? *localMatrix : SkMatrix::I());
}

SkConicalGradient::SkConicalGradient(const SkPoint& start,
                                     SkScalar startRadius,
                                     const SkPoint& end,
                                     SkScalar endRadius,
                                     const Descriptor& desc,
                                     Type type,
                                     const SkMatrix& gradientMatrix,
                                     const FocalData& data)
        : SkGradientBaseShader(desc, gradientMatrix)
        , fCenter1(start)
        , fCenter2(end)
        , fRadius1(startRadius)
        , fRadius2(endRadius)
        , fType(type) {
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    if (type == Type::kFocal) {
        fFocalData = data;
    }
}

bool SkConicalGradient::isOpaque() const {
    // Because areas outside the cone are left untouched, we cannot treat the
    // shader as opaque even if the gradient itself is opaque.
    // TODO(junov): Compute whether the cone fills the plane crbug.com/222380
    return false;
}

// Returns the original non-sorted version of the gradient
SkShaderBase::GradientType SkConicalGradient::asGradient(GradientInfo* info,
                                                         SkMatrix* localMatrix) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kConical;
}

sk_sp<SkFlattenable> SkConicalGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    SkMatrix legacyLocalMatrix, *lmPtr = nullptr;
    if (!desc.unflatten(buffer, &legacyLocalMatrix)) {
        return nullptr;
    }
    if (!legacyLocalMatrix.isIdentity()) {
        lmPtr = &legacyLocalMatrix;
    }
    SkPoint c1 = buffer.readPoint();
    SkPoint c2 = buffer.readPoint();
    SkScalar r1 = buffer.readScalar();
    SkScalar r2 = buffer.readScalar();

    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkGradientShader::MakeTwoPointConical(c1,
                                                 r1,
                                                 c2,
                                                 r2,
                                                 desc.fColors,
                                                 std::move(desc.fColorSpace),
                                                 desc.fPositions,
                                                 desc.fColorCount,
                                                 desc.fTileMode,
                                                 desc.fInterpolation,
                                                 lmPtr);
}

void SkConicalGradient::flatten(SkWriteBuffer& buffer) const {
    this->SkGradientBaseShader::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

void SkConicalGradient::appendGradientStages(SkArenaAlloc* alloc,
                                             SkRasterPipeline* p,
                                             SkRasterPipeline* postPipeline) const {
    const auto dRadius = fRadius2 - fRadius1;

    if (fType == Type::kRadial) {
        p->append(SkRasterPipelineOp::xy_to_radius);

        // Tiny twist: radial computes a t for [0, r2], but we want a t for [r1, r2].
        auto scale = std::max(fRadius1, fRadius2) / dRadius;
        auto bias = -fRadius1 / dRadius;

        p->appendMatrix(alloc, SkMatrix::Translate(bias, 0) * SkMatrix::Scale(scale, 1));
        return;
    }

    if (fType == Type::kStrip) {
        auto* ctx = alloc->make<SkRasterPipelineContexts::Conical2PtCtx>();
        SkScalar scaledR0 = fRadius1 / this->getCenterX1();
        ctx->fP0 = scaledR0 * scaledR0;
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_strip, ctx);
        p->append(SkRasterPipelineOp::mask_2pt_conical_nan, ctx);
        postPipeline->append(SkRasterPipelineOp::apply_vector_mask, &ctx->fMask);
        return;
    }

    auto* ctx = alloc->make<SkRasterPipelineContexts::Conical2PtCtx>();
    ctx->fP0 = 1 / fFocalData.fR1;
    ctx->fP1 = fFocalData.fFocalX;

    if (fFocalData.isFocalOnCircle()) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_focal_on_circle);
    } else if (fFocalData.isWellBehaved()) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_well_behaved, ctx);
    } else if (fFocalData.isSwapped() || 1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_smaller, ctx);
    } else {
        p->append(SkRasterPipelineOp::xy_to_2pt_conical_greater, ctx);
    }

    if (!fFocalData.isWellBehaved()) {
        p->append(SkRasterPipelineOp::mask_2pt_conical_degenerates, ctx);
    }
    if (1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipelineOp::negate_x);
    }
    if (!fFocalData.isNativelyFocal()) {
        p->append(SkRasterPipelineOp::alter_2pt_conical_compensate_focal, ctx);
    }
    if (fFocalData.isSwapped()) {
        p->append(SkRasterPipelineOp::alter_2pt_conical_unswap);
    }
    if (!fFocalData.isWellBehaved()) {
        postPipeline->append(SkRasterPipelineOp::apply_vector_mask, &ctx->fMask);
    }
}

// assumes colors is SkColor4f* and pos is SkScalar*
#define EXPAND_1_COLOR(count)            \
    SkColor4f tmp[2];                    \
    do {                                 \
        if (1 == count) {                \
            tmp[0] = tmp[1] = colors[0]; \
            colors = tmp;                \
            pos = nullptr;               \
            count = 2;                   \
        }                                \
    } while (0)

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkTileMode mode,
                                                      const Interpolation& interpolation,
                                                      const SkMatrix* localMatrix) {
    if (startRadius < 0 || endRadius < 0) {
        return nullptr;
    }
    if (!SkGradientBaseShader::ValidGradient(colors, colorCount, mode, interpolation)) {
        return nullptr;
    }
    if (SkScalarNearlyZero((start - end).length(), SkGradientBaseShader::kDegenerateThreshold)) {
        // If the center positions are the same, then the gradient is the radial variant of a 2 pt
        // conical gradient, an actual radial gradient (startRadius == 0), or it is fully degenerate
        // (startRadius == endRadius).
        if (SkScalarNearlyEqual(
                    startRadius, endRadius, SkGradientBaseShader::kDegenerateThreshold)) {
            // Degenerate case, where the interpolation region area approaches zero. The proper
            // behavior depends on the tile mode, which is consistent with the default degenerate
            // gradient behavior, except when mode = clamp and the radii > 0.
            if (mode == SkTileMode::kClamp &&
                endRadius > SkGradientBaseShader::kDegenerateThreshold) {
                // The interpolation region becomes an infinitely thin ring at the radius, so the
                // final gradient will be the first color repeated from p=0 to 1, and then a hard
                // stop switching to the last color at p=1.
                static constexpr SkScalar circlePos[3] = {0, 1, 1};
                SkColor4f reColors[3] = {colors[0], colors[0], colors[colorCount - 1]};
                return MakeRadial(start,
                                  endRadius,
                                  reColors,
                                  std::move(colorSpace),
                                  circlePos,
                                  3,
                                  mode,
                                  interpolation,
                                  localMatrix);
            } else {
                // Otherwise use the default degenerate case
                return SkGradientBaseShader::MakeDegenerateGradient(
                        colors, pos, colorCount, std::move(colorSpace), mode);
            }
        } else if (SkScalarNearlyZero(startRadius, SkGradientBaseShader::kDegenerateThreshold)) {
            // We can treat this gradient as radial, which is faster. If we got here, we know
            // that endRadius is not equal to 0, so this produces a meaningful gradient
            return MakeRadial(start,
                              endRadius,
                              colors,
                              std::move(colorSpace),
                              pos,
                              colorCount,
                              mode,
                              interpolation,
                              localMatrix);
        }
        // Else it's the 2pt conical radial variant with no degenerate radii, so fall through to the
        // regular 2pt constructor.
    }

    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    EXPAND_1_COLOR(colorCount);

    SkGradientBaseShader::Descriptor desc(
            colors, std::move(colorSpace), pos, colorCount, mode, interpolation);
    return SkConicalGradient::Create(start, startRadius, end, endRadius, desc, localMatrix);
}

#undef EXPAND_1_COLOR

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
    SkColorConverter converter(colors, colorCount);
    return MakeTwoPointConical(start,
                               startRadius,
                               end,
                               endRadius,
                               converter.fColors4f.begin(),
                               nullptr,
                               pos,
                               colorCount,
                               mode,
                               flags,
                               localMatrix);
}

void SkRegisterConicalGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkConicalGradient);
    // Previous name
    SkFlattenable::Register("SkTwoPointConicalGradient", SkConicalGradient::CreateProc);
}
