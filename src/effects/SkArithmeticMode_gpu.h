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
    static const GrFragmentProcessor* Create(float k1, float k2, float k3, float k4,
                                             bool enforcePMColor, const GrFragmentProcessor* dst) {
        return new GrArithmeticFP(k1, k2, k3, k4, enforcePMColor, dst);
    }

    ~GrArithmeticFP() override {};

    const char* name() const override { return "Arithmetic"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("K1: %.2f K2: %.2f K3: %.2f K4: %.2f", fK1, fK2, fK3, fK4);
        return str;
    }

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GrArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                   const GrFragmentProcessor* dst);

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class GrArithmeticXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(float k1, float k2, float k3, float k4, bool enforcePMColor) {
        return new GrArithmeticXPFactory(k1, k2, k3, k4, enforcePMColor);
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrArithmeticXPFactory(float k1, float k2, float k3, float k4, bool enforcePMColor); 

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override; 

    bool willReadDstColor(const GrCaps& caps,
                          const GrPipelineOptimizations& optimizations,
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
