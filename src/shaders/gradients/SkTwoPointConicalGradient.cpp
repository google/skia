/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient.h"

#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "../../jumper/SkJumper.h"

sk_sp<SkShader> SkTwoPointConicalGradient::Create(const SkPoint& c0, SkScalar r0,
                                                  const SkPoint& c1, SkScalar r1,
                                                  const Descriptor& desc) {
    SkMatrix gradientMatrix;
    Type     gradientType;

    if (SkScalarNearlyZero((c0 - c1).length())) {
        // Concentric case: we can pretend we're radial (with a tiny twist).
        const SkScalar scale = 1.0f / SkTMax(r0, r1);
        gradientMatrix = SkMatrix::MakeTrans(-c1.x(), -c1.y());
        gradientMatrix.postScale(scale, scale);

        gradientType = Type::kRadial;
    } else {
        const SkPoint centers[2] = { c0    , c1     };
        const SkPoint unitvec[2] = { {0, 0}, {1, 0} };

        if (!gradientMatrix.setPolyToPoly(centers, unitvec, 2)) {
            // Degenerate case.
            return nullptr;
        }

        if (SkScalarNearlyZero(shader.getDiffRadius())) {
            gradientType = Type::kStrip;
        } else {
            gradientType = Type::kFocal;

            SkScalar focalX = - r0 / (r1 - r0);
            if (SkScalarNearlyZero(focalX - 1)) {
                // swap r0, r1
                gradientMatrix.postTranslate(-1, 0);
                gradientMatrix.postScale(-1, 1);
                std::swap(r0, r1);
                fRadius0 = 0;
                fDiffRadius = -fDiffRadius;
                fIsSwapped = true;
                focalX = 0; // - fRadius0 / fDiffRadius;
                gradientType = Type::kSwappedFocal;
            }

            // Map {focal point, (1, 0)} to {(0, 0), (1, 0)}
            const SkPoint from[2]   = { {focalX, 0}, {1, 0} };
            const SkPoint to[2]     = { {0, 0}, {1, 0} };
            SkMatrix focalMatrix;
            if (!focalMatrix.setPolyToPoly(from, to, 2)) {
                SkDEBUGFAILF("Mapping focal point failed unexpectedly for focalX = %f.\n", focalX);
                // We won't be able to draw the gradient; at least make sure that we initialize the
                // memory to prevent security issues.
                focalMatrix = SkMatrix::MakeScale(1, 1);
            }
            gradientMatrix.postConcat(focalMatrix);
            r0 /= SkScalarAbs(1 - focalX);
            r1 /= SkScalarAbs(1 - focalX);

            // The following transformations are just to accelerate the shader computation by saving
            // some arithmatic operations.
            bool isFocalOnCircle = SkScalarNearlyZero(1 - r1);
            if (isFocalOnCircle) {
                gradientMatrix.postScale(0.5, 0.5); // r1 = 1 so r1 + 1 = 2 and 0.5 = 1 / (r1 + 1)
            } else {
                SkScalar scale = SkScalarAbs(r1 / (r1 - r0)); // |1 - f|
                gradientMatrix.postScale(scale, scale);
            }
       }

    }

    return sk_sp<SkShader>(new SkTwoPointConicalGradient(c0, r0, c1, r1, desc,
                                                         gradientType, gradientMatrix));
}

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
        const SkPoint& start, SkScalar startRadius,
        const SkPoint& end, SkScalar endRadius,
        const Descriptor& desc, Type type, const SkMatrix& gradientMatrix)
    : SkGradientShaderBase(desc, gradientMatrix)
    , fCenter1(start)
    , fCenter2(end)
    , fRadius1(startRadius)
    , fRadius2(endRadius)
    , fType(type)
{
    // this is degenerate, and should be caught by our caller
    SkASSERT(fCenter1 != fCenter2 || fRadius1 != fRadius2);
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

    if (buffer.isVersionLT(SkReadBuffer::k2PtConicalNoFlip_Version) && buffer.readBool()) {
        // legacy flipped gradient
        SkTSwap(c1, c2);
        SkTSwap(r1, r2);

        SkColor4f* colors = desc.mutableColors();
        SkScalar* pos = desc.mutablePos();
        const int last = desc.fCount - 1;
        const int half = desc.fCount >> 1;
        for (int i = 0; i < half; ++i) {
            SkTSwap(colors[i], colors[last - i]);
            if (pos) {
                SkScalar tmp = pos[i];
                pos[i] = SK_Scalar1 - pos[last - i];
                pos[last - i] = SK_Scalar1 - tmp;
            }
        }
        if (pos) {
            if (desc.fCount & 1) {
                pos[half] = SK_Scalar1 - pos[half];
            }
        }
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

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "SkTwoPointConicalGradient_gpu.h"

std::unique_ptr<GrFragmentProcessor> SkTwoPointConicalGradient::asFragmentProcessor(
        const AsFPArgs& args) const {
    SkASSERT(args.fContext);
    return Gr2PtConicalGradientEffect::Make(
            GrGradientEffect::CreateArgs(args.fContext, this, args.fLocalMatrix, fTileMode,
                                         args.fDstColorSpaceInfo->colorSpace()));
}

#endif

sk_sp<SkShader> SkTwoPointConicalGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    const AutoXformColors xformedColors(*this, xformer);
    return SkGradientShader::MakeTwoPointConical(fCenter1, fRadius1, fCenter2, fRadius2,
                                                 xformedColors.fColors.get(), fOrigPos, fColorCount,
                                                 fTileMode, fGradFlags, &this->getLocalMatrix());
}


#ifndef SK_IGNORE_TO_STRING
void SkTwoPointConicalGradient::toString(SkString* str) const {
    str->append("SkTwoPointConicalGradient: (");

    str->append("center1: (");
    str->appendScalar(fCenter1.fX);
    str->append(", ");
    str->appendScalar(fCenter1.fY);
    str->append(") radius1: ");
    str->appendScalar(fRadius1);
    str->append(" ");

    str->append("center2: (");
    str->appendScalar(fCenter2.fX);
    str->append(", ");
    str->appendScalar(fCenter2.fY);
    str->append(") radius2: ");
    str->appendScalar(fRadius2);
    str->append(" ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

void SkTwoPointConicalGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                                     SkRasterPipeline* postPipeline) const {
    const auto dRadius = fRadius2 - fRadius1;

    if (fType == Type::kRadial) {
        p->append(SkRasterPipeline::xy_to_radius);

        // Tiny twist: radial computes a t for [0, r2], but we want a t for [r1, r2].
        auto scale =  SkTMax(fRadius1, fRadius2) / dRadius;
        auto bias  = -fRadius1 / dRadius;

        p->append_matrix(alloc, SkMatrix::Concat(SkMatrix::MakeTrans(bias, 0),
                                                 SkMatrix::MakeScale(scale, 1)));
        return;
    }

    if (fType == Type::kStrip) {
        // TODO To be implemented.
        return;
    }

    auto* ctx = alloc->make<SkJumper_2PtConicalCtx>();
    ctx->fP0 = 1/fRadius2;
    ctx->fP1 = fRadius1 / (fRadius2 - fRadius1);

    if (this->isFocalOnCircle()) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_linear);
    } else if (this->isWellBehaved()) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_well_behaved, ctx);
    } else if (this->fIsSwapped || !this->isRadiusIncreasing()) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_smaller);
    } else {
        p->append(SkRasterPipeline::xy_to_2pt_conical_greater);
    }

    if (!SkNearlyZero(ctx->fP1)) {
        if (this->isRadiusIncreasing()) {
            p->append(SkRasterPipeline::xy_to_2pt_conical_compensate_focal_positive, ctx);
        } else {
            p->append(SkRasterPipeline::xy_to_2pt_conical_compensate_focal_negative, ctx);
        }
    }

    if (fIsSwapped) {
        p->append(SkRasterPipeline::xy_to_2pt_conical_unswap);
    }
}
