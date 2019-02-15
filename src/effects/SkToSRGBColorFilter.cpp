/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkToSRGBColorFilter.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
    #include "GrColorSpaceXform.h"
#endif

void SkToSRGBColorFilter::onAppendStages(SkRasterPipeline* p,
                                         SkColorSpace* /*dst color space*/,
                                         SkArenaAlloc* alloc,
                                         bool shaderIsOpaque) const {
    bool shaderIsNormalized = false;
    alloc->make<SkColorSpaceXformSteps>(fSrcColorSpace.get(), kPremul_SkAlphaType,
                                        sk_srgb_singleton() , kPremul_SkAlphaType)
        ->apply(p, shaderIsNormalized);
}

sk_sp<SkColorFilter> SkToSRGBColorFilter::Make(sk_sp<SkColorSpace> srcColorSpace) {
    if (!srcColorSpace || srcColorSpace->isSRGB()) {
        return nullptr;
    } else {
        return sk_sp<SkColorFilter>(new SkToSRGBColorFilter(std::move(srcColorSpace)));
    }
}

SkToSRGBColorFilter::SkToSRGBColorFilter(sk_sp<SkColorSpace> srcColorSpace)
        : fSrcColorSpace(std::move(srcColorSpace)) {
    SkASSERT(fSrcColorSpace);
}

sk_sp<SkFlattenable> SkToSRGBColorFilter::CreateProc(SkReadBuffer& buffer) {
    auto data = buffer.readByteArrayAsData();
    return data ? Make(SkColorSpace::Deserialize(data->data(), data->size())) : nullptr;
}

void SkToSRGBColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeDataAsByteArray(fSrcColorSpace->serialize().get());
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkToSRGBColorFilter::asFragmentProcessor(
        GrRecordingContext*, const GrColorSpaceInfo&) const {
    return GrColorSpaceXformEffect::Make(fSrcColorSpace.get(), kPremul_SkAlphaType,
                                         sk_srgb_singleton(),  kPremul_SkAlphaType);
}
#endif
