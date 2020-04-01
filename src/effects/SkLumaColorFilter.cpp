/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/private/SkColorData.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkVM.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/effects/generated/GrLumaColorFilterEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

bool SkLumaColorFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipeline::bt709_luminance_or_luma_to_alpha);
    rec.fPipeline->append(SkRasterPipeline::clamp_0);
    rec.fPipeline->append(SkRasterPipeline::clamp_1);
    return true;
}

skvm::Color SkLumaColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                                         SkColorSpace* dstCS,
                                         skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    return {
        p->splat(0.0f),
        p->splat(0.0f),
        p->splat(0.0f),
        clamp01(c.r * 0.2126f + c.g * 0.7152f + c.b * 0.0722f),
    };
}

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    return sk_sp<SkColorFilter>(new SkLumaColorFilter);
}

SkLumaColorFilter::SkLumaColorFilter() : INHERITED() {}

sk_sp<SkFlattenable> SkLumaColorFilter::CreateProc(SkReadBuffer&) {
    return Make();
}

void SkLumaColorFilter::flatten(SkWriteBuffer&) const {}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkLumaColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorInfo&) const {
    return GrLumaColorFilterEffect::Make();
}
#endif
