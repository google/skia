/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFloatingPoint.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkSweepGradient.h"
#include "SkWriteBuffer.h"

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
    if (!buffer.isVersionLT(SkReadBuffer::kTileInfoInSweepGradient_Version)) {
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

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkSweepGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    return GrGradientShader::MakeSweep(*this, args);
}

#endif
