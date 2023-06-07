/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkRuntimeBlender.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include <string>

using namespace skia_private;

#if defined(SK_BUILD_FOR_DEBUGGER)
    #define SK_LENIENT_SKSL_DESERIALIZATION 1
#else
    #define SK_LENIENT_SKSL_DESERIALIZATION 0
#endif

sk_sp<SkFlattenable> SkRuntimeBlender::CreateProc(SkReadBuffer& buffer) {
    if (!buffer.validate(buffer.allowSkSL())) {
        return nullptr;
    }

    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForBlender, std::move(sksl));
#if !SK_LENIENT_SKSL_DESERIALIZATION
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
#endif

    STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!SkRuntimeEffectPriv::ReadChildEffects(buffer, effect.get(), &children)) {
        return nullptr;
    }

#if SK_LENIENT_SKSL_DESERIALIZATION
    if (!effect) {
        SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL blender.\n");
        return nullptr;
    }
#endif

    return effect->makeBlender(std::move(uniforms), SkSpan(children));
}

bool SkRuntimeBlender::onAppendStages(const SkStageRec& rec) const {
#ifdef SK_ENABLE_SKSL_IN_RASTER_PIPELINE
    if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
        // SkRP has support for many parts of #version 300 already, but for now, we restrict its
        // usage in runtime effects to just #version 100.
        return false;
    }
    if (const SkSL::RP::Program* program = fEffect->getRPProgram(/*debugTrace=*/nullptr)) {
        SkSpan<const float> uniforms = SkRuntimeEffectPriv::UniformsAsSpan(
                fEffect->uniforms(),
                fUniforms,
                /*alwaysCopyIntoAlloc=*/false,
                rec.fDstCS,
                rec.fAlloc);
        SkShaders::MatrixRec matrix(SkMatrix::I());
        matrix.markCTMApplied();
        RuntimeEffectRPCallbacks callbacks(rec, matrix, fChildren, fEffect->fSampleUsages);
        bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
        return success;
    }
#endif
    return false;
}

void SkRuntimeBlender::flatten(SkWriteBuffer& buffer) const {
    buffer.writeString(fEffect->source().c_str());
    buffer.writeDataAsByteArray(fUniforms.get());
    SkRuntimeEffectPriv::WriteChildEffects(buffer, fChildren);
}

#ifdef SK_ENABLE_SKVM
skvm::Color SkRuntimeBlender::onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                                        const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                                        SkArenaAlloc* alloc) const {
    if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
        return {};
    }

    sk_sp<const SkData> inputs = SkRuntimeEffectPriv::TransformUniforms(fEffect->uniforms(),
                                                                        fUniforms,
                                                                        colorInfo.colorSpace());
    SkASSERT(inputs);

    SkShaders::MatrixRec mRec(SkMatrix::I());
    mRec.markTotalMatrixInvalid();
    RuntimeEffectVMCallbacks callbacks(p, uniforms, alloc, fChildren, mRec, src, colorInfo);
    std::vector<skvm::Val> uniform = SkRuntimeEffectPriv::MakeSkVMUniforms(p,
                                                                           uniforms,
                                                                           fEffect->uniformSize(),
                                                                           *inputs);

    // Emit the blend function as an SkVM program.
    skvm::Coord zeroCoord = {p->splat(0.0f), p->splat(0.0f)};
    return SkSL::ProgramToSkVM(*fEffect->fBaseProgram, fEffect->fMain, p,/*debugTrace=*/nullptr,
                               SkSpan(uniform), /*device=*/zeroCoord, /*local=*/zeroCoord,
                               src, dst, &callbacks);
}
#endif

#if defined(SK_GRAPHITE)
void SkRuntimeBlender::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                skgpu::graphite::PaintParamsKeyBuilder* builder,
                                skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            fEffect->uniforms(),
            fUniforms,
            keyContext.dstColorInfo().colorSpace());
    SkASSERT(uniforms);

    RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer,
                                   { fEffect, std::move(uniforms) });

    SkRuntimeEffectPriv::AddChildrenToKey(fChildren, fEffect->children(), keyContext, builder,
                                          gatherer);

    builder->endBlock();
}
#endif
