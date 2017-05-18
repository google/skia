/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAlphaThresholdFragmentProcessor_DEFINED
#define GrAlphaThresholdFragmentProcessor_DEFINED

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrColorSpaceXform.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrProcessorUnitTest.h"

class GrAlphaThresholdFragmentProcessor : public GrFragmentProcessor {

public:
    static sk_sp<GrFragmentProcessor> Make(GrResourceProvider* resourceProvider,
                                           sk_sp<GrTextureProxy> proxy,
                                           sk_sp<GrColorSpaceXform> colorSpaceXform,
                                           sk_sp<GrTextureProxy> maskProxy,
                                           float innerThreshold,
                                           float outerThreshold,
                                           const SkIRect& bounds) {
        return sk_sp<GrFragmentProcessor>(new GrAlphaThresholdFragmentProcessor(
                                                                    resourceProvider,
                                                                    std::move(proxy),
                                                                    std::move(colorSpaceXform),
                                                                    std::move(maskProxy),
                                                                    innerThreshold, outerThreshold,
                                                                    bounds));
    }

    const char* name() const override { return "Alpha Threshold"; }

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

    GrColorSpaceXform* colorSpaceXform() const { return fColorSpaceXform.get(); }

private:
    static OptimizationFlags OptFlags(float outerThreshold);

    GrAlphaThresholdFragmentProcessor(GrResourceProvider*,
                                      sk_sp<GrTextureProxy> proxy,
                                      sk_sp<GrColorSpaceXform> colorSpaceXform,
                                      sk_sp<GrTextureProxy> maskProxy,
                                      float innerThreshold,
                                      float outerThreshold,
                                      const SkIRect& bounds);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    float fInnerThreshold;
    float fOuterThreshold;
    GrCoordTransform fImageCoordTransform;
    TextureSampler   fImageTextureSampler;
    // Color space transform is for the image (not the mask)
    sk_sp<GrColorSpaceXform> fColorSpaceXform;
    GrCoordTransform fMaskCoordTransform;
    TextureSampler   fMaskTextureSampler;

    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
