/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSRGBEffect_DEFINED
#define GrSRGBEffect_DEFINED

#include "GrFragmentProcessor.h"

class GrSRGBEffect : public GrFragmentProcessor {
public:
    enum class Mode {
        kLinearToSRGB,
        kSRGBToLinear,
    };

    /**
     * Creates an effect that applies the sRGB transfer function (or its inverse)
     */
    static sk_sp<GrFragmentProcessor> Make(Mode mode);

    const char* name() const override { return "sRGB"; }

    Mode mode() const { return fMode; }

private:
    GrSRGBEffect(Mode mode);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override;

    Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrFragmentProcessor INHERITED;
};

#endif
