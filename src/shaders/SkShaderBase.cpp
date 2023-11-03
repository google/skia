/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkShaderBase.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/private/SkColorData.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/shaders/SkLocalMatrixShader.h"

#include <cstring>

class SkWriteBuffer;

namespace SkShaders {
MatrixRec::MatrixRec(const SkMatrix& ctm) : fCTM(ctm) {}

std::optional<MatrixRec> MatrixRec::apply(const SkStageRec& rec, const SkMatrix& postInv) const {
    SkMatrix total = fPendingLocalMatrix;
    if (!fCTMApplied) {
        total = SkMatrix::Concat(fCTM, total);
    }
    if (!total.invert(&total)) {
        return {};
    }
    total = SkMatrix::Concat(postInv, total);
    if (!fCTMApplied) {
        rec.fPipeline->append(SkRasterPipelineOp::seed_shader);
    }
    // appendMatrix is a no-op if total worked out to identity.
    rec.fPipeline->appendMatrix(rec.fAlloc, total);
    return MatrixRec{fCTM,
                     fTotalLocalMatrix,
                     /*pendingLocalMatrix=*/SkMatrix::I(),
                     fTotalMatrixIsValid,
                     /*ctmApplied=*/true};
}

std::tuple<SkMatrix, bool> MatrixRec::applyForFragmentProcessor(const SkMatrix& postInv) const {
    SkASSERT(!fCTMApplied);
    SkMatrix total;
    if (!fPendingLocalMatrix.invert(&total)) {
        return {SkMatrix::I(), false};
    }
    return {SkMatrix::Concat(postInv, total), true};
}

MatrixRec MatrixRec::applied() const {
    // We mark the CTM as "not applied" because we *never* apply the CTM for FPs. Their starting
    // coords are local, not device, coords.
    return MatrixRec{fCTM,
                     fTotalLocalMatrix,
                     /*pendingLocalMatrix=*/SkMatrix::I(),
                     fTotalMatrixIsValid,
                     /*ctmApplied=*/false};
}

MatrixRec MatrixRec::concat(const SkMatrix& m) const {
    return {fCTM,
            SkShaderBase::ConcatLocalMatrices(fTotalLocalMatrix, m),
            SkShaderBase::ConcatLocalMatrices(fPendingLocalMatrix, m),
            fTotalMatrixIsValid,
            fCTMApplied};
}

}  // namespace SkShaders

///////////////////////////////////////////////////////////////////////////////////////

SkShaderBase::SkShaderBase() = default;

SkShaderBase::~SkShaderBase() = default;

void SkShaderBase::flatten(SkWriteBuffer& buffer) const { this->INHERITED::flatten(buffer); }

bool SkShaderBase::asLuminanceColor(SkColor* colorPtr) const {
    SkColor storage;
    if (nullptr == colorPtr) {
        colorPtr = &storage;
    }
    if (this->onAsLuminanceColor(colorPtr)) {
        *colorPtr = SkColorSetA(*colorPtr, 0xFF);  // we only return opaque
        return true;
    }
    return false;
}

SkShaderBase::Context* SkShaderBase::makeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    // We always fall back to raster pipeline when perspective is present.
    auto totalMatrix = rec.fMatrixRec.totalMatrix();
    if (totalMatrix.hasPerspective() || !totalMatrix.invert(nullptr)) {
        return nullptr;
    }

    return this->onMakeContext(rec, alloc);
#else
    return nullptr;
#endif
}

SkShaderBase::Context::Context(const SkShaderBase& shader, const ContextRec& rec)
        : fShader(shader) {
    // We should never use a context with perspective.
    SkASSERT(!rec.fMatrixRec.totalMatrix().hasPerspective());

    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(rec.fMatrixRec.totalInverse(&fTotalInverse));

    fPaintAlpha = rec.fPaintAlpha;
}

SkShaderBase::Context::~Context() {}

bool SkShaderBase::ContextRec::isLegacyCompatible(SkColorSpace* shaderColorSpace) const {
    // In legacy pipelines, shaders always produce premul (or opaque) and the destination is also
    // always premul (or opaque).  (And those "or opaque" caveats won't make any difference here.)
    SkAlphaType shaderAT = kPremul_SkAlphaType, dstAT = kPremul_SkAlphaType;
    return 0 ==
           SkColorSpaceXformSteps{shaderColorSpace, shaderAT, fDstColorSpace, dstAT}.flags.mask();
}

sk_sp<SkShader> SkShaderBase::makeAsALocalMatrixShader(SkMatrix*) const { return nullptr; }

bool SkShaderBase::appendRootStages(const SkStageRec& rec, const SkMatrix& ctm) const {
    return this->appendStages(rec, SkShaders::MatrixRec(ctm));
}

bool SkShaderBase::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const {
    // SkShader::Context::shadeSpan() handles the paint opacity internally,
    // but SkRasterPipelineBlitter applies it as a separate stage.
    // We skip the internal shadeSpan() step by forcing the alpha to be opaque.
    ContextRec cr(SK_AlphaOPAQUE, mRec, rec.fDstColorType, sk_srgb_singleton(), rec.fSurfaceProps);

    struct CallbackCtx : SkRasterPipeline_CallbackCtx {
        sk_sp<const SkShader> shader;
        Context* ctx;
    };
    auto cb = rec.fAlloc->make<CallbackCtx>();
    cb->shader = sk_ref_sp(this);
    cb->ctx = as_SB(this)->makeContext(cr, rec.fAlloc);
    cb->fn = [](SkRasterPipeline_CallbackCtx* self, int active_pixels) {
        auto c = (CallbackCtx*)self;
        int x = (int)c->rgba[0], y = (int)c->rgba[1];
        SkPMColor tmp[SkRasterPipeline_kMaxStride_highp];
        c->ctx->shadeSpan(x, y, tmp, active_pixels);

        for (int i = 0; i < active_pixels; i++) {
            auto rgba_4f = SkPMColor4f::FromPMColor(tmp[i]);
            memcpy(c->rgba + 4 * i, rgba_4f.vec(), 4 * sizeof(float));
        }
    };

    if (cb->ctx) {
        rec.fPipeline->append(SkRasterPipelineOp::seed_shader);
        rec.fPipeline->append(SkRasterPipelineOp::callback, cb);
        rec.fAlloc
                ->make<SkColorSpaceXformSteps>(
                        sk_srgb_singleton(), kPremul_SkAlphaType, rec.fDstCS, kPremul_SkAlphaType)
                ->apply(rec.fPipeline);
        return true;
    }
    return false;
}

sk_sp<SkShader> SkShaderBase::makeWithCTM(const SkMatrix& postM) const {
    return sk_sp<SkShader>(new SkCTMShader(sk_ref_sp(this), postM));
}

// need a cheap way to invert the alpha channel of a shader (i.e. 1 - a)
sk_sp<SkShader> SkShaderBase::makeInvertAlpha() const {
    return this->makeWithColorFilter(SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
}
