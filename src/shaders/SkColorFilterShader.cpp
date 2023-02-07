/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkColorFilterShader.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

SkColorFilterShader::SkColorFilterShader(sk_sp<SkShader> shader,
                                         float alpha,
                                         sk_sp<SkColorFilter> filter)
    : fShader(std::move(shader))
    , fFilter(as_CFB_sp(std::move(filter)))
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
    return fShader->isOpaque() && fAlpha == 1.0f && as_CFB(fFilter)->isAlphaUnchanged();
}

void SkColorFilterShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShader.get());
    SkASSERT(fAlpha == 1.0f);  // Not exposed in public API SkShader::makeWithColorFilter().
    buffer.writeFlattenable(fFilter.get());
}

bool SkColorFilterShader::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    if (!as_SB(fShader)->appendStages(rec, mRec)) {
        return false;
    }
    if (fAlpha != 1.0f) {
        rec.fPipeline->append(SkRasterPipelineOp::scale_1_float, rec.fAlloc->make<float>(fAlpha));
    }
    if (!fFilter->appendStages(rec, fShader->isOpaque())) {
        return false;
    }
    return true;
}

skvm::Color SkColorFilterShader::program(skvm::Builder* p,
                                         skvm::Coord device,
                                         skvm::Coord local,
                                         skvm::Color paint,
                                         const MatrixRec& mRec,
                                         const SkColorInfo& dst,
                                         skvm::Uniforms* uniforms,
                                         SkArenaAlloc* alloc) const {
    // Run the shader.
    skvm::Color c = as_SB(fShader)->program(p, device, local, paint, mRec, dst, uniforms, alloc);
    if (!c) {
        return {};
    }
    // Scale that by alpha.
    if (fAlpha != 1.0f) {
        skvm::F32 A = p->uniformF(uniforms->pushF(fAlpha));
        c.r *= A;
        c.g *= A;
        c.b *= A;
        c.a *= A;
    }

    // Finally run that through the color filter.
    return fFilter->program(p,c, dst, uniforms,alloc);
}

#if SK_SUPPORT_GPU
/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor>
SkColorFilterShader::asFragmentProcessor(const GrFPArgs& args, const MatrixRec& mRec) const {
    auto shaderFP = as_SB(fShader)->asFragmentProcessor(args, mRec);
    if (!shaderFP) {
        return nullptr;
    }

    // TODO I guess, but it shouldn't come up as used today.
    SkASSERT(fAlpha == 1.0f);

    auto [success, fp] = fFilter->asFragmentProcessor(std::move(shaderFP), args.fContext,
                                                      *args.fDstColorInfo, args.fSurfaceProps);
    // If the filter FP could not be created, we still want to return the shader FP, so checking
    // success can be omitted here.
    return std::move(fp);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_GRAPHITE_ENABLED

void SkColorFilterShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    ColorFilterShaderBlock::BeginBlock(keyContext, builder, gatherer);

    as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    as_CFB(fFilter)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();
}

#endif // SK_ENABLE_SKSL

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShader::makeWithColorFilter(sk_sp<SkColorFilter> filter) const {
    SkShader* base = const_cast<SkShader*>(this);
    if (!filter) {
        return sk_ref_sp(base);
    }
    return sk_make_sp<SkColorFilterShader>(sk_ref_sp(base), 1.0f, std::move(filter));
}
