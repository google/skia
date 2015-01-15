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
    static GrFragmentProcessor* Create(float k1, float k2, float k3, float k4, bool enforcePMColor,
                               GrTexture* background) {
        return SkNEW_ARGS(GrArithmeticFP, (k1, k2, k3, k4, enforcePMColor, background));
    }

    ~GrArithmeticFP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Arithmetic"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    bool onIsEqual(const GrFragmentProcessor&) const SK_OVERRIDE;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GrArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                   GrTexture* background);

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

class GrArithmeticXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(float k1, float k2, float k3, float k4, bool enforcePMColor) {
        return SkNEW_ARGS(GrArithmeticXP, (k1, k2, k3, k4, enforcePMColor));
    }

    ~GrArithmeticXP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Arithmetic"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* overrideColor,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE {
        blendInfo->fSrcBlend = kOne_GrBlendCoeff;
        blendInfo->fDstBlend = kZero_GrBlendCoeff;
        blendInfo->fBlendConstant = 0;
    }

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    GrArithmeticXP(float k1, float k2, float k3, float k4, bool enforcePMColor);

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
        const GrArithmeticXP& xp = xpBase.cast<GrArithmeticXP>();
        if (fK1 != xp.fK1 ||
            fK2 != xp.fK2 ||
            fK3 != xp.fK3 ||
            fK4 != xp.fK4 ||
            fEnforcePMColor != xp.fEnforcePMColor) {
            return false;
        }
        return true;
    }

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrArithmeticXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(float k1, float k2, float k3, float k4, bool enforcePMColor) {
        return SkNEW_ARGS(GrArithmeticXPFactory, (k1, k2, k3, k4, enforcePMColor));
    }

    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return GrArithmeticXP::Create(fK1, fK2, fK3, fK4, fEnforcePMColor);
    }

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE {
        return true;
    }

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

    bool canTweakAlphaForCoverage() const SK_OVERRIDE {
        return false;
    }

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

    bool willReadDst(const GrProcOptInfo& colorPOI,
                     const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

private:
    GrArithmeticXPFactory(float k1, float k2, float k3, float k4, bool enforcePMColor); 

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
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
