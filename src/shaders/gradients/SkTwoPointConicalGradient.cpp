/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTwoPointConicalGradient.h"

#include "SkRasterPipeline.h"
#include "../../jumper/SkJumper.h"

SkTwoPointConicalGradient::SkTwoPointConicalGradient(
        const SkPoint& start, SkScalar startRadius,
        const SkPoint& end, SkScalar endRadius,
        bool flippedGrad, const Descriptor& desc)
    : SkGradientShaderBase(desc, SkMatrix::I())
    , fCenter1(start)
    , fCenter2(end)
    , fRadius1(startRadius)
    , fRadius2(endRadius)
    , fFlippedGrad(flippedGrad)
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
SkShader::GradientType SkTwoPointConicalGradient::asAGradient(
    GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info, fFlippedGrad);
        info->fPoint[0] = fCenter1;
        info->fPoint[1] = fCenter2;
        info->fRadius[0] = fRadius1;
        info->fRadius[1] = fRadius2;
        if (fFlippedGrad) {
            SkTSwap(info->fPoint[0], info->fPoint[1]);
            SkTSwap(info->fRadius[0], info->fRadius[1]);
        }
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

    if (buffer.readBool()) {    // flipped
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
    buffer.writeBool(fFlippedGrad);
}

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "SkTwoPointConicalGradient_gpu.h"

sk_sp<GrFragmentProcessor> SkTwoPointConicalGradient::asFragmentProcessor(
        const AsFPArgs& args) const {
    SkASSERT(args.fContext);
    SkASSERT(fPtsToUnit.isIdentity());
    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(fColorSpace.get(),
                                                                       args.fDstColorSpace);
    sk_sp<GrFragmentProcessor> inner(Gr2PtConicalGradientEffect::Make(
        GrGradientEffect::CreateArgs(args.fContext, this, args.fLocalMatrix, fTileMode,
                                     std::move(colorSpaceXform), SkToBool(args.fDstColorSpace))));
    if (!inner) {
        return nullptr;
    }
    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
}

#endif

sk_sp<SkShader> SkTwoPointConicalGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkSTArray<8, SkColor> origColorsStorage(fColorCount);
    SkSTArray<8, SkScalar> origPosStorage(fColorCount);
    SkSTArray<8, SkColor> xformedColorsStorage(fColorCount);
    SkColor* origColors = origColorsStorage.begin();
    SkScalar* origPos = fOrigPos ? origPosStorage.begin() : nullptr;
    SkColor* xformedColors = xformedColorsStorage.begin();

    // Flip if necessary
    SkPoint center1 = fFlippedGrad ? fCenter2 : fCenter1;
    SkPoint center2 = fFlippedGrad ? fCenter1 : fCenter2;
    SkScalar radius1 = fFlippedGrad ? fRadius2 : fRadius1;
    SkScalar radius2 = fFlippedGrad ? fRadius1 : fRadius2;
    for (int i = 0; i < fColorCount; i++) {
        origColors[i] = fFlippedGrad ? fOrigColors[fColorCount - i - 1] : fOrigColors[i];
        if (origPos) {
            origPos[i] = fFlippedGrad ? 1.0f - fOrigPos[fColorCount - i - 1] : fOrigPos[i];
        }
    }

    xformer->apply(xformedColors, origColors, fColorCount);
    return SkGradientShader::MakeTwoPointConical(center1, radius1, center2, radius2, xformedColors,
                                                 origPos, fColorCount, fTileMode, fGradFlags,
                                                 &this->getLocalMatrix());
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

bool SkTwoPointConicalGradient::adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                                            SkMatrix* matrix,
                                                            SkRasterPipeline* p,
                                                            SkRasterPipeline* postPipeline) const {
    const auto dCenter = (fCenter1 - fCenter2).length();
    const auto dRadius = fRadius2 - fRadius1;
    SkASSERT(dRadius >= 0);

    // When the two circles are concentric, we can pretend we're radial (with a tiny *twist).
    if (SkScalarNearlyZero(dCenter)) {
        matrix->postTranslate(-fCenter1.fX, -fCenter1.fY);
        matrix->postScale(1 / fRadius2, 1 / fRadius2);
        p->append(SkRasterPipeline::xy_to_radius);

        // Tiny twist: radial computes a t for [0, r2], but we want a t for [r1, r2].
        auto scale =  fRadius2 / dRadius;
        auto bias  = -fRadius1 / dRadius;

        p->append_matrix(alloc, SkMatrix::Concat(SkMatrix::MakeTrans(bias, 0),
                                                 SkMatrix::MakeScale(scale, 1)));

        return true;
    }

    // To simplify the stage math, we transform the universe (translate/scale/rotate)
    // such that fCenter1 -> (0, 0) and fCenter2 -> (1, 0).
    SkMatrix map_to_unit_vector;
    const SkPoint centers[2] = { fCenter1, fCenter2 };
    const SkPoint unitvec[2] = { {0, 0}, {1, 0} };
    if (!map_to_unit_vector.setPolyToPoly(centers, unitvec, 2)) {
        return false;
    }
    matrix->postConcat(map_to_unit_vector);

    // Since we've squashed the centers into a unit vector, we must also scale
    // all the coefficient variables by (1 / dCenter).
    const auto coeffA = 1 - dRadius * dRadius / (dCenter * dCenter);
    auto* ctx = alloc->make<SkJumper_2PtConicalCtx>();
    ctx->fCoeffA    = coeffA;
    ctx->fInvCoeffA = 1 / coeffA;
    ctx->fR0        = fRadius1 / dCenter;
    ctx->fDR        = dRadius  / dCenter;

    // Is the solver guaranteed to not produce degenerates?
    bool isWellBehaved = true;

    if (SkScalarNearlyZero(coeffA)) {
        // The focal point is on the edge of the end circle.
        p->append(SkRasterPipeline::xy_to_2pt_conical_linear, ctx);
        isWellBehaved = false;
    } else {
        if (dCenter + fRadius1 > fRadius2) {
            // The focal point is outside the end circle.

            // We want the larger root, per spec:
            //   "For all values of ω where r(ω) > 0, starting with the value of ω nearest
            //    to positive infinity and ending with the value of ω nearest to negative
            //    infinity, draw the circumference of the circle with radius r(ω) at position
            //    (x(ω), y(ω)), with the color at ω, but only painting on the parts of the
            //    bitmap that have not yet been painted on by earlier circles in this step for
            //    this rendering of the gradient."
            // (https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-createradialgradient)
            p->append(fFlippedGrad ? SkRasterPipeline::xy_to_2pt_conical_quadratic_min
                                   : SkRasterPipeline::xy_to_2pt_conical_quadratic_max, ctx);
            isWellBehaved = false;
        } else {
            // The focal point is inside (well-behaved case).
            p->append(SkRasterPipeline::xy_to_2pt_conical_quadratic_max, ctx);
        }
    }

    if (!isWellBehaved) {
        p->append(SkRasterPipeline::mask_2pt_conical_degenerates, ctx);
        postPipeline->append(SkRasterPipeline::apply_vector_mask, &ctx->fMask);
    }

    return true;
}
