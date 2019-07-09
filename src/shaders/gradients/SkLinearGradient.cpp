/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkLinearGradient.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/gradients/Sk4fLinearGradient.h"

static SkMatrix pts_to_unit_matrix(const SkPoint pts[2]) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    SkMatrix matrix;
    matrix.setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix.postTranslate(-pts[0].fX, -pts[0].fY);
    matrix.postScale(inv, inv);
    return matrix;
}

///////////////////////////////////////////////////////////////////////////////

SkLinearGradient::SkLinearGradient(const SkPoint pts[2], const Descriptor& desc)
    : SkGradientShaderBase(desc, pts_to_unit_matrix(pts))
    , fStart(pts[0])
    , fEnd(pts[1]) {
}

sk_sp<SkFlattenable> SkLinearGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::MakeLinear(pts, desc.fColors, std::move(desc.fColorSpace), desc.fPos,
                                        desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkLinearGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    // make sure our colorspaces are compatible with legacy blits
    if (!rec.isLegacyCompatible(fColorSpace.get())) {
        return nullptr;
    }
    // Can't use legacy blit if we can't represent our colors as SkColors
    if (!this->colorsCanConvertToSkColor()) {
        return nullptr;
    }

    return fTileMode != SkTileMode::kDecal
        ? CheckedMakeContext<LinearGradient4fContext>(alloc, *this, rec)
        : nullptr;
}
#endif

void SkLinearGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline*,
                                            SkRasterPipeline*) const {
    // No extra stage needed for linear gradients.
}

SkShader::GradientType SkLinearGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkLinearGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    return GrGradientShader::MakeLinear(*this, args);
}

#endif
