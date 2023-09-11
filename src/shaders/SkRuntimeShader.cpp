/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/shaders/SkRuntimeShader.h"

#include "include/core/SkCapabilities.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#if defined(SK_BUILD_FOR_DEBUGGER)
    constexpr bool kLenientSkSLDeserialization = true;
#else
    constexpr bool kLenientSkSLDeserialization = false;
#endif

class SkColorSpace;
struct SkIPoint;

SkRuntimeShader::SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                                 sk_sp<SkSL::DebugTracePriv> debugTrace,
                                 sk_sp<const SkData> uniforms,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children)
        : fEffect(std::move(effect))
        , fDebugTrace(std::move(debugTrace))
        , fUniformData(std::move(uniforms))
        , fChildren(children.begin(), children.end()) {}

SkRuntimeShader::SkRuntimeShader(sk_sp<SkRuntimeEffect> effect,
                                 sk_sp<SkSL::DebugTracePriv> debugTrace,
                                 UniformsCallback uniformsCallback,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children)
        : fEffect(std::move(effect))
        , fDebugTrace(std::move(debugTrace))
        , fUniformsCallback(std::move(uniformsCallback))
        , fChildren(children.begin(), children.end()) {}

static sk_sp<SkSL::DebugTracePriv> make_debug_trace(SkRuntimeEffect* effect,
                                                    const SkIPoint& coord) {
    auto debugTrace = sk_make_sp<SkSL::DebugTracePriv>();
    debugTrace->setSource(effect->source());
    debugTrace->setTraceCoord(coord);
    return debugTrace;
}

SkRuntimeEffect::TracedShader SkRuntimeShader::makeTracedClone(const SkIPoint& coord) {
    sk_sp<SkRuntimeEffect> unoptimized = fEffect->makeUnoptimizedClone();
    sk_sp<SkSL::DebugTracePriv> debugTrace = make_debug_trace(unoptimized.get(), coord);
    auto debugShader = sk_make_sp<SkRuntimeShader>(
            unoptimized, debugTrace, this->uniformData(nullptr), SkSpan(fChildren));

    return SkRuntimeEffect::TracedShader{std::move(debugShader), std::move(debugTrace)};
}

bool SkRuntimeShader::appendStages(const SkStageRec& rec, const SkShaders::MatrixRec& mRec) const {
    if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
        // SkRP has support for many parts of #version 300 already, but for now, we restrict its
        // usage in runtime effects to just #version 100.
        return false;
    }
    if (const SkSL::RP::Program* program = fEffect->getRPProgram(fDebugTrace.get())) {
        std::optional<SkShaders::MatrixRec> newMRec = mRec.apply(rec);
        if (!newMRec.has_value()) {
            return false;
        }
        SkSpan<const float> uniforms =
                SkRuntimeEffectPriv::UniformsAsSpan(fEffect->uniforms(),
                                                    this->uniformData(rec.fDstCS),
                                                    /*alwaysCopyIntoAlloc=*/fUniformData == nullptr,
                                                    rec.fDstCS,
                                                    rec.fAlloc);
        RuntimeEffectRPCallbacks callbacks(rec, *newMRec, fChildren, fEffect->fSampleUsages);
        bool success = program->appendStages(rec.fPipeline, rec.fAlloc, &callbacks, uniforms);
        return success;
    }
    return false;
}

void SkRuntimeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeString(fEffect->source().c_str());
    buffer.writeDataAsByteArray(this->uniformData(nullptr).get());
    SkRuntimeEffectPriv::WriteChildEffects(buffer, fChildren);
}

sk_sp<const SkData> SkRuntimeShader::uniformData(const SkColorSpace* dstCS) const {
    if (fUniformData) {
        return fUniformData;
    }

    // We want to invoke the uniforms-callback each time a paint occurs.
    SkASSERT(fUniformsCallback);
    sk_sp<const SkData> uniforms = fUniformsCallback({dstCS});
    SkASSERT(uniforms && uniforms->size() == fEffect->uniformSize());
    return uniforms;
}

sk_sp<SkFlattenable> SkRuntimeShader::CreateProc(SkReadBuffer& buffer) {
    if (!buffer.validate(buffer.allowSkSL())) {
        return nullptr;
    }

    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    SkTLazy<SkMatrix> localM;
    if (buffer.isVersionLT(SkPicturePriv::kNoShaderLocalMatrix)) {
        uint32_t flags = buffer.read32();
        if (flags & kHasLegacyLocalMatrix_Flag) {
            buffer.readMatrix(localM.init());
        }
    }

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    if constexpr (!kLenientSkSLDeserialization) {
        if (!buffer.validate(effect != nullptr)) {
            return nullptr;
        }
    }

    skia_private::STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!SkRuntimeEffectPriv::ReadChildEffects(buffer, effect.get(), &children)) {
        return nullptr;
    }

    if constexpr (kLenientSkSLDeserialization) {
        if (!effect) {
            // If any children were SkShaders, return the first one. This is a reasonable fallback.
            for (int i = 0; i < children.size(); i++) {
                if (children[i].shader()) {
                    SkDebugf("Serialized SkSL failed to compile. Replacing shader with child %d.\n",
                             i);
                    return sk_ref_sp(children[i].shader());
                }
            }

            // We don't know what to do, so just return nullptr (but *don't* poison the buffer).
            SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL shader.\n");
            return nullptr;
        }
    }

    return effect->makeShader(std::move(uniforms), SkSpan(children), localM.getMaybeNull());
}
