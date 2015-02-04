/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDisableColorXP_DEFINED
#define GrDisableColorXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"

class GrProcOptInfo;

/**
 * This xfer processor disables color writing. Thus color and coverage and ignored and no blending
 * occurs. This XP is usful for things like stenciling.
 */
class GrDisableColorXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create() {
        return SkNEW(GrDisableColorXP);
    }

    ~GrDisableColorXP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Disable Color"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* color,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE {
        return GrXferProcessor::kIgnoreColor_OptFlag | GrXferProcessor::kIgnoreCoverage_OptFlag;
    }

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE;

private:
    GrDisableColorXP();

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
        return true;
    }

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrDisableColorXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create() {
        return SkNEW(GrDisableColorXPFactory);
    }

    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE {
        return true;
    }

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

    bool canTweakAlphaForCoverage() const SK_OVERRIDE { return true; }

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput* output) const SK_OVERRIDE {
        output->fBlendedColorFlags = 0;
        output->fWillBlendWithDst = 0;
    }

    bool willReadDst() const SK_OVERRIDE { return false; }

private:
    GrDisableColorXPFactory();

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        return true;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    typedef GrXPFactory INHERITED;
};

#endif

