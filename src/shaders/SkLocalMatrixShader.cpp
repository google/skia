/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkTLazy.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkLocalMatrixShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#endif

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

SkShaderBase::GradientType SkLocalMatrixShader::asGradient(GradientInfo* info,
                                                           SkMatrix* localMatrix) const {
    GradientType type = as_SB(fWrappedShader)->asGradient(info, localMatrix);
    if (type != SkShaderBase::GradientType::kNone && localMatrix) {
        *localMatrix = ConcatLocalMatrices(fLocalMatrix, *localMatrix);
    }
    return type;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkLocalMatrixShader::asFragmentProcessor(
        const GrFPArgs& args, const MatrixRec& mRec) const {
    return as_SB(fWrappedShader)->asFragmentProcessor(args, mRec.concat(fLocalMatrix));
}
#endif

#ifdef SK_GRAPHITE_ENABLED
void SkLocalMatrixShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    LocalMatrixShaderBlock::LMShaderData lmShaderData(fLocalMatrix);

    KeyContextWithLocalMatrix newContext(keyContext, fLocalMatrix);

    LocalMatrixShaderBlock::BeginBlock(newContext, builder, gatherer, &lmShaderData);

    as_SB(fWrappedShader)->addToKey(newContext, builder, gatherer);

    builder->endBlock();
}
#endif

sk_sp<SkFlattenable> SkLocalMatrixShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    auto baseShader(buffer.readShader());
    if (!baseShader) {
        return nullptr;
    }
    return baseShader->makeWithLocalMatrix(lm);
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(fLocalMatrix);
    buffer.writeFlattenable(fWrappedShader.get());
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkLocalMatrixShader::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    SkTCopyOnFirstWrite<SkMatrix> lm(fLocalMatrix);
    if (rec.fLocalMatrix) {
        *lm.writable() = ConcatLocalMatrices(*rec.fLocalMatrix, *lm);
    }

    ContextRec newRec(rec);
    newRec.fLocalMatrix = lm;

    return as_SB(fWrappedShader)->makeContext(newRec, alloc);
}
#endif

SkImage* SkLocalMatrixShader::onIsAImage(SkMatrix* outMatrix, SkTileMode* mode) const {
    SkMatrix imageMatrix;
    SkImage* image = fWrappedShader->isAImage(&imageMatrix, mode);
    if (image && outMatrix) {
        *outMatrix = ConcatLocalMatrices(fLocalMatrix, imageMatrix);
    }

    return image;
}

bool SkLocalMatrixShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    return as_SB(fWrappedShader)->appendStages(rec, mRec.concat(fLocalMatrix));
}

skvm::Color SkLocalMatrixShader::program(skvm::Builder* p,
                                         skvm::Coord device,
                                         skvm::Coord local,
                                         skvm::Color paint,
                                         const MatrixRec& mRec,
                                         const SkColorInfo& dst,
                                         skvm::Uniforms* uniforms,
                                         SkArenaAlloc* alloc) const {
    return as_SB(fWrappedShader)->program(p,
                                          device,
                                          local,
                                          paint,
                                          mRec.concat(fLocalMatrix),
                                          dst,
                                          uniforms,
                                          alloc);
}

sk_sp<SkShader> SkShader::makeWithLocalMatrix(const SkMatrix& localMatrix) const {
    if (localMatrix.isIdentity()) {
        return sk_ref_sp(const_cast<SkShader*>(this));
    }

    const SkMatrix* lm = &localMatrix;

    sk_sp<SkShader> baseShader;
    SkMatrix otherLocalMatrix;
    sk_sp<SkShader> proxy = as_SB(this)->makeAsALocalMatrixShader(&otherLocalMatrix);
    if (proxy) {
        otherLocalMatrix = SkShaderBase::ConcatLocalMatrices(localMatrix, otherLocalMatrix);
        lm = &otherLocalMatrix;
        baseShader = proxy;
    } else {
        baseShader = sk_ref_sp(const_cast<SkShader*>(this));
    }

    return sk_make_sp<SkLocalMatrixShader>(std::move(baseShader), *lm);
}

////////////////////////////////////////////////////////////////////

/**
 *  Replaces the CTM when used. Created to support clipShaders, which have to be evaluated
 *  using the CTM that was present at the time they were specified (which may be different
 *  from the CTM at the time something is drawn through the clip.
 */
class SkCTMShader final : public SkShaderBase {
public:
    SkCTMShader(sk_sp<SkShader> proxy, const SkMatrix& ctm)
    : fProxyShader(std::move(proxy))
    , fCTM(ctm)
    {}

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override {
        return as_SB(fProxyShader)->asGradient(info, localMatrix);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override { SkASSERT(false); }

    bool appendStages(const SkStageRec& rec, const MatrixRec&) const override {
        return as_SB(fProxyShader)->appendRootStages(rec, fCTM);
    }

    skvm::Color program(skvm::Builder* p,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec& mRec,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override {
        return as_SB(fProxyShader)->rootProgram(p, device, paint, fCTM, dst, uniforms, alloc);
    }

private:
    SK_FLATTENABLE_HOOKS(SkCTMShader)

    sk_sp<SkShader> fProxyShader;
    SkMatrix        fCTM;

    using INHERITED = SkShaderBase;
};


#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkCTMShader::asFragmentProcessor(const GrFPArgs& args,
                                                                      const MatrixRec& mRec) const {
    SkMatrix ctmInv;
    if (!fCTM.invert(&ctmInv)) {
        return nullptr;
    }

    auto base = as_SB(fProxyShader)->asRootFragmentProcessor(args, fCTM);
    if (!base) {
        return nullptr;
    }

    // In order for the shader to be evaluated with the original CTM, we explicitly evaluate it
    // at sk_FragCoord, and pass that through the inverse of the original CTM. This avoids requiring
    // local coords for the shader and mapping from the draw's local to device and then back.
    return GrFragmentProcessor::DeviceSpace(GrMatrixEffect::Make(ctmInv, std::move(base)));
}
#endif

sk_sp<SkFlattenable> SkCTMShader::CreateProc(SkReadBuffer& buffer) {
    SkASSERT(false);
    return nullptr;
}

sk_sp<SkShader> SkShaderBase::makeWithCTM(const SkMatrix& postM) const {
    return sk_sp<SkShader>(new SkCTMShader(sk_ref_sp(this), postM));
}
