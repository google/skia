/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestBatch_DEFINED
#define GrTestBatch_DEFINED

#include "GrBatchFlushState.h"
#include "GrGeometryProcessor.h"

#include "batches/GrVertexBatch.h"

/*
 * A simple solid color batch only for testing purposes which actually doesn't batch at all. It
 * saves having to fill out some boiler plate methods.
 */
class GrTestBatch : public GrVertexBatch {
public:
    virtual const char* name() const override = 0;

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fColor);
        coverage->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fColor);

        fOptimizations.fColorIgnored = !overrides.readsColor();
        fOptimizations.fUsesLocalCoords = overrides.readsLocalCoords();
        fOptimizations.fCoverageIgnored = !overrides.readsCoverage();
    }

protected:
    GrTestBatch(uint32_t classID, const SkRect& bounds, GrColor color)
        : INHERITED(classID)
        , fColor(color) {
        this->setBounds(bounds);
    }

    struct Optimizations {
        bool fColorIgnored = false;
        bool fUsesLocalCoords = false;
        bool fCoverageIgnored = false;
    };

    GrColor color() const { return fColor; }
    const Optimizations optimizations() const { return fOptimizations; }

private:
    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override {
        return false;
    }

    GrColor       fColor;
    Optimizations fOptimizations;

    typedef GrVertexBatch INHERITED;
};

#endif
