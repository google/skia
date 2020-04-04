/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "include/core/SkTypes.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrTextureDomain.h"

#include "include/core/SkYUVAIndex.h"

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    // The domain supported by this effect is more limited than the general GrTextureDomain due
    // to the multi-planar, varying resolution images that it has to sample. If 'domain' is provided
    // it is the Y plane's domain. This will automatically inset for bilinear filtering, and only
    // the clamp wrap mode is supported.
    static std::unique_ptr<GrFragmentProcessor> Make(const sk_sp<GrTextureProxy> proxies[],
                                                     const SkYUVAIndex indices[4],
                                                     SkYUVColorSpace yuvColorSpace,
                                                     GrSamplerState::Filter filterMode,
                                                     const SkMatrix& localMatrix = SkMatrix::I(),
                                                     const SkRect* domain = nullptr);
#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "YUVtoRGBEffect"; }

private:
    GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4], int numPlanes,
                     const SkYUVAIndex yuvaIndices[4], SkYUVColorSpace yuvColorSpace);

    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkYUVAIndex      fYUVAIndices[4];
    SkYUVColorSpace  fYUVColorSpace;
};
#endif
