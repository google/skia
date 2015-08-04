/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConfigConversionEffect_DEFINED
#define GrConfigConversionEffect_DEFINED

#include "GrSingleTextureEffect.h"

class GrInvariantOutput;

/**
 * This class is used to perform config conversions. Clients may want to read/write data that is
 * unpremultiplied. Also on some systems reading/writing BGRA or RGBA is faster. In those cases we
 * read/write using the faster path and perform an R/B swap in the shader if the client data is in
 * the slower config.
 */
class GrConfigConversionEffect : public GrSingleTextureEffect {
public:
    /**
     * The PM->UPM or UPM->PM conversions to apply.
     */
    enum PMConversion {
        kNone_PMConversion = 0,
        kMulByAlpha_RoundUp_PMConversion,
        kMulByAlpha_RoundDown_PMConversion,
        kDivByAlpha_RoundUp_PMConversion,
        kDivByAlpha_RoundDown_PMConversion,

        kPMConversionCnt
    };

    static const GrFragmentProcessor* Create(GrProcessorDataManager*, GrTexture*,
                                             bool swapRedAndBlue, PMConversion, const SkMatrix&);

    const char* name() const override { return "Config Conversion"; }

    GrGLFragmentProcessor* createGLInstance() const override;

    bool swapsRedAndBlue() const { return fSwapRedAndBlue; }
    PMConversion  pmConversion() const { return fPMConversion; }

    // This function determines whether it is possible to choose PM->UPM and UPM->PM conversions
    // for which in any PM->UPM->PM->UPM sequence the two UPM values are the same. This means that
    // if pixels are read back to a UPM buffer, written back to PM to the GPU, and read back again
    // both reads will produce the same result. This test is quite expensive and should not be run
    // multiple times for a given context.
    static void TestForPreservingPMConversions(GrContext* context,
                                               PMConversion* PMToUPMRule,
                                               PMConversion* UPMToPMRule);

private:
    GrConfigConversionEffect(GrProcessorDataManager*,
                             GrTexture*,
                             bool swapRedAndBlue,
                             PMConversion pmConversion,
                             const SkMatrix& matrix);

    void onGetGLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    bool            fSwapRedAndBlue;
    PMConversion    fPMConversion;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
