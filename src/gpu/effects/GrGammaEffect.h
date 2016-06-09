/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGammaEffect_DEFINED
#define GrGammaEffect_DEFINED

#include "GrFragmentProcessor.h"

class GrGammaEffect : public GrFragmentProcessor {
public:
    enum class Mode {
        kLinearToSRGB,
        kSRGBToLinear,
        kExponential,
    };

    /**
    * Creates an effect that applies a gamma curve.
    */
    static sk_sp<GrFragmentProcessor> Make(SkScalar gamma);

    const char* name() const override { return "Gamma"; }

    Mode mode() const { return fMode; }
    SkScalar gamma() const { return fGamma; }

private:
    GrGammaEffect(Mode mode, SkScalar gamma);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    Mode fMode;
    SkScalar fGamma;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
