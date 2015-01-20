/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverageSetOpXP_DEFINED
#define GrCoverageSetOpXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkRegion.h"

class GrProcOptInfo;

/**
 * This xfer processor directly blends the the src coverage with the dst using a set operator. It is
 * useful for rendering coverage masks using CSG. It can optionally invert the src coverage before
 * applying the set operator.
 * */
class GrCoverageSetOpXP : public GrXferProcessor {
public:
    static GrXferProcessor* Create(SkRegion::Op regionOp, bool invertCoverage) {
        return SkNEW_ARGS(GrCoverageSetOpXP, (regionOp, invertCoverage));
    }

    ~GrCoverageSetOpXP() SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "Coverage Set Op"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE { return false; }

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* color,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE;

    bool invertCoverage() const { return fInvertCoverage; }

private:
    GrCoverageSetOpXP(SkRegion::Op regionOp, bool fInvertCoverage);

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
        const GrCoverageSetOpXP& xp = xpBase.cast<GrCoverageSetOpXP>();
        return (fRegionOp == xp.fRegionOp &&
                fInvertCoverage == xp.fInvertCoverage);
    }

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrCoverageSetOpXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkRegion::Op regionOp, bool invertCoverage = false);

    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE {
        return true;
    }

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE {
        return true;
    }

    bool canTweakAlphaForCoverage() const SK_OVERRIDE { return false; }

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

    bool willReadDst() const SK_OVERRIDE { return false; }

private:
    GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage);

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrCoverageSetOpXPFactory& xpf = xpfBase.cast<GrCoverageSetOpXPFactory>();
        return fRegionOp == xpf.fRegionOp;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXPFactory INHERITED;
};
#endif

