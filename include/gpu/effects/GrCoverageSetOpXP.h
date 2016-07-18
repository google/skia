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
 */
class GrCoverageSetOpXPFactory : public GrXPFactory {
public:
    static sk_sp<GrXPFactory> Make(SkRegion::Op regionOp, bool invertCoverage = false);

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage);

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool onWillReadDstColor(const GrCaps&, const GrPipelineOptimizations&) const override {
        return false;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const GrCoverageSetOpXPFactory& xpf = xpfBase.cast<GrCoverageSetOpXPFactory>();
        return fRegionOp == xpf.fRegionOp;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkRegion::Op fRegionOp;
    bool         fInvertCoverage;

    typedef GrXPFactory INHERITED;
};
#endif

