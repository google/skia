/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkLocalMatrixShader.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include "src/shaders/gradients/SkGradientShaderBase.h"

class SkSweepGradient final : public SkGradientShaderBase {
public:
    SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1, const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::Coord coord, skvm::I32* mask) const final;
private:
    friend void ::SkRegisterSweepGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkSweepGradient)

    const SkPoint  fCenter;
    const SkScalar fTBias;
    const SkScalar fTScale;
};

SkSweepGradient::SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1,
                                 const Descriptor& desc)
    : SkGradientShaderBase(desc, SkMatrix::Translate(-center.x(), -center.y()))
    , fCenter(center)
    , fTBias(-t0)
    , fTScale(1 / (t1 - t0))
{
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
    this->SkGradientShaderBase::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fTBias);
    buffer.writeScalar(fTScale);
}

void SkSweepGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                           SkRasterPipeline*) const {
    p->append(SkRasterPipelineOp::xy_to_unit_angle);
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

#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/gradients/GrGradientShader.h"

std::unique_ptr<GrFragmentProcessor> SkSweepGradient::asFragmentProcessor(
        const GrFPArgs& args, const MatrixRec& mRec) const {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in (sqrt(x^2
    // + y^2) + x) as the second parameter to atan2 in these cases. We let the device handle the
    // undefined behavior of the second paramenter being 0 instead of doing the divide ourselves and
    // using atan instead.
    int useAtanWorkaround =
            args.fContext->priv().caps()->shaderCaps()->fAtan2ImplementedAsAtanYOverX;
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform half bias;"
        "uniform half scale;"
        "uniform int useAtanWorkaround;"  // specialized

        "half4 main(float2 coord) {"
            "half angle = bool(useAtanWorkaround)"
                    "? half(2 * atan(-coord.y, length(coord) - coord.x))"
                    ": half(atan(-coord.y, -coord.x));"

            // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
            "half t = (angle * 0.1591549430918 + 0.5 + bias) * scale;"
            "return half4(t, 1, 0, 0);" // y = 1 for always valid
        "}"
    );

    // The sweep gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "SweepLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput,
                             "bias", fTBias,
                             "scale", fTScale,
                             "useAtanWorkaround", GrSkSLFP::Specialize(useAtanWorkaround));
    return GrGradientShader::MakeGradientFP(*this, args, mRec, std::move(fp));
}

#endif

#ifdef SK_GRAPHITE_ENABLED
void SkSweepGradient::addToKey(const skgpu::graphite::KeyContext& keyContext,
                               skgpu::graphite::PaintParamsKeyBuilder* builder,
                               skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkColor4fXformer xformedColors(this, keyContext.dstColorInfo().colorSpace());
    const SkPMColor4f* colors = xformedColors.fColors.begin();

    GradientShaderBlocks::GradientData data(SkShaderBase::GradientType::kSweep,
                                            fCenter, { 0.0f, 0.0f },
                                            0.0, 0.0f,
                                            fTBias, fTScale,
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
    if (!SkGradientShaderBase::ValidGradient(colors, colorCount, mode, interpolation)) {
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

    if (SkScalarNearlyEqual(startAngle, endAngle, SkGradientShaderBase::kDegenerateThreshold)) {
        // Degenerate gradient, which should follow default degenerate behavior unless it is
        // clamped and the angle is greater than 0.
        if (mode == SkTileMode::kClamp && endAngle > SkGradientShaderBase::kDegenerateThreshold) {
            // In this case, the first color is repeated from 0 to the angle, then a hardstop
            // switches to the last color (all other colors are compressed to the infinitely thin
            // interpolation region).
            static constexpr SkScalar clampPos[3] = {0, 1, 1};
            SkColor4f reColors[3] = {colors[0], colors[0], colors[colorCount - 1]};
            return MakeSweep(cx, cy, reColors, std::move(colorSpace), clampPos, 3, mode, 0,
                             endAngle, interpolation, localMatrix);
        } else {
            return SkGradientShaderBase::MakeDegenerateGradient(colors, pos, colorCount,
                                                                std::move(colorSpace), mode);
        }
    }

    if (startAngle <= 0 && endAngle >= 360) {
        // If the t-range includes [0,1], then we can always use clamping (presumably faster).
        mode = SkTileMode::kClamp;
    }

    SkGradientShaderBase::ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc(opt.fColors, std::move(colorSpace), opt.fPos,
                                          opt.fCount, mode, interpolation);

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
