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
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include <cstdint>
#include <string>

using namespace skia_private;

#if defined(SK_BUILD_FOR_DEBUGGER)
    constexpr bool kLenientSkSLDeserialization = true;
#else
    constexpr bool kLenientSkSLDeserialization = false;
#endif

void SkRuntimeBlender::flatten(SkWriteBuffer& buffer) const {
    if (SkKnownRuntimeEffects::IsSkiaKnownRuntimeEffect(fEffect->fStableKey)) {
        // We only serialize Skia-internal stableKeys. First party stable keys are not serialized.
        buffer.write32(fEffect->fStableKey);
    } else {
        buffer.write32(0);
        buffer.writeString(fEffect->source().c_str());
    }
    buffer.writeDataAsByteArray(fUniforms.get());
    SkRuntimeEffectPriv::WriteChildEffects(buffer, fChildren);
}

sk_sp<SkFlattenable> SkRuntimeBlender::CreateProc(SkReadBuffer& buffer) {
    if (!buffer.validate(buffer.allowSkSL())) {
        return nullptr;
    }

    sk_sp<SkRuntimeEffect> effect;
    if (!buffer.isVersionLT(SkPicturePriv::kSerializeStableKeys)) {
        uint32_t candidateStableKey = buffer.readUInt();
        effect = SkKnownRuntimeEffects::MaybeGetKnownRuntimeEffect(candidateStableKey);
        if (!effect && !buffer.validate(candidateStableKey == 0)) {
            return nullptr;
        }
    }

    if (!effect) {
        SkString sksl;
        buffer.readString(&sksl);
        effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForBlender, std::move(sksl));
    }
    if constexpr (!kLenientSkSLDeserialization) {
        if (!buffer.validate(effect != nullptr)) {
            return nullptr;
        }
    }

    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!SkRuntimeEffectPriv::ReadChildEffects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    if constexpr (kLenientSkSLDeserialization) {
        if (!effect) {
            SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL blender.\n");
            return nullptr;
        }
    }

    return effect->makeBlender(std::move(uniforms), SkSpan(children));
}

bool SkRuntimeBlender::onAppendStages(const SkStageRec& rec) const {
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
    return false;
}
