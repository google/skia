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
 * A simple solid color batch only for testing purposes which actually doesn't batch at all. It
 * saves having to fill out some boiler plate methods.
 */
class GrTestMeshDrawOp : public GrMeshDrawOp {
public:
    virtual const char* name() const override = 0;

protected:
    GrTestMeshDrawOp(uint32_t classID, const SkRect& bounds, GrColor color)
            : INHERITED(classID), fColor(color) {
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kYes);
    }

    void getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput* input) const override {
        input->pipelineColorInput()->setKnownFourComponents(fColor);
        input->pipelineCoverageInput()->setUnknownSingleComponent();
    }

    void applyPipelineOptimizations(const GrPipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);

        fOptimizations.fColorIgnored = !optimizations.readsColor();
        fOptimizations.fUsesLocalCoords = optimizations.readsLocalCoords();
        fOptimizations.fCoverageIgnored = !optimizations.readsCoverage();
    }

    struct Optimizations {
        bool fColorIgnored = false;
        bool fUsesLocalCoords = false;
        bool fCoverageIgnored = false;
    };

    GrColor color() const { return fColor; }
    const Optimizations optimizations() const { return fOptimizations; }

private:
    bool onCombineIfPossible(GrOp*, const GrCaps&) override { return false; }

    GrColor       fColor;
    Optimizations fOptimizations;

    typedef GrMeshDrawOp INHERITED;
};

#endif
