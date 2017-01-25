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
    };

    /**
     * Creates an effect that applies the sRGB transfer function (or its inverse)
     */
    static sk_sp<GrFragmentProcessor> Make(Mode mode);

    const char* name() const override { return "Gamma"; }

    Mode mode() const { return fMode; }

private:
    GrGammaEffect(Mode mode);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
