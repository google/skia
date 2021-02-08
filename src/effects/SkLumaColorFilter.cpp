/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkVM.h"

#if SK_SUPPORT_GPU
#include "src/gpu/effects/generated/GrLumaColorFilterEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

class SkLumaColorFilterImpl : public SkColorFilterBase {
public:
#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&) const override {
        return GrFPSuccess(GrLumaColorFilterEffect::Make(std::move(inputFP)));
    }
#endif

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&) {
        return SkLumaColorFilter::Make();
    }

protected:
    void flatten(SkWriteBuffer&) const override {}

private:
    Factory getFactory() const override { return CreateProc; }
    const char* getTypeName() const override { return "SkLumaColorFilter"; }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        rec.fPipeline->append(SkRasterPipeline::bt709_luminance_or_luma_to_alpha);
        rec.fPipeline->append(SkRasterPipeline::clamp_0);
        rec.fPipeline->append(SkRasterPipeline::clamp_1);
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, SkColorSpace*, skvm::Uniforms*,
                          SkArenaAlloc*) const override {
        return {
            p->splat(0.0f),
            p->splat(0.0f),
            p->splat(0.0f),
            clamp01(c.r * 0.2126f + c.g * 0.7152f + c.b * 0.0722f),
        };
    }

    using INHERITED = SkColorFilterBase;
};

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
#if defined(SK_SUPPORT_LEGACY_RUNTIME_EFFECTS)
    return sk_sp<SkColorFilter>(new SkLumaColorFilterImpl);
#else
    static SkRuntimeEffect* effect = []{
        const char* code =
            "uniform shader input;"
            "half4 main() {"
                "return saturate(dot(half3(0.2126, 0.7152, 0.0722), sample(input).rgb)).000r;"
            "}";
        auto [effect, err] = SkRuntimeEffect::Make(SkString{code}, SkRuntimeEffect::Options{});
        SkASSERT(effect && err.isEmpty());
        return effect.release();
    }();

    sk_sp<SkColorFilter> input = nullptr;
    return effect->makeColorFilter(SkData::MakeEmpty(), &input, 1);
#endif
}

void SkLumaColorFilter::RegisterFlattenable() {
    SkFlattenable::Register("SkLumaColorFilter", SkLumaColorFilterImpl::CreateProc);
}
