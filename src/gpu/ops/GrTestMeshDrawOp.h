/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestMeshDrawOp_DEFINED
#define GrTestMeshDrawOp_DEFINED

#include "GrGeometryProcessor.h"
#include "GrOpFlushState.h"

#include "ops/GrMeshDrawOp.h"

/*
 * A simple solid color GrLegacyMeshDrawOp for testing purposes which doesn't ever combine.
 * Subclassing this in tests saves having to fill out some boiler plate methods.
 */
class GrTestMeshDrawOp : public GrLegacyMeshDrawOp {
public:
    const char* name() const override = 0;

protected:
    GrTestMeshDrawOp(uint32_t classID, const SkRect& bounds, GrColor color)
            : INHERITED(classID), fColor(color) {
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kYes);
    }

    GrColor color() const { return fColor; }

    bool usesLocalCoords() const { return fUsesLocalCoords; }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);
        fUsesLocalCoords = optimizations.readsLocalCoords();
    }

    bool onCombineIfPossible(GrOp*, const GrCaps&) override { return false; }

    GrColor fColor;
    bool fUsesLocalCoords = false;

    typedef GrLegacyMeshDrawOp INHERITED;
};

#endif
