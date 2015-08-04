/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticMode_gpu_DEFINED
#define SkArithmeticMode_gpu_DEFINED

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrCaps.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"

class GrInvariantOutput;
class GrProcOptInfo;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrGLArtithmeticFP;

class GrArithmeticFP : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager, float k1, float k2,
                                       float k3, float k4, bool enforcePMColor,
                                       GrTexture* background) {
        return SkNEW_ARGS(GrArithmeticFP, (procDataManager, k1, k2, k3, k4, enforcePMColor,
                                           background));
    }

    ~GrArithmeticFP() override {};

    const char* name() const override { return "Arithmetic"; }

    GrGLFragmentProcessor* createGLInstance() const override;

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrArithmeticFP(GrProcessorDataManager*, float k1, float k2, float k3, float k4,
                   bool enforcePMColor, GrTexture* background);

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;
    GrCoordTransform            fBackgroundTransform;
    GrTextureAccess             fBackgroundAccess;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class GrArithmeticXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(float k1, float k2, float k3, float k4, bool enforcePMColor) {
        return SkNEW_ARGS(GrArithmeticXPFactory, (k1, k2, k3, k4, enforcePMColor));
    }

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const override {
        return true;
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrArithmeticXPFactory(float k1, float k2, float k3, float k4, bool enforcePMColor); 

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override; 

    bool willReadDstColor(const GrCaps& caps,
                          const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI,
                          bool hasMixedSamples) const override {
        return true;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const GrArithmeticXPFactory& xpf = xpfBase.cast<GrArithmeticXPFactory>();
        if (fK1 != xpf.fK1 ||
            fK2 != xpf.fK2 ||
            fK3 != xpf.fK3 ||
            fK4 != xpf.fK4 ||
            fEnforcePMColor != xpf.fEnforcePMColor) {
            return false;
        }
        return true;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;

    typedef GrXPFactory INHERITED;
};

#endif
#endif
