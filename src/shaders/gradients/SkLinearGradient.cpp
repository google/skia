/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkLinearGradient.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

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
    SkMatrix legacyLocalMatrix;
    if (!desc.unflatten(buffer, &legacyLocalMatrix)) {
        return nullptr;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::MakeLinear(pts,
                                        desc.fColors,
                                        std::move(desc.fColorSpace),
                                        desc.fPositions,
                                        desc.fColorCount,
                                        desc.fTileMode,
                                        desc.fInterpolation,
                                        &legacyLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

void SkLinearGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline*,
                                            SkRasterPipeline*) const {
    // No extra stage needed for linear gradients.
}

skvm::F32 SkLinearGradient::transformT(skvm::Builder* p, skvm::Uniforms*,
                                       skvm::Coord coord, skvm::I32* mask) const {
    // We've baked getting t in x into the matrix, so this is pretty trivial.
    return coord.x;
}

SkShaderBase::GradientType SkLinearGradient::asGradient(GradientInfo* info,
                                                        SkMatrix* localMatrix) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kLinear;
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "src/gpu/ganesh/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkLinearGradient::asFragmentProcessor(
        const GrFPArgs& args, const MatrixRec& mRec) const {
    return GrGradientShader::MakeLinear(*this, args, mRec);
}

#endif

#ifdef SK_GRAPHITE_ENABLED
void SkLinearGradient::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                skgpu::graphite::PaintParamsKeyBuilder* builder,
                                skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkColor4fXformer xformedColors(this, keyContext.dstColorInfo().colorSpace());
    const SkPMColor4f* colors = xformedColors.fColors.begin();

    GradientShaderBlocks::GradientData data(GradientType::kLinear,
                                            fStart, fEnd,
                                            0.0f, 0.0f,
                                            0.0f, 0.0f,
                                            fTileMode,
                                            fColorCount,
                                            colors,
                                            fPositions,
                                            fInterpolation);

    MakeInterpolatedToDst(keyContext, builder, gatherer,
                          data, fInterpolation,
                          xformedColors.fIntermediateColorSpace.get());
}
#endif

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[],
                                             int colorCount,
                                             SkTileMode mode,
                                             const Interpolation& interpolation,
                                             const SkMatrix* localMatrix) {
    if (!pts || !SkScalarIsFinite((pts[1] - pts[0]).length())) {
        return nullptr;
    }
    if (!SkGradientShaderBase::ValidGradient(colors, colorCount, mode, interpolation)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShaders::Color(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    if (SkScalarNearlyZero((pts[1] - pts[0]).length(),
                           SkGradientShaderBase::kDegenerateThreshold)) {
        // Degenerate gradient, the only tricky complication is when in clamp mode, the limit of
        // the gradient approaches two half planes of solid color (first and last). However, they
        // are divided by the line perpendicular to the start and end point, which becomes undefined
        // once start and end are exactly the same, so just use the end color for a stable solution.
        return SkGradientShaderBase::MakeDegenerateGradient(colors, pos, colorCount,
                                                            std::move(colorSpace), mode);
    }

    SkGradientShaderBase::ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc(opt.fColors, std::move(colorSpace), opt.fPos,
                                          opt.fCount, mode, interpolation);
    return SkLocalMatrixShader::MakeWrapped<SkLinearGradient>(localMatrix, pts, desc);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor colors[],
                                             const SkScalar pos[],
                                             int colorCount,
                                             SkTileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    SkColorConverter converter(colors, colorCount);
    return MakeLinear(pts, converter.fColors4f.begin(), nullptr, pos, colorCount, mode, flags,
                      localMatrix);
}

void SkRegisterLinearGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkLinearGradient);
}
