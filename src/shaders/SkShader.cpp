/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkAtomics.h"
#include "SkBitmapProcShader.h"
#include "SkColorShader.h"
#include "SkColorSpaceXformer.h"
#include "SkEmptyShader.h"
#include "SkMallocPixelRef.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureShader.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkScalar.h"
#include "SkShaderBase.h"
#include "SkTLazy.h"
#include "SkWriteBuffer.h"
#include "../jumper/SkJumper.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

//#define SK_TRACK_SHADER_LIFETIME

#ifdef SK_TRACK_SHADER_LIFETIME
    static int32_t gShaderCounter;
#endif

static inline void inc_shader_counter() {
#ifdef SK_TRACK_SHADER_LIFETIME
    int32_t prev = sk_atomic_inc(&gShaderCounter);
    SkDebugf("+++ shader counter %d\n", prev + 1);
#endif
}
static inline void dec_shader_counter() {
#ifdef SK_TRACK_SHADER_LIFETIME
    int32_t prev = sk_atomic_dec(&gShaderCounter);
    SkDebugf("--- shader counter %d\n", prev - 1);
#endif
}

SkShaderBase::SkShaderBase(const SkMatrix* localMatrix)
    : fLocalMatrix(localMatrix ? *localMatrix : SkMatrix::I()) {
    inc_shader_counter();
    // Pre-cache so future calls to fLocalMatrix.getType() are threadsafe.
    (void)fLocalMatrix.getType();
}

SkShaderBase::~SkShaderBase() {
    dec_shader_counter();
}

void SkShaderBase::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    bool hasLocalM = !fLocalMatrix.isIdentity();
    buffer.writeBool(hasLocalM);
    if (hasLocalM) {
        buffer.writeMatrix(fLocalMatrix);
    }
}

bool SkShaderBase::computeTotalInverse(const SkMatrix& ctm,
                                       const SkMatrix* outerLocalMatrix,
                                       SkMatrix* totalInverse) const {
    SkMatrix total = SkMatrix::Concat(ctm, fLocalMatrix);
    if (outerLocalMatrix) {
        total.preConcat(*outerLocalMatrix);
    }

    return total.invert(totalInverse);
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
    return this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, nullptr)
        ? this->onMakeContext(rec, alloc)
        : nullptr;
}

SkShaderBase::Context* SkShaderBase::makeBurstPipelineContext(const ContextRec& rec,
                                                              SkArenaAlloc* alloc) const {

    SkASSERT(rec.fPreferredDstType == ContextRec::kPM4f_DstType);

    // Always use vanilla stages for perspective.
    if (rec.fMatrix->hasPerspective() || fLocalMatrix.hasPerspective()) {
        return nullptr;
    }

    return this->computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, nullptr)
        ? this->onMakeBurstPipelineContext(rec, alloc)
        : nullptr;
}

SkShaderBase::Context::Context(const SkShaderBase& shader, const ContextRec& rec)
    : fShader(shader), fCTM(*rec.fMatrix)
{
    // We should never use a context for RP-only shaders.
    SkASSERT(!shader.isRasterPipelineOnly(*rec.fMatrix));
    // ... or for perspective.
    SkASSERT(!rec.fMatrix->hasPerspective());
    SkASSERT(!rec.fLocalMatrix || !rec.fLocalMatrix->hasPerspective());

    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(fShader.computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &fTotalInverse));

    fPaintAlpha = rec.fPaint->getAlpha();
}

SkShaderBase::Context::~Context() {}

void SkShaderBase::Context::shadeSpan4f(int x, int y, SkPM4f dst[], int count) {
    const int N = 128;
    SkPMColor tmp[N];
    while (count > 0) {
        int n = SkTMin(count, N);
        this->shadeSpan(x, y, tmp, n);
        for (int i = 0; i < n; ++i) {
            dst[i] = SkPM4f::FromPMColor(tmp[i]);
        }
        dst += n;
        x += n;
        count -= n;
    }
}

const SkMatrix& SkShader::getLocalMatrix() const {
    return as_SB(this)->getLocalMatrix();
}

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
bool SkShader::isABitmap(SkBitmap* outTexture, SkMatrix* outMatrix, TileMode xy[2]) const {
    return  as_SB(this)->onIsABitmap(outTexture, outMatrix, xy);
}
#endif

SkImage* SkShader::isAImage(SkMatrix* localMatrix, TileMode xy[2]) const {
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

sk_sp<SkShader> SkShader::makeAsALocalMatrixShader(SkMatrix*) const {
    return nullptr;
}

sk_sp<SkShader> SkShader::MakeEmptyShader() { return sk_make_sp<SkEmptyShader>(); }

sk_sp<SkShader> SkShader::MakeColorShader(SkColor color) { return sk_make_sp<SkColorShader>(color); }

sk_sp<SkShader> SkShader::MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                           const SkMatrix* localMatrix) {
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    return SkMakeBitmapShader(src, tmx, tmy, localMatrix, kIfMutable_SkCopyPixelsMode);
}

sk_sp<SkShader> SkShader::MakePictureShader(sk_sp<SkPicture> src, TileMode tmx, TileMode tmy,
                                            const SkMatrix* localMatrix, const SkRect* tile) {
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    return SkPictureShader::Make(std::move(src), tmx, tmy, localMatrix, tile);
}

#ifndef SK_IGNORE_TO_STRING
void SkShaderBase::toString(SkString* str) const {
    if (!fLocalMatrix.isIdentity()) {
        str->append(" ");
        fLocalMatrix.toString(str);
    }
}
#endif

bool SkShaderBase::appendStages(const StageRec& rec) const {
    return this->onAppendStages(rec);
}

bool SkShaderBase::onAppendStages(const StageRec& rec) const {
    // SkShader::Context::shadeSpan4f() handles the paint opacity internally,
    // but SkRasterPipelineBlitter applies it as a separate stage.
    // We skip the internal shadeSpan4f() step by forcing the paint opaque.
    SkTCopyOnFirstWrite<SkPaint> opaquePaint(rec.fPaint);
    if (rec.fPaint.getAlpha() != SK_AlphaOPAQUE) {
        opaquePaint.writable()->setAlpha(SK_AlphaOPAQUE);
    }

    ContextRec cr(*opaquePaint, rec.fCTM, rec.fLocalM, ContextRec::kPM4f_DstType, rec.fDstCS);

    struct CallbackCtx : SkJumper_CallbackCtx {
        sk_sp<SkShader> shader;
        Context*        ctx;
    };
    auto cb = rec.fAlloc->make<CallbackCtx>();
    cb->shader = rec.fDstCS ? SkColorSpaceXformer::Make(sk_ref_sp(rec.fDstCS))->apply(this)
                            : sk_ref_sp((SkShader*)this);
    cb->ctx = as_SB(cb->shader)->makeContext(cr, rec.fAlloc);
    cb->fn  = [](SkJumper_CallbackCtx* self, int active_pixels) {
        auto c = (CallbackCtx*)self;
        int x = (int)c->rgba[0],
        y = (int)c->rgba[1];
        c->ctx->shadeSpan4f(x,y, (SkPM4f*)c->rgba, active_pixels);
    };

    if (cb->ctx) {
        rec.fPipeline->append_seed_shader();
        rec.fPipeline->append(SkRasterPipeline::callback, cb);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkEmptyShader::CreateProc(SkReadBuffer&) {
    return SkShader::MakeEmptyShader();
}

#ifndef SK_IGNORE_TO_STRING
#include "SkEmptyShader.h"

void SkEmptyShader::toString(SkString* str) const {
    str->append("SkEmptyShader: (");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
