/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkLumaColorFilter.h"
#include "include/core/SkString.h"
#include "src/core/SkColorData.h"
#include "src/core/SkPM4f.h"
#include "src/core/SkRasterPipeline.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/effects/GrLumaColorFilterEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

void SkLumaColorFilter::onAppendStages(SkRasterPipeline* p,
                                       SkColorSpace* dst,
                                       SkArenaAlloc* scratch,
                                       bool shaderIsOpaque) const {
    p->append(SkRasterPipeline::luminance_to_alpha);
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
        GrContext*, const GrColorSpaceInfo&) const {
    return GrLumaColorFilterEffect::Make();
}
#endif
