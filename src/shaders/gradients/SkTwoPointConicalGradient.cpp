/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkFloatingPoint.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/gradients/SkTwoPointConicalGradient.h"

#include <utility>

// Please see https://skia.org/dev/design/conical for how our shader works.

bool SkTwoPointConicalGradient::FocalData::set(SkScalar r0, SkScalar r1, SkMatrix* matrix) {
    fIsSwapped = false;
    fFocalX = sk_ieee_float_divide(r0, (r0 - r1));
    if (SkScalarNearlyZero(fFocalX - 1)) {
        // swap r0, r1
        matrix->postTranslate(-1, 0);
        matrix->postScale(-1, 1);
        std::swap(r0, r1);
        fFocalX = 0; // because r0 is now 0
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
    fR1 = r1 / SkScalarAbs(1 - fFocalX); // focalMatrix has a scale of 1/(1-f)

    // The following transformations are just to accelerate the shader computation by saving
    // some arithmatic operations.
    if (this->isFocalOnCircle()) {
        matrix->postScale(0.5, 0.5);
    } else {
        matrix->postScale(fR1 / (fR1 * fR1 - 1), 1 / sqrt(SkScalarAbs(fR1 * fR1 - 1)));
    }
    matrix->postScale(SkScalarAbs(1 - fFocalX), SkScalarAbs(1 - fFocalX)); // scale |1 - f|
    return true;
}

sk_sp<SkShader> SkTwoPointConicalGradient::Create(const SkPoint& c0, SkScalar r0,
                                                  const SkPoint& c1, SkScalar r1,
                                                  const Descriptor& desc) {
    SkMatrix gradientMatrix;
    Type     gradientType;

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
        const SkPoint centers[2] = { c0    , c1     };
        const SkPoint unitvec[2] = { {0, 0}, {1, 0} };

        if (!gradientMatrix.setPolyToPoly(centers, unitvec, 2)) {
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
    return sk_sp<SkShader>(new SkTwoPointConicalGradient(c0, r0, c1, r1, desc,
                                                         gradientType, gradientMatrix, focalData));
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
        const SkPoint& start, SkScalar startRadius,
        const SkPoint& end, SkScalar endRadius,
        const Descriptor& desc, Type type, const SkMatrix& gradientMatrix, const FocalData& data)
    : SkGradientShaderBase(desc, gradientMatrix)
    , fCenter1(start)
    , fCenter2(end)
    , fRadius1(startRadius)
    , fRadius2(endRadius)
    , fType(type)
{
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
    if (type == Type::kFocal) {
        fFocalData = data;
    }
}

bool SkTwoPointConicalGradient::isOpaque() const {
    // Because areas outside the cone are left untouched, we cannot treat the
    // shader as opaque even if the gradient itself is opaque.
    // TODO(junov): Compute whether the cone fills the plane crbug.com/222380
    return false;
}

// Returns the original non-sorted version of the gradient
SkShader::GradientType SkTwoPointConicalGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
    }
    return kConical_GradientType;
}

sk_sp<SkFlattenable> SkTwoPointConicalGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint c1 = buffer.readPoint();
    SkPoint c2 = buffer.readPoint();
    SkScalar r1 = buffer.readScalar();
    SkScalar r2 = buffer.readScalar();

    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkGradientShader::MakeTwoPointConical(c1, r1, c2, r2, desc.fColors,
                                                 std::move(desc.fColorSpace), desc.fPos,
                                                 desc.fCount, desc.fTileMode, desc.fGradFlags,
                                                 desc.fLocalMatrix);
}

void SkTwoPointConicalGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter1);
    buffer.writePoint(fCenter2);
    buffer.writeScalar(fRadius1);
    buffer.writeScalar(fRadius2);
}

void SkTwoPointConicalGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                                     SkRasterPipeline* postPipeline) const {
    const auto dRadius = fRadius2 - fRadius1;

    if (fType == Type::kRadial) {
        p->append(SkRasterPipeline::xy_to_radius);

        // Tiny twist: radial computes a t for [0, r2], but we want a t for [r1, r2].
        auto scale =  std::max(fRadius1, fRadius2) / dRadius;
        auto bias  = -fRadius1 / dRadius;

        p->append_matrix(alloc, SkMatrix::Translate(bias, 0) * SkMatrix::Scale(scale, 1));
        return;
    }

    if (fType == Type::kStrip) {
        auto* ctx = alloc->make<SkRasterPipeline_2PtConicalCtx>();
        SkScalar scaledR0 = fRadius1 / this->getCenterX1();
        ctx->fP0 = scaledR0 * scaledR0;
        p->append(SkRasterPipeline::xy_to_2pt_conical_strip, ctx);
        p->append(SkRasterPipeline::mask_2pt_conical_nan, ctx);
        postPipeline->append(SkRasterPipeline::apply_vector_mask, &ctx->fMask);
        return;
    }

    auto* ctx = alloc->make<SkRasterPipeline_2PtConicalCtx>();
    ctx->fP0 = 1/fFocalData.fR1;
    ctx->fP1 = fFocalData.fFocalX;

    if (fFocalData.isFocalOnCircle()) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_focal_on_circle);
    } else if (fFocalData.isWellBehaved()) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_well_behaved, ctx);
    } else if (fFocalData.isSwapped() || 1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_smaller, ctx);
    } else {
        p->append(SkRasterPipeline::xy_to_2pt_conical_greater, ctx);
    }

    if (!fFocalData.isWellBehaved()) {
        p->append(SkRasterPipeline::mask_2pt_conical_degenerates, ctx);
    }
    if (1 - fFocalData.fFocalX < 0) {
        p->append(SkRasterPipeline::negate_x);
    }
    if (!fFocalData.isNativelyFocal()) {
        p->append(SkRasterPipeline::alter_2pt_conical_compensate_focal, ctx);
    }
    if (fFocalData.isSwapped()) {
        p->append(SkRasterPipeline::alter_2pt_conical_unswap);
    }
    if (!fFocalData.isWellBehaved()) {
        postPipeline->append(SkRasterPipeline::apply_vector_mask, &ctx->fMask);
    }
}

skvm::F32 SkTwoPointConicalGradient::transformT(skvm::Builder* p, skvm::Uniforms* uniforms,
                                                skvm::Coord coord, skvm::I32* mask) const {
    auto mag = [](skvm::F32 x, skvm::F32 y) { return sqrt(x*x + y*y); };

    // See https://skia.org/dev/design/conical, and onAppendStages() above.
    // There's a lot going on here, and I'm not really sure what's independent
    // or disjoint, what can be reordered, simplified, etc.  Tweak carefully.

    const skvm::F32 x = coord.x,
                    y = coord.y;
    if (fType == Type::kRadial) {
        float denom = 1.0f / (fRadius2 - fRadius1),
              scale = std::max(fRadius1, fRadius2) * denom,
               bias =                  -fRadius1 * denom;
        return mag(x,y) * p->uniformF(uniforms->pushF(scale))
                        + p->uniformF(uniforms->pushF(bias ));
    }

    if (fType == Type::kStrip) {
        float r = fRadius1 / this->getCenterX1();
        skvm::F32 t = x + sqrt(p->uniformF(uniforms->pushF(r*r)) - y*y);

        *mask = (t == t);   // t != NaN
        return t;
    }

    const skvm::F32 invR1 = p->uniformF(uniforms->pushF(1 / fFocalData.fR1));

    skvm::F32 t;
    if (fFocalData.isFocalOnCircle()) {
        t = (y/x) * y + x;       // (x^2 + y^2) / x  ~~>  x + y^2/x  ~~>  y/x * y + x
    } else if (fFocalData.isWellBehaved()) {
        t = mag(x,y) - x*invR1;
    } else {
        skvm::F32 k = sqrt(x*x - y*y);
        if (fFocalData.isSwapped() || 1 - fFocalData.fFocalX < 0) {
            k = -k;
        }
        t = k - x*invR1;
    }

    if (!fFocalData.isWellBehaved()) {
        // TODO: not sure why we consider t == 0 degenerate
        *mask = (t > 0.0f);  // and implicitly, t != NaN
    }

    const skvm::F32 focalX = p->uniformF(uniforms->pushF(fFocalData.fFocalX));
    if (1 - fFocalData.fFocalX < 0)    { t = -t; }
    if (!fFocalData.isNativelyFocal()) { t += focalX; }
    if ( fFocalData.isSwapped())       { t = 1.0f - t; }
    return t;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkTwoPointConicalGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    return GrGradientShader::MakeConical(*this, args);
}

#endif
