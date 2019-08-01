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

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/effects/generated/GrLumaColorFilterEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

bool SkLumaColorFilter::onAppendStages(const SkStageRec& rec, bool /*shaderIsOpaque*/) const {
    rec.fPipeline->append(SkRasterPipeline::bt709_luminance_or_luma_to_alpha);
    rec.fPipeline->append(SkRasterPipeline::clamp_0);
    rec.fPipeline->append(SkRasterPipeline::clamp_1);
    return true;
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
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return GrLumaColorFilterEffect::Make();
}
#endif
