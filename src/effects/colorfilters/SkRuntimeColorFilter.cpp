/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkRuntimeColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/sksl/codegen/SkSLRasterPipelineBuilder.h"

#include <string>
#include <utility>

SkRuntimeColorFilter::SkRuntimeColorFilter(sk_sp<SkRuntimeEffect> effect,
                                           sk_sp<const SkData> uniforms,
                                           SkSpan<const SkRuntimeEffect::ChildPtr> children)
        : fEffect(std::move(effect))
        , fUniforms(std::move(uniforms))
        , fChildren(children.begin(), children.end()) {}

bool SkRuntimeColorFilter::appendStages(const SkStageRec& rec, bool) const {
    if (!SkRuntimeEffectPriv::CanDraw(SkCapabilities::RasterBackend().get(), fEffect.get())) {
        // SkRP has support for many parts of #version 300 already, but for now, we restrict its
        // usage in runtime effects to just #version 100.
        return false;
    }
    if (const SkSL::RP::Program* program = fEffect->getRPProgram(/*debugTrace=*/nullptr)) {
        SkSpan<const float> uniforms =
                SkRuntimeEffectPriv::UniformsAsSpan(fEffect->uniforms(),
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

bool SkRuntimeColorFilter::onIsAlphaUnchanged() const {
    return fEffect->isAlphaUnchanged();
}

void SkRuntimeColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeString(fEffect->source().c_str());
    buffer.writeDataAsByteArray(fUniforms.get());
    SkRuntimeEffectPriv::WriteChildEffects(buffer, fChildren);
}

SkRuntimeEffect* SkRuntimeColorFilter::asRuntimeEffect() const { return fEffect.get(); }

sk_sp<SkFlattenable> SkRuntimeColorFilter::CreateProc(SkReadBuffer& buffer) {
    if (!buffer.validate(buffer.allowSkSL())) {
        return nullptr;
    }

    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, std::move(sksl));
#if !SK_LENIENT_SKSL_DESERIALIZATION
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
#endif

    skia_private::STArray<4, SkRuntimeEffect::ChildPtr> children;
    if (!SkRuntimeEffectPriv::ReadChildEffects(buffer, effect.get(), &children)) {
        return nullptr;
    }

#if SK_LENIENT_SKSL_DESERIALIZATION
    if (!effect) {
        SkDebugf("Serialized SkSL failed to compile. Ignoring/dropping SkSL color filter.\n");
        return nullptr;
    }
#endif

    return effect->makeColorFilter(std::move(uniforms), SkSpan(children));
}

/////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Lerp(float weight, sk_sp<SkColorFilter> cf0,
                                                        sk_sp<SkColorFilter> cf1) {
    if (!cf0 && !cf1) {
        return nullptr;
    }
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }

    if (cf0 == cf1) {
        return cf0; // or cf1
    }

    if (weight <= 0) {
        return cf0;
    }
    if (weight >= 1) {
        return cf1;
    }

    static const SkRuntimeEffect* effect =
            SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                      "uniform colorFilter cf0;"
                                      "uniform colorFilter cf1;"
                                      "uniform half weight;"
                                      "half4 main(half4 color) {"
                                      "return mix(cf0.eval(color), cf1.eval(color), weight);"
                                      "}")
                    .release();
    SkASSERT(effect);

    sk_sp<SkColorFilter> inputs[] = {cf0,cf1};
    return effect->makeColorFilter(SkData::MakeWithCopy(&weight, sizeof(weight)),
                                   inputs, std::size(inputs));
}

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    static const SkRuntimeEffect* effect = SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 inColor) {"
            "return saturate(dot(half3(0.2126, 0.7152, 0.0722), inColor.rgb)).000r;"
        "}"
    ).release();
    SkASSERT(effect);

    return effect->makeColorFilter(SkData::MakeEmpty());
}

sk_sp<SkColorFilter> SkOverdrawColorFilter::MakeWithSkColors(const SkColor colors[kNumColors]) {
    static const SkRuntimeEffect* effect = SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "uniform half4 color0, color1, color2, color3, color4, color5;"

        "half4 main(half4 color) {"
            "half alpha = 255.0 * color.a;"
            "return alpha < 0.5 ? color0"
                 ": alpha < 1.5 ? color1"
                 ": alpha < 2.5 ? color2"
                 ": alpha < 3.5 ? color3"
                 ": alpha < 4.5 ? color4 : color5;"
        "}"
    ).release();

    if (effect) {
        auto data = SkData::MakeUninitialized(kNumColors * sizeof(SkPMColor4f));
        SkPMColor4f* premul = (SkPMColor4f*)data->writable_data();
        for (int i = 0; i < kNumColors; ++i) {
            premul[i] = SkColor4f::FromColor(colors[i]).premul();
        }
        return effect->makeColorFilter(std::move(data));
    }
    return nullptr;
}

