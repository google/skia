/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConfigConversionEffect_DEFINED
#define GrConfigConversionEffect_DEFINED

#include "GrFragmentProcessor.h"

/**
 * This class is used to perform config conversions. Clients may want to read/write data that is
 * unpremultiplied.
 */
class GrConfigConversionEffect : public GrFragmentProcessor {
public:
    /**
     * The PM->UPM or UPM->PM conversions to apply.
     */
    enum PMConversion {
        kToPremul_PMConversion = 0,
        kToUnpremul_PMConversion,
        kPMConversionCnt
    };

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then performs
     *  the requested premul or unpremul conversion.
     */
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor>, PMConversion);

    const char* name() const override { return "Config Conversion"; }

    PMConversion  pmConversion() const { return fPMConversion; }

    // This function determines whether it is possible to choose PM->UPM and UPM->PM conversions
    // for which in any PM->UPM->PM->UPM sequence the two UPM values are the same. This means that
    // if pixels are read back to a UPM buffer, written back to PM to the GPU, and read back again
    // both reads will produce the same result. This test is quite expensive and should not be run
    // multiple times for a given context.
    static bool TestForPreservingPMConversions(GrContext* context);

private:
    GrConfigConversionEffect(PMConversion);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    PMConversion    fPMConversion;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};

#endif
