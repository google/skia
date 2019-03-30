/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLumaColorFilter.h"
#include "SkColorData.h"
#include "SkEffectPriv.h"
#include "SkRasterPipeline.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "effects/GrLumaColorFilterEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#endif

bool SkLumaColorFilter::onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipeline::luminance_to_alpha);
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
