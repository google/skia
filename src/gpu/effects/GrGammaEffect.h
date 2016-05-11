/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGammaEffect_DEFINED
#define GrGammaEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrGammaEffect : public GrSingleTextureEffect {
public:
    /**
     * Creates an effect that applies a gamma curve. The source texture is always
     * sampled unfiltered and with clamping.
     */
    static const GrFragmentProcessor* Create(GrTexture*, SkScalar gamma);

    const char* name() const override { return "Gamma"; }

    bool gammaIsSRGB() const { return fGammaIsSRGB; }
    SkScalar gamma() const { return fGamma; }

private:
    GrGammaEffect(GrTexture*, SkScalar gamma);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    bool fGammaIsSRGB;
    SkScalar fGamma;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
