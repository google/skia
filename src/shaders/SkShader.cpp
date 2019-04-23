/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkScalar.h"
#include "include/private/SkArenaAlloc.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkPictureShader.h"
#include "src/shaders/SkShaderBase.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif

SkShaderBase::SkShaderBase(const SkMatrix* localMatrix)
    : fLocalMatrix(localMatrix ? *localMatrix : SkMatrix::I()) {
    // Pre-cache so future calls to fLocalMatrix.getType() are threadsafe.
    (void)fLocalMatrix.getType();
}

SkShaderBase::~SkShaderBase() {}

void SkShaderBase::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    bool hasLocalM = !fLocalMatrix.isIdentity();
    buffer.writeBool(hasLocalM);
    if (hasLocalM) {
        buffer.writeMatrix(fLocalMatrix);
    }
}

SkTCopyOnFirstWrite<SkMatrix>
SkShaderBase::totalLocalMatrix(const SkMatrix* preLocalMatrix,
                               const SkMatrix* postLocalMatrix) const {
    SkTCopyOnFirstWrite<SkMatrix> m(fLocalMatrix);

    if (preLocalMatrix) {
        m.writable()->preConcat(*preLocalMatrix);
    }

    if (postLocalMatrix) {
        m.writable()->postConcat(*postLocalMatrix);
    }

    return m;
}

bool SkShaderBase::computeTotalInverse(const SkMatrix& ctm,
                                       const SkMatrix* outerLocalMatrix,
                                       SkMatrix* totalInverse) const {
    return SkMatrix::Concat(ctm, *this->totalLocalMatrix(outerLocalMatrix)).invert(totalInverse);
}

bool SkShaderBase::asLuminanceColor(SkColor* colorPtr) const {
    SkColor storage;
    if (nullptr == colorPtr) {
        colorPtr = &storage;
    }
    if (this->onAsLuminanceColor(colorPtr)) {
        *colorPtr = SkColorSetA(*colorPtr, 0xFF);   // we only return opaque
        return true;
    }
    return false;
}

SkShaderBase::Context* SkShaderBase::makeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    // We always fall back to raster pipeline when perspective is present.
    if (rec.fMatrix->hasPerspective() ||
        fLocalMatrix.hasPerspective() ||
        (rec.fLocalMatrix && rec.fLocalMatrix->hasPerspective()) ||
        !this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, nullptr)) {
        return nullptr;
    }

    return this->onMakeContext(rec, alloc);
#else
    return nullptr;
#endif
}

SkShaderBase::Context::Context(const SkShaderBase& shader, const ContextRec& rec)
    : fShader(shader), fCTM(*rec.fMatrix)
{
    // We should never use a context with perspective.
    SkASSERT(!rec.fMatrix->hasPerspective());
    SkASSERT(!rec.fLocalMatrix || !rec.fLocalMatrix->hasPerspective());
    SkASSERT(!shader.getLocalMatrix().hasPerspective());

    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(fShader.computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &fTotalInverse));

    fPaintAlpha = rec.fPaint->getAlpha();
}

SkShaderBase::Context::~Context() {}

bool SkShaderBase::ContextRec::isLegacyCompatible(SkColorSpace* shaderColorSpace) const {
    return !SkColorSpaceXformSteps::Required(shaderColorSpace, fDstColorSpace);
}

SkImage* SkShader::isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const {
    return as_SB(this)->onIsAImage(localMatrix, xy);
}

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkShaderBase::asFragmentProcessor(const GrFPArgs&) const {
    return nullptr;
}
#endif

sk_sp<SkShader> SkShaderBase::makeAsALocalMatrixShader(SkMatrix*) const {
    return nullptr;
}

sk_sp<SkShader> SkShaders::Empty() { return sk_make_sp<SkEmptyShader>(); }
sk_sp<SkShader> SkShaders::Color(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> SkBitmap::makeShader(SkTileMode tmx, SkTileMode tmy, const SkMatrix* lm) const {
    if (lm && !lm->invert(nullptr)) {
        return nullptr;
    }
    return SkMakeBitmapShader(*this, tmx, tmy, lm, kIfMutable_SkCopyPixelsMode);
}

sk_sp<SkShader> SkBitmap::makeShader(const SkMatrix* lm) const {
    return this->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, lm);
}

bool SkShaderBase::appendStages(const SkStageRec& rec) const {
    return this->onAppendStages(rec);
}

bool SkShaderBase::onAppendStages(const SkStageRec& rec) const {
    // SkShader::Context::shadeSpan() handles the paint opacity internally,
    // but SkRasterPipelineBlitter applies it as a separate stage.
    // We skip the internal shadeSpan() step by forcing the paint opaque.
    SkTCopyOnFirstWrite<SkPaint> opaquePaint(rec.fPaint);
    if (rec.fPaint.getAlpha() != SK_AlphaOPAQUE) {
        opaquePaint.writable()->setAlpha(SK_AlphaOPAQUE);
    }

    ContextRec cr(*opaquePaint, rec.fCTM, rec.fLocalM, rec.fDstColorType, sk_srgb_singleton());

    struct CallbackCtx : SkRasterPipeline_CallbackCtx {
        sk_sp<const SkShader> shader;
        Context*              ctx;
    };
    auto cb = rec.fAlloc->make<CallbackCtx>();
    cb->shader = sk_ref_sp(this);
    cb->ctx = as_SB(this)->makeContext(cr, rec.fAlloc);
    cb->fn  = [](SkRasterPipeline_CallbackCtx* self, int active_pixels) {
        auto c = (CallbackCtx*)self;
        int x = (int)c->rgba[0],
            y = (int)c->rgba[1];
        SkPMColor tmp[SkRasterPipeline_kMaxStride];
        c->ctx->shadeSpan(x,y, tmp, active_pixels);

        for (int i = 0; i < active_pixels; i++) {
            auto rgba_4f = SkPMColor4f::FromPMColor(tmp[i]);
            memcpy(c->rgba + 4*i, rgba_4f.vec(), 4*sizeof(float));
        }
    };

    if (cb->ctx) {
        rec.fPipeline->append(SkRasterPipeline::seed_shader);
        rec.fPipeline->append(SkRasterPipeline::callback, cb);
        rec.fAlloc->make<SkColorSpaceXformSteps>(sk_srgb_singleton(), kPremul_SkAlphaType,
                                                 rec.fDstCS,          kPremul_SkAlphaType)
            ->apply(rec.fPipeline, true);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShaders::Empty();
}
