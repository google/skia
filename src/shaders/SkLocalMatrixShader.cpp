/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixProvider.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkLocalMatrixShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#endif

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkLocalMatrixShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    return as_SB(fProxyShader)->asFragmentProcessor(
        GrFPArgs::WithPreLocalMatrix(args, this->getLocalMatrix()));
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
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeFlattenable(fProxyShader.get());
}

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
SkShaderBase::Context* SkLocalMatrixShader::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    SkTCopyOnFirstWrite<SkMatrix> lm(this->getLocalMatrix());
    if (rec.fLocalMatrix) {
        lm.writable()->preConcat(*rec.fLocalMatrix);
    }

    ContextRec newRec(rec);
    newRec.fLocalMatrix = lm;

    return as_SB(fProxyShader)->makeContext(newRec, alloc);
}
#endif

SkImage* SkLocalMatrixShader::onIsAImage(SkMatrix* outMatrix, SkTileMode* mode) const {
    SkMatrix imageMatrix;
    SkImage* image = fProxyShader->isAImage(&imageMatrix, mode);
    if (image && outMatrix) {
        // Local matrix must be applied first so it is on the right side of the concat.
        *outMatrix = SkMatrix::Concat(imageMatrix, this->getLocalMatrix());
    }

    return image;
}

bool SkLocalMatrixShader::onAppendStages(const SkStageRec& rec) const {
    SkTCopyOnFirstWrite<SkMatrix> lm(this->getLocalMatrix());
    if (rec.fLocalM) {
        lm.writable()->preConcat(*rec.fLocalM);
    }

    SkStageRec newRec = rec;
    newRec.fLocalM = lm;
    return as_SB(fProxyShader)->appendStages(newRec);
}


skvm::Color SkLocalMatrixShader::onProgram(skvm::Builder* p,
                                           skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                           const SkMatrixProvider& matrices, const SkMatrix* localM,
                                           const SkColorInfo& dst,
                                           skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    SkTCopyOnFirstWrite<SkMatrix> lm(this->getLocalMatrix());
    if (localM) {
        lm.writable()->preConcat(*localM);
    }
    return as_SB(fProxyShader)->program(p, device,local, paint,
                                        matrices,lm.get(), dst,
                                        uniforms,alloc);
}

sk_sp<SkShader> SkShader::makeWithLocalMatrix(const SkMatrix& localMatrix) const {
    if (localMatrix.isIdentity()) {
        return sk_ref_sp(const_cast<SkShader*>(this));
    }

    const SkMatrix* lm = &localMatrix;

    sk_sp<SkShader> baseShader;
    SkMatrix otherLocalMatrix;
    sk_sp<SkShader> proxy(as_SB(this)->makeAsALocalMatrixShader(&otherLocalMatrix));
    if (proxy) {
        otherLocalMatrix.preConcat(localMatrix);
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

    GradientType asAGradient(GradientInfo* info) const override {
        return fProxyShader->asAGradient(info);
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override { SkASSERT(false); }

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override { return nullptr; }
#endif

    bool onAppendStages(const SkStageRec& rec) const override {
        SkOverrideDeviceMatrixProvider matrixProvider(fCTM);
        SkStageRec newRec = {
            rec.fPipeline,
            rec.fAlloc,
            rec.fDstColorType,
            rec.fDstCS,
            rec.fPaint,
            rec.fLocalM,
            matrixProvider,
        };
        return as_SB(fProxyShader)->appendStages(newRec);
    }

    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider& matrices, const SkMatrix* localM,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        SkOverrideDeviceMatrixProvider matrixProvider(fCTM);
        return as_SB(fProxyShader)->program(p, device,local, paint,
                                            matrixProvider,localM, dst,
                                            uniforms,alloc);
    }

private:
    SK_FLATTENABLE_HOOKS(SkCTMShader)

    sk_sp<SkShader> fProxyShader;
    SkMatrix        fCTM;

    using INHERITED = SkShaderBase;
};


#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkCTMShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkMatrix ctmInv;
    if (!fCTM.invert(&ctmInv)) {
        return nullptr;
    }

    auto ctmProvider = SkOverrideDeviceMatrixProvider(fCTM);
    auto base = as_SB(fProxyShader)->asFragmentProcessor(
        GrFPArgs::WithPreLocalMatrix(args.withNewMatrixProvider(ctmProvider),
                                     this->getLocalMatrix()));
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
