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
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/shaders/SkLocalMatrixShader.h"

#include <atomic>

class SkWriteBuffer;

namespace {

// The 32-bit shader ID counter is not expected to wrap. In the unlikely event that it does, we
// assume that the associated shaders will be sufficiently temporally separated such that shaders
// with the same recycled IDs are no longer in use.
uint32_t next_unique_id() {
    static std::atomic<uint32_t> gNextUniqueID{SK_InvalidUniqueID + 1};
    uint32_t id = SK_InvalidUniqueID;
    do {
        id = gNextUniqueID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}

} // anonymous namespace

namespace SkShaders {
MatrixRec::MatrixRec(const SkMatrix& ctm) : fCTM(ctm) {}

std::optional<MatrixRec> MatrixRec::apply(const SkStageRec& rec, const SkMatrix& postInv) const {
    SkMatrix total = fPendingLocalMatrix;
    if (!fCTMApplied) {
        total = SkMatrix::Concat(fCTM, total);
    }
    if (auto inv = total.invert()) {
        total = SkMatrix::Concat(postInv, *inv);
    } else {
        return {};
    }
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
    if (auto total = fPendingLocalMatrix.invert()) {
        return {SkMatrix::Concat(postInv, *total), true};
    } else {
        return {SkMatrix::I(), false};
    }
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

SkShaderBase::SkShaderBase() : fUniqueID(next_unique_id()) {}

SkShaderBase::~SkShaderBase() = default;

void SkShaderBase::flatten(SkWriteBuffer& buffer) const { this->INHERITED::flatten(buffer); }

bool SkShaderBase::asLuminanceColor(SkColor4f* colorPtr) const {
    SkColor4f storage;
    if (nullptr == colorPtr) {
        colorPtr = &storage;
    }
    if (this->onAsLuminanceColor(colorPtr)) {
        colorPtr->fA = 1.0f;  // we only return opaque
        return true;
    }
    return false;
}

SkShaderBase::Context* SkShaderBase::makeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    // We always fall back to raster pipeline when perspective is present.
    auto totalMatrix = rec.fMatrixRec.totalMatrix();
    if (totalMatrix.hasPerspective() || !totalMatrix.invert()) {
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

    fPaintAlpha = rec.fPaintAlpha;
}

SkShaderBase::Context::~Context() {}

bool SkShaderBase::ContextRec::isLegacyCompatible(SkColorSpace* shaderColorSpace) const {
    // In legacy pipelines, shaders always produce premul (or opaque) and the destination is also
    // always premul (or opaque).  (And those "or opaque" caveats won't make any difference here.)
    SkAlphaType shaderAT = kPremul_SkAlphaType, dstAT = kPremul_SkAlphaType;
    return 0 ==
           SkColorSpaceXformSteps{shaderColorSpace, shaderAT, fDstColorSpace, dstAT}.fFlags.mask();
}

sk_sp<SkShader> SkShaderBase::makeAsALocalMatrixShader(SkMatrix*) const { return nullptr; }

bool SkShaderBase::appendRootStages(const SkStageRec& rec, const SkMatrix& ctm) const {
    return this->appendStages(rec, SkShaders::MatrixRec(ctm));
}

sk_sp<SkShader> SkShaderBase::makeWithCTM(const SkMatrix& postM) const {
    return sk_sp<SkShader>(new SkCTMShader(sk_ref_sp(this), postM));
}

// need a cheap way to invert the alpha channel of a shader (i.e. 1 - a)
sk_sp<SkShader> SkShaderBase::makeInvertAlpha() const {
    return this->makeWithColorFilter(SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
}
