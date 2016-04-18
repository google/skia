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

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrProcessorUnitTest.h"

class GrAlphaThresholdFragmentProcessor : public GrFragmentProcessor {

public:
    static sk_sp<GrFragmentProcessor> Make(GrTexture* texture,
                                           GrTexture* maskTexture,
                                           float innerThreshold,
                                           float outerThreshold,
                                           const SkIRect& bounds);

    const char* name() const override { return "Alpha Threshold"; }

    float innerThreshold() const { return fInnerThreshold; }
    float outerThreshold() const { return fOuterThreshold; }

private:
    GrAlphaThresholdFragmentProcessor(GrTexture* texture,
                                      GrTexture* maskTexture,
                                      float innerThreshold,
                                      float outerThreshold,
                                      const SkIRect& bounds);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    float fInnerThreshold;
    float fOuterThreshold;
    GrCoordTransform fImageCoordTransform;
    GrTextureAccess  fImageTextureAccess;
    GrCoordTransform fMaskCoordTransform;
    GrTextureAccess  fMaskTextureAccess;

    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
