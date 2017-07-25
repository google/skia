/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkToSRGBColorFilter.h"
#include "SkPM4f.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkRasterPipeline.h"
#include "SkString.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "effects/GrNonlinearColorSpaceXformEffect.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#endif

void SkToSRGBColorFilter::onAppendStages(SkRasterPipeline* p,
                                         SkColorSpace* dst,
                                         SkArenaAlloc* scratch,
                                         bool shaderIsOpaque) const {
    // TODO
}

sk_sp<SkColorFilter> SkToSRGBColorFilter::Make(sk_sp<SkColorSpace> srcColorSpace) {
    return sk_sp<SkColorFilter>(new SkToSRGBColorFilter(std::move(srcColorSpace)));
}

SkToSRGBColorFilter::SkToSRGBColorFilter(sk_sp<SkColorSpace> srcColorSpace)
        : fSrcColorSpace(std::move(srcColorSpace)) {}

sk_sp<SkFlattenable> SkToSRGBColorFilter::CreateProc(SkReadBuffer&) {
    // TODO
    return nullptr;
}

void SkToSRGBColorFilter::flatten(SkWriteBuffer&) const {
    // TODO
}

#ifndef SK_IGNORE_TO_STRING
void SkToSRGBColorFilter::toString(SkString* str) const {
    // TODO
    str->append("SkToSRGBColorFilter ");
}
#endif

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkToSRGBColorFilter::asFragmentProcessor(GrContext*,
                                                                    SkColorSpace*) const {
    return GrNonlinearColorSpaceXformEffect::Make(fSrcColorSpace.get(),
                                                  SkColorSpace::MakeSRGB().get());
}
#endif
