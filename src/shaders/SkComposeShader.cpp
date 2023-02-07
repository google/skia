/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

class SkShader_Blend final : public SkShaderBase {
public:
    SkShader_Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src)
            : fDst(std::move(dst))
            , fSrc(std::move(src))
            , fMode(mode) {}

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
    SkShader_Blend(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    bool appendStages(const SkStageRec&, const MatrixRec&) const override;
    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec& mRec,
                        const SkColorInfo& dst,
                        skvm::Uniforms*,
                        SkArenaAlloc*) const override;

private:
    friend void ::SkRegisterComposeShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkShader_Blend)

    sk_sp<SkShader>     fDst;
    sk_sp<SkShader>     fSrc;
    SkBlendMode         fMode;

    using INHERITED = SkShaderBase;
};

sk_sp<SkFlattenable> SkShader_Blend::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> dst(buffer.readShader());
    sk_sp<SkShader> src(buffer.readShader());
    if (!buffer.validate(dst && src)) {
        return nullptr;
    }

    unsigned mode = buffer.read32();

    if (mode == kCustom_SkBlendMode) {
        sk_sp<SkBlender> blender = buffer.readBlender();
        if (buffer.validate(blender != nullptr)) {
            return SkShaders::Blend(std::move(blender), std::move(dst), std::move(src));
        }
    } else {
        if (buffer.validate(mode <= (unsigned)SkBlendMode::kLastMode)) {
            return SkShaders::Blend(static_cast<SkBlendMode>(mode), std::move(dst), std::move(src));
        }
    }
    return nullptr;
}

void SkShader_Blend::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32((int)fMode);
}

// Returns the output of e0, and leaves the output of e1 in r,g,b,a
static float* append_two_shaders(const SkStageRec& rec,
                                 const SkShaderBase::MatrixRec& mRec,
                                 SkShader* s0,
                                 SkShader* s1) {
    struct Storage {
        float   fCoords[2 * SkRasterPipeline_kMaxStride];
        float   fRes0  [4 * SkRasterPipeline_kMaxStride];
    };
    auto storage = rec.fAlloc->make<Storage>();

    // Note we cannot simply apply mRec here and then unconditionally store the coordinates. When
    // building for Android Framework it would interrupt the backwards local matrix concatenation if
    // mRec had a pending local matrix and either of the children also had a local matrix.
    // b/256873449
    if (mRec.rasterPipelineCoordsAreSeeded()) {
        rec.fPipeline->append(SkRasterPipelineOp::store_src_rg, storage->fCoords);
    }
    if (!as_SB(s0)->appendStages(rec, mRec)) {
        return nullptr;
    }
    rec.fPipeline->append(SkRasterPipelineOp::store_src, storage->fRes0);

    if (mRec.rasterPipelineCoordsAreSeeded()) {
        rec.fPipeline->append(SkRasterPipelineOp::load_src_rg, storage->fCoords);
    }
    if (!as_SB(s1)->appendStages(rec, mRec)) {
        return nullptr;
    }
    return storage->fRes0;
}

bool SkShader_Blend::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    float* res0 = append_two_shaders(rec, mRec, fDst.get(), fSrc.get());
    if (!res0) {
        return false;
    }

    rec.fPipeline->append(SkRasterPipelineOp::load_dst, res0);
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

skvm::Color SkShader_Blend::program(skvm::Builder* p,
                                    skvm::Coord device,
                                    skvm::Coord local,
                                    skvm::Color paint,
                                    const MatrixRec& mRec,
                                    const SkColorInfo& cinfo,
                                    skvm::Uniforms* uniforms,
                                    SkArenaAlloc* alloc) const {
    skvm::Color d,s;
    if ((d = as_SB(fDst)->program(p, device, local, paint, mRec, cinfo, uniforms, alloc)) &&
        (s = as_SB(fSrc)->program(p, device, local, paint, mRec, cinfo, uniforms, alloc))) {
        return p->blend(fMode, s,d);
    }
    return {};
}

#if SK_SUPPORT_GPU

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor>
SkShader_Blend::asFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    auto fpA = as_SB(fDst)->asFragmentProcessor(args, mRec);
    auto fpB = as_SB(fSrc)->asFragmentProcessor(args, mRec);
    if (!fpA || !fpB) {
        // This is unexpected. Both src and dst shaders should be valid. Just fail.
        return nullptr;
    }
    return GrBlendFragmentProcessor::Make(std::move(fpB), std::move(fpA), fMode);
}
#endif

#ifdef SK_GRAPHITE_ENABLED
void SkShader_Blend::addToKey(const skgpu::graphite::KeyContext& keyContext,
                              skgpu::graphite::PaintParamsKeyBuilder* builder,
                              skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SkSpan<const float> porterDuffConstants = skgpu::GetPorterDuffBlendConstants(fMode);
    if (!porterDuffConstants.empty()) {
        PorterDuffBlendShaderBlock::BeginBlock(keyContext, builder, gatherer,
                                               {porterDuffConstants});
    } else {
        BlendShaderBlock::BeginBlock(keyContext, builder, gatherer, {fMode});
    }

    as_SB(fDst)->addToKey(keyContext, builder, gatherer);
    as_SB(fSrc)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();
}
#endif

sk_sp<SkShader> SkShaders::Blend(SkBlendMode mode, sk_sp<SkShader> dst, sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    switch (mode) {
        case SkBlendMode::kClear: return Color(0);
        case SkBlendMode::kDst:   return dst;
        case SkBlendMode::kSrc:   return src;
        default: break;
    }
    return sk_sp<SkShader>(new SkShader_Blend(mode, std::move(dst), std::move(src)));
}

sk_sp<SkShader> SkShaders::Blend(sk_sp<SkBlender> blender,
                                 sk_sp<SkShader> dst,
                                 sk_sp<SkShader> src) {
    if (!src || !dst) {
        return nullptr;
    }
    if (!blender) {
        return SkShaders::Blend(SkBlendMode::kSrcOver, std::move(dst), std::move(src));
    }
    if (std::optional<SkBlendMode> mode = as_BB(blender)->asBlendMode()) {
        return sk_make_sp<SkShader_Blend>(mode.value(), std::move(dst), std::move(src));
    }

#ifdef SK_ENABLE_SKSL
    // This isn't a built-in blend mode; we might as well use a runtime effect to evaluate it.
    static SkRuntimeEffect* sBlendEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform blender b;"
        "uniform shader d, s;"
        "half4 main(float2 xy) {"
            "return b.eval(s.eval(xy), d.eval(xy));"
        "}"
    );
    SkRuntimeEffect::ChildPtr children[] = {std::move(blender), std::move(dst), std::move(src)};
    return sBlendEffect->makeShader(/*uniforms=*/{}, children);
#else
    // We need SkSL to render this blend.
    return nullptr;
#endif
}

void SkRegisterComposeShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkShader_Blend);
}
