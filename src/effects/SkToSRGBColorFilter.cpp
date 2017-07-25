/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkString.h"
#include "SkToSRGBColorFilter.h"

#if SK_SUPPORT_GPU
    #include "effects/GrNonlinearColorSpaceXformEffect.h"
#endif

void SkToSRGBColorFilter::onAppendStages(SkRasterPipeline* p,
                                         SkColorSpace* /*dst color space*/,
                                         SkArenaAlloc* alloc,
                                         bool shaderIsOpaque) const {
    if (fSrcColorSpace->isSRGB()) {
        // That was easy.
        return;
    }

    // Step 1: Linearize by undoing the src transfer function.
    // Linear and sRGB will return true to isNumericalTransferFn(), so we check them first.
    SkColorSpaceTransferFn srcFn;
    if (fSrcColorSpace->gammaIsLinear()) {
        // Nothing to do.
    } else if (fSrcColorSpace->gammaCloseToSRGB()) {
        p->append_from_srgb(shaderIsOpaque ? kOpaque_SkAlphaType
                                           : kPremul_SkAlphaType);
    } else if (fSrcColorSpace->isNumericalTransferFn(&srcFn)) {
        p->append(SkRasterPipeline::parametric_r, &srcFn);
        p->append(SkRasterPipeline::parametric_g, &srcFn);
        p->append(SkRasterPipeline::parametric_b, &srcFn);
    } else {
        SkDEBUGFAIL("Looks like we got a table transfer function here, quite unexpectedly.");
        // TODO: If we really need to handle this, we can, but I don't think Ganesh does.
    }

    // Step 2: Transform to sRGB gamut, without clamping.
    (void)append_gamut_transform_noclamp(p,
                                         alloc->makeArrayDefault<float>(12),
                                         fSrcColorSpace.get(),
                                         SkColorSpace::MakeSRGB().get());

    // Step 3: Back to sRGB encoding.
    p->append(SkRasterPipeline::to_srgb);
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
