/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/SkColorFilterShader.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include <utility>

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

bool SkColorFilterShader::appendStages(const SkStageRec& rec,
                                       const SkShaders::MatrixRec& mRec) const {
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

#if defined(SK_ENABLE_SKVM)
skvm::Color SkColorFilterShader::program(skvm::Builder* p,
                                         skvm::Coord device,
                                         skvm::Coord local,
                                         skvm::Color paint,
                                         const SkShaders::MatrixRec& mRec,
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
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GRAPHITE)

void SkColorFilterShader::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                   skgpu::graphite::PaintParamsKeyBuilder* builder,
                                   skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    ColorFilterShaderBlock::BeginBlock(keyContext, builder, gatherer);

    as_SB(fShader)->addToKey(keyContext, builder, gatherer);
    as_CFB(fFilter)->addToKey(keyContext, builder, gatherer);

    builder->endBlock();
}

#endif  // SK_ENABLE_SKSL
