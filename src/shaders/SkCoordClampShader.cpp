/*
 * Copyright 2023 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#if defined(SK_GANESH)
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif // SK_GRAPHITE

class SkShader_CoordClamp final : public SkShaderBase {
public:
    SkShader_CoordClamp(sk_sp<SkShader> shader, const SkRect& subset)
            : fShader(std::move(shader)), fSubset(subset) {}

#if defined(SK_GANESH)
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

protected:
    SkShader_CoordClamp(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const MatrixRec&) const override;
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;

private:
    friend void ::SkRegisterCoordClampShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkShader_CoordClamp)

    sk_sp<SkShader> fShader;
    SkRect fSubset;
};

sk_sp<SkFlattenable> SkShader_CoordClamp::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shader(buffer.readShader());
    SkRect subset = buffer.readRect();
    if (!buffer.validate(SkToBool(shader))) {
        return nullptr;
    }
    return SkShaders::CoordClamp(std::move(shader), subset);
}

void SkShader_CoordClamp::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    buffer.writeRect(fSubset);
}

bool SkShader_CoordClamp::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    std::optional<MatrixRec> childMRec = mRec.apply(rec);
    if (!childMRec.has_value()) {
        return false;
    }
    // Strictly speaking, childMRec's total matrix is not valid. It is only valid inside the subset
    // rectangle. However, we don't mark it as such because we want the "total matrix is valid"
    // behavior in SkImageShader for filtering.
    auto clampCtx = rec.fAlloc->make<SkRasterPipeline_CoordClampCtx>();
    *clampCtx = {fSubset.fLeft, fSubset.fTop, fSubset.fRight, fSubset.fBottom};
    rec.fPipeline->append(SkRasterPipelineOp::clamp_x_and_y, clampCtx);
    return as_SB(fShader)->appendStages(rec, *childMRec);
}

skvm::Color SkShader_CoordClamp::program(skvm::Builder* p,
                                         skvm::Coord device,
                                         skvm::Coord local,
                                         skvm::Color paint,
                                         const MatrixRec& mRec,
                                         const SkColorInfo& cinfo,
                                         skvm::Uniforms* uniforms,
                                         SkArenaAlloc* alloc) const {
    std::optional<MatrixRec> childMRec = mRec.apply(p, &local, uniforms);
    if (!childMRec.has_value()) {
        return {};
    }
    // See comment in appendStages about not marking childMRec with an invalid total matrix.

    auto l = uniforms->pushF(fSubset.left());
    auto t = uniforms->pushF(fSubset.top());
    auto r = uniforms->pushF(fSubset.right());
    auto b = uniforms->pushF(fSubset.bottom());

    local.x = p->clamp(local.x, p->uniformF(l), p->uniformF(r));
    local.y = p->clamp(local.y, p->uniformF(t), p->uniformF(b));

    return as_SB(fShader)->program(p, device, local, paint, *childMRec, cinfo, uniforms, alloc);
}

#if defined(SK_GANESH)
std::unique_ptr<GrFragmentProcessor> SkShader_CoordClamp::asFragmentProcessor(
        const GrFPArgs& args, const MatrixRec& mRec) const {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            "uniform shader c;"
            "uniform float4 s;"
            "half4 main(float2 p) {"
                "return c.eval(clamp(p, s.LT, s.RB));"
            "}");

    auto fp = as_SB(fShader)->asFragmentProcessor(args, mRec.applied());
    if (!fp) {
        return nullptr;
    }

    GrSkSLFP::OptFlags flags = GrSkSLFP::OptFlags::kNone;
    if (fp->compatibleWithCoverageAsAlpha()) {
        flags |= GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha;
    }
    if (fp->preservesOpaqueInput()) {
        flags |= GrSkSLFP::OptFlags::kPreservesOpaqueInput;
    }
    fp = GrSkSLFP::Make(effect,
                        "clamp_fp",
                        /*inputFP=*/nullptr,
                        flags,
                        "c", std::move(fp),
                        "s", fSubset);
    bool success;
    std::tie(success, fp) = mRec.apply(std::move(fp));
    return success ? std::move(fp) : nullptr;
}
#endif  // defined(SK_GANESH)

#if defined(SK_GRAPHITE)
void SkShader_CoordClamp::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    CoordClampShaderBlock::CoordClampData data(fSubset);

    CoordClampShaderBlock::BeginBlock(keyContext, builder, gatherer, &data);
        as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    builder->endBlock();
}
#endif // SK_GRAPHITE

void SkRegisterCoordClampShaderFlattenable() { SK_REGISTER_FLATTENABLE(SkShader_CoordClamp); }

sk_sp<SkShader> SkShaders::CoordClamp(sk_sp<SkShader> shader, const SkRect& subset) {
    if (!shader) {
        return nullptr;
    }
    if (!subset.isSorted()) {
        return nullptr;
    }
    return sk_make_sp<SkShader_CoordClamp>(std::move(shader), subset);
}
