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
#include "src/shaders/gradients/SkSweepGradient.h"

SkSweepGradient::SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1,
                                 const Descriptor& desc)
    : SkGradientShaderBase(desc, SkMatrix::MakeTrans(-center.x(), -center.y()))
    , fCenter(center)
    , fTBias(-t0)
    , fTScale(1 / (t1 - t0))
{
    SkASSERT(t0 < t1);
}

SkShader::GradientType SkSweepGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
    }
    return kSweep_GradientType;
}

static std::tuple<SkScalar, SkScalar> angles_from_t_coeff(SkScalar tBias, SkScalar tScale) {
    return std::make_tuple(-tBias * 360, (sk_ieee_float_divide(1, tScale) - tBias) * 360);
}

sk_sp<SkFlattenable> SkSweepGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();

    SkScalar startAngle = 0,
               endAngle = 360;
    if (!buffer.isVersionLT(SkPicturePriv::kTileInfoInSweepGradient_Version)) {
        const auto tBias  = buffer.readScalar(),
                   tScale = buffer.readScalar();
        std::tie(startAngle, endAngle) = angles_from_t_coeff(tBias, tScale);
    }

    return SkGradientShader::MakeSweep(center.x(), center.y(), desc.fColors,
                                       std::move(desc.fColorSpace), desc.fPos, desc.fCount,
                                       desc.fTileMode, startAngle, endAngle,
                                       desc.fGradFlags, desc.fLocalMatrix);
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fTBias);
    buffer.writeScalar(fTScale);
}

void SkSweepGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                           SkRasterPipeline*) const {
    p->append(SkRasterPipeline::xy_to_unit_angle);
    p->append_matrix(alloc, SkMatrix::Concat(SkMatrix::MakeScale(fTScale, 1),
                                             SkMatrix::MakeTrans(fTBias , 0)));
}


skvm::F32 SkSweepGradient::transformT(skvm::Builder* p, skvm::Uniforms* uniforms,
                                      skvm::F32 x, skvm::F32 y, skvm::I32* mask) const {
    skvm::F32 xabs = p->abs(x),
              yabs = p->abs(y),
             slope = p->div(p->min(xabs, yabs),
                            p->max(xabs, yabs));
    skvm::F32 s = p->mul(slope, slope);

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    const float A = +0.15912117063999176025390625f,
                B = -5.185396969318389892578125e-2f,
                C = +2.476101927459239959716796875e-2f,
                D = -7.0547382347285747528076171875e-3f;
    skvm::F32 phi = p->mul(slope, p->mad(s,
                                  p->mad(s,
                                  p->mad(s, p->splat(D),
                                            p->splat(C)),
                                            p->splat(B)),
                                            p->splat(A)));
    phi = p->select(p->lt (xabs, yabs)       , p->sub(p->splat(1/4.0f), phi), phi);
    phi = p->select(p->lt (x, p->splat(0.0f)), p->sub(p->splat(1/2.0f), phi), phi);
    phi = p->select(p->lt (y, p->splat(0.0f)), p->sub(p->splat(1/1.0f), phi), phi);

    skvm::F32 t = p->bit_cast(p->bit_and(p->bit_cast(phi),   // t = phi if phi != NaN
                                         p->eq(phi, phi)));

    if (fTScale != 1.0f || fTBias != 0.0f) {
        t = p->mad(t, p->uniformF(uniforms->pushF(fTScale))
                    , p->uniformF(uniforms->pushF(fTScale*fTBias)));
    }
    return t;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkSweepGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    return GrGradientShader::MakeSweep(*this, args);
}

#endif
