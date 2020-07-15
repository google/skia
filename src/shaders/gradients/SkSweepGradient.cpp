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
    : SkGradientShaderBase(desc, SkMatrix::Translate(-center.x(), -center.y()))
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

    const auto tBias  = buffer.readScalar(),
               tScale = buffer.readScalar();
    auto [startAngle, endAngle] = angles_from_t_coeff(tBias, tScale);

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
    p->append_matrix(alloc, SkMatrix::Scale(fTScale, 1) * SkMatrix::Translate(fTBias, 0));
}

skvm::F32 SkSweepGradient::transformT(skvm::Builder* p, skvm::Uniforms* uniforms,
                                      skvm::Coord coord, skvm::I32* mask) const {
    skvm::F32 xabs = abs(coord.x),
              yabs = abs(coord.y),
             slope = min(xabs, yabs) / max(xabs, yabs);
    skvm::F32 s = slope * slope;

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    skvm::F32 phi = slope * poly(s, -7.0547382347285747528076171875e-3f,
                                    +2.476101927459239959716796875e-2f,
                                    -5.185396969318389892578125e-2f,
                                    +0.15912117063999176025390625f);
    phi = select(   xabs < yabs, (1/4.0f) - phi, phi);
    phi = select(coord.x < 0.0f, (1/2.0f) - phi, phi);
    phi = select(coord.y < 0.0f, (1/1.0f) - phi, phi);

    skvm::F32 t = select(is_NaN(phi), p->splat(0.0f)
                                    , phi);

    if (fTScale != 1.0f || fTBias != 0.0f) {
        t = t * p->uniformF(uniforms->pushF(fTScale))
              + p->uniformF(uniforms->pushF(fTScale*fTBias));
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
