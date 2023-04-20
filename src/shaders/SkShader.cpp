/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

SkShaderBase::SkShaderBase() = default;

SkShaderBase::~SkShaderBase() = default;

SkShaderBase::MatrixRec::MatrixRec(const SkMatrix& ctm) : fCTM(ctm) {}

std::optional<SkShaderBase::MatrixRec>
SkShaderBase::MatrixRec::apply(const SkStageRec& rec, const SkMatrix& postInv) const {
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
    // append_matrix is a no-op if total worked out to identity.
    rec.fPipeline->append_matrix(rec.fAlloc, total);
    return MatrixRec{fCTM,
                     fTotalLocalMatrix,
                     /*pendingLocalMatrix=*/SkMatrix::I(),
                     fTotalMatrixIsValid,
                     /*ctmApplied=*/true};
}

std::optional<SkShaderBase::MatrixRec>
SkShaderBase::MatrixRec::apply(skvm::Builder* p,
                               skvm::Coord* local,
                               skvm::Uniforms* uniforms,
                               const SkMatrix& postInv) const {
    SkMatrix total = fPendingLocalMatrix;
    if (!fCTMApplied) {
        total = SkMatrix::Concat(fCTM, total);
    }
    if (!total.invert(&total)) {
        return {};
    }
    total = SkMatrix::Concat(postInv, total);
    // ApplyMatrix is a no-op if total worked out to identity.
    *local = SkShaderBase::ApplyMatrix(p, total, *local, uniforms);
    return MatrixRec{fCTM,
                     fTotalLocalMatrix,
                     /*pendingLocalMatrix=*/SkMatrix::I(),
                     fTotalMatrixIsValid,
                     /*ctmApplied=*/true};
}

#if defined(SK_GANESH)
GrFPResult SkShaderBase::MatrixRec::apply(std::unique_ptr<GrFragmentProcessor> fp,
                                          const SkMatrix& postInv) const {
    // FP matrices work differently than SkRasterPipeline and SkVM. The starting coordinates
    // provided to the root SkShader's FP are already in local space. So we never apply the inverse
    // CTM.
    SkASSERT(!fCTMApplied);
    SkMatrix total;
    if (!fPendingLocalMatrix.invert(&total)) {
        return {false, std::move(fp)};
    }
    total = SkMatrix::Concat(postInv, total);
    // GrMatrixEffect returns 'fp' if total worked out to identity.
    return {true, GrMatrixEffect::Make(total, std::move(fp))};
}

SkShaderBase::MatrixRec SkShaderBase::MatrixRec::applied() const {
    // We mark the CTM as "not applied" because we *never* apply the CTM for FPs. Their starting
    // coords are local, not device, coords.
    return MatrixRec{fCTM,
                     fTotalLocalMatrix,
                     /*pendingLocalMatrix=*/SkMatrix::I(),
                     fTotalMatrixIsValid,
                     /*ctmApplied=*/false};
}
#endif

SkShaderBase::MatrixRec SkShaderBase::MatrixRec::concat(const SkMatrix& m) const {
    return {fCTM,
            SkShaderBase::ConcatLocalMatrices(fTotalLocalMatrix, m),
            SkShaderBase::ConcatLocalMatrices(fPendingLocalMatrix, m),
            fTotalMatrixIsValid,
            fCTMApplied};
}

void SkShaderBase::flatten(SkWriteBuffer& buffer) const { this->INHERITED::flatten(buffer); }

bool SkShaderBase::computeTotalInverse(const SkMatrix& ctm,
                                       const SkMatrix* localMatrix,
                                       SkMatrix* totalInverse) const {
    return (localMatrix ? SkMatrix::Concat(ctm, *localMatrix) : ctm).invert(totalInverse);
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
    if (rec.fMatrix->hasPerspective() || (rec.fLocalMatrix && rec.fLocalMatrix->hasPerspective()) ||
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

    // Because the context parameters must be valid at this point, we know that the matrix is
    // invertible.
    SkAssertResult(fShader.computeTotalInverse(*rec.fMatrix, rec.fLocalMatrix, &fTotalInverse));

    fPaintAlpha = rec.fPaintAlpha;
}

SkShaderBase::Context::~Context() {}

bool SkShaderBase::ContextRec::isLegacyCompatible(SkColorSpace* shaderColorSpace) const {
    // In legacy pipelines, shaders always produce premul (or opaque) and the destination is also
    // always premul (or opaque).  (And those "or opaque" caveats won't make any difference here.)
    SkAlphaType shaderAT = kPremul_SkAlphaType,
                   dstAT = kPremul_SkAlphaType;
    return 0 == SkColorSpaceXformSteps{shaderColorSpace, shaderAT,
                                         fDstColorSpace,    dstAT}.flags.mask();
}

SkImage* SkShader::isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const {
    return as_SB(this)->onIsAImage(localMatrix, xy);
}

#if defined(SK_GANESH)
std::unique_ptr<GrFragmentProcessor>
SkShaderBase::asRootFragmentProcessor(const GrFPArgs& args, const SkMatrix& ctm) const {
    return this->asFragmentProcessor(args, MatrixRec(ctm));
}

std::unique_ptr<GrFragmentProcessor> SkShaderBase::asFragmentProcessor(const GrFPArgs&,
                                                                       const MatrixRec&) const {
    return nullptr;
}
#endif

sk_sp<SkShader> SkShaderBase::makeAsALocalMatrixShader(SkMatrix*) const {
    return nullptr;
}

#if defined(SK_GRAPHITE)
// TODO: add implementations for derived classes
void SkShaderBase::addToKey(const skgpu::graphite::KeyContext& keyContext,
                            skgpu::graphite::PaintParamsKeyBuilder* builder,
                            skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    SolidColorShaderBlock::BeginBlock(keyContext, builder, gatherer, {1, 0, 0, 1});
    builder->endBlock();
}
#endif

bool SkShaderBase::appendRootStages(const SkStageRec& rec, const SkMatrix& ctm) const {
    return this->appendStages(rec, MatrixRec(ctm));
}

bool SkShaderBase::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    // SkShader::Context::shadeSpan() handles the paint opacity internally,
    // but SkRasterPipelineBlitter applies it as a separate stage.
    // We skip the internal shadeSpan() step by forcing the paint opaque.
    SkColor4f opaquePaintColor = rec.fPaintColor.makeOpaque();

    // We don't have a separate ctm and local matrix at this point. Just pass the combined matrix
    // as the CTM. TODO: thread the MatrixRec through the legacy context system.
    auto tm = mRec.totalMatrix();
    ContextRec cr(opaquePaintColor,
                  tm,
                  nullptr,
                  rec.fDstColorType,
                  sk_srgb_singleton(),
                  rec.fSurfaceProps);

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
        SkPMColor tmp[SkRasterPipeline_kMaxStride_highp];
        c->ctx->shadeSpan(x,y, tmp, active_pixels);

        for (int i = 0; i < active_pixels; i++) {
            auto rgba_4f = SkPMColor4f::FromPMColor(tmp[i]);
            memcpy(c->rgba + 4*i, rgba_4f.vec(), 4*sizeof(float));
        }
    };

    if (cb->ctx) {
        rec.fPipeline->append(SkRasterPipelineOp::seed_shader);
        rec.fPipeline->append(SkRasterPipelineOp::callback, cb);
        rec.fAlloc->make<SkColorSpaceXformSteps>(sk_srgb_singleton(), kPremul_SkAlphaType,
                                                 rec.fDstCS,          kPremul_SkAlphaType)
            ->apply(rec.fPipeline);
        return true;
    }
    return false;
}

skvm::Color SkShaderBase::rootProgram(skvm::Builder* p,
                                      skvm::Coord device,
                                      skvm::Color paint,
                                      const SkMatrix& ctm,
                                      const SkColorInfo& dst,
                                      skvm::Uniforms* uniforms,
                                      SkArenaAlloc* alloc) const {
    // Shader subclasses should always act as if the destination were premul or opaque.
    // SkVMBlitter handles all the coordination of unpremul itself, via premul.
    SkColorInfo tweaked = dst.alphaType() == kUnpremul_SkAlphaType
                           ? dst.makeAlphaType(kPremul_SkAlphaType)
                           : dst;

    // Force opaque alpha for all opaque shaders.
    //
    // This is primarily nice in that we usually have a 1.0f constant splat
    // somewhere in the program anyway, and this will let us drop the work the
    // shader notionally does to produce alpha, p->extract(...), etc. in favor
    // of that simple hoistable splat.
    //
    // More subtly, it makes isOpaque() a parameter to all shader program
    // generation, guaranteeing that is-opaque bit is mixed into the overall
    // shader program hash and blitter Key.  This makes it safe for us to use
    // that bit to make decisions when constructing an SkVMBlitter, like doing
    // SrcOver -> Src strength reduction.
    if (auto color = this->program(p,
                                   device,
                                   /*local=*/device,
                                   paint,
                                   MatrixRec(ctm),
                                   tweaked,
                                   uniforms,
                                   alloc)) {
        if (this->isOpaque()) {
            color.a = p->splat(1.0f);
        }
        return color;
    }
    return {};
}

// need a cheap way to invert the alpha channel of a shader (i.e. 1 - a)
sk_sp<SkShader> SkShaderBase::makeInvertAlpha() const {
    return this->makeWithColorFilter(SkColorFilters::Blend(0xFFFFFFFF, SkBlendMode::kSrcOut));
}


skvm::Coord SkShaderBase::ApplyMatrix(skvm::Builder* p, const SkMatrix& m,
                                      skvm::Coord coord, skvm::Uniforms* uniforms) {
    skvm::F32 x = coord.x,
              y = coord.y;
    if (m.isIdentity()) {
        // That was easy.
    } else if (m.isTranslate()) {
        x = p->add(x, p->uniformF(uniforms->pushF(m[2])));
        y = p->add(y, p->uniformF(uniforms->pushF(m[5])));
    } else if (m.isScaleTranslate()) {
        x = p->mad(x, p->uniformF(uniforms->pushF(m[0])), p->uniformF(uniforms->pushF(m[2])));
        y = p->mad(y, p->uniformF(uniforms->pushF(m[4])), p->uniformF(uniforms->pushF(m[5])));
    } else {  // Affine or perspective.
        auto dot = [&,x,y](int row) {
            return p->mad(x, p->uniformF(uniforms->pushF(m[3*row+0])),
                   p->mad(y, p->uniformF(uniforms->pushF(m[3*row+1])),
                             p->uniformF(uniforms->pushF(m[3*row+2]))));
        };
        x = dot(0);
        y = dot(1);
        if (m.hasPerspective()) {
            x = x * (1.0f / dot(2));
            y = y * (1.0f / dot(2));
        }
    }
    return {x,y};
}
