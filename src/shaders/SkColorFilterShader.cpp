/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorFilterShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#endif

SkColorFilterShader::SkColorFilterShader(sk_sp<SkShader> shader,
                                         float alpha,
                                         sk_sp<SkColorFilter> filter)
    : fShader(std::move(shader))
    , fFilter(std::move(filter))
    , fAlpha (alpha)
{
    SkASSERT(fShader);
    SkASSERT(fFilter);
}

sk_sp<SkFlattenable> SkColorFilterShader::CreateProc(SkReadBuffer& buffer) {
    auto shader = buffer.readShader();
    auto filter = buffer.readColorFilter();
    if (!shader || !filter) {
        return nullptr;
    }
    return sk_make_sp<SkColorFilterShader>(shader, 1.0f, filter);
}

bool SkColorFilterShader::isOpaque() const {
    return fShader->isOpaque()
        && fAlpha == 1.0f
        && (fFilter->getFlags() & SkColorFilter::kAlphaUnchanged_Flag) != 0;
}

void SkColorFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    SkASSERT(fAlpha == 1.0f);  // Not exposed in public API SkShader::makeWithColorFilter().
    buffer.writeFlattenable(fFilter.get());
}

bool SkColorFilterShader::onAppendStages(const SkStageRec& rec) const {
    if (!as_SB(fShader)->appendStages(rec)) {
        return false;
    }
    if (fAlpha != 1.0f) {
        rec.fPipeline->append(SkRasterPipeline::scale_1_float, rec.fAlloc->make<float>(fAlpha));
    }
    fFilter->appendStages(rec, fShader->isOpaque());
    return true;
}

bool SkColorFilterShader::onProgram(skvm::Builder* p,
                                    const SkMatrix& ctm, const SkMatrix* localM,
                                    SkFilterQuality quality, SkColorSpace* dstCS,
                                    skvm::Uniforms* uniforms, SkArenaAlloc* alloc,
                                    skvm::F32 x, skvm::F32 y,
                                    skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const {
    // Run the shader.
    if (!as_SB(fShader)->program(p, ctm,localM, quality,dstCS, uniforms,alloc, x,y, r,g,b,a)) {
        return false;
    }

    // Scale that by alpha.
    if (fAlpha != 1.0f) {
        skvm::F32 A = p->uniformF(uniforms->pushF(fAlpha));
        *r = p->mul(*r, A);
        *g = p->mul(*g, A);
        *b = p->mul(*b, A);
        *a = p->mul(*a, A);
    }

    // Finally run that through the color filter.
    if (!fFilter->program(p, dstCS, uniforms,alloc, r,g,b,a)) {
        return false;
    }

    return true;
}

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

#include "include/gpu/GrContext.h"

std::unique_ptr<GrFragmentProcessor> SkColorFilterShader::asFragmentProcessor(
        const GrFPArgs& args) const {
    auto fp1 = as_SB(fShader)->asFragmentProcessor(args);
    if (!fp1) {
        return nullptr;
    }

    // TODO I guess, but it shouldn't come up as used today.
    SkASSERT(fAlpha == 1.0f);

    auto fp2 = fFilter->asFragmentProcessor(args.fContext, *args.fDstColorInfo);
    if (!fp2) {
        return fp1;
    }

    std::unique_ptr<GrFragmentProcessor> fpSeries[] = { std::move(fp1), std::move(fp2) };
    return GrFragmentProcessor::RunInSeries(fpSeries, 2);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShader::makeWithColorFilter(sk_sp<SkColorFilter> filter) const {
    SkShader* base = const_cast<SkShader*>(this);
    if (!filter) {
        return sk_ref_sp(base);
    }
    return sk_make_sp<SkColorFilterShader>(sk_ref_sp(base), 1.0f, filter);
}
