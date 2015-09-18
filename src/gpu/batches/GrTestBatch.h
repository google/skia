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
#include "GrVertexBuffer.h"

#include "batches/GrVertexBatch.h"

/*
 * A simple batch only for testing purposes which actually doesn't batch at all, but can fit into
 * the batch pipeline and generate arbitrary geometry
 */
class GrTestBatch : public GrVertexBatch {
public:
    struct Geometry {
        GrColor fColor;
    };

    virtual const char* name() const override = 0;

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(this->geoData(0)->fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineOptimizations& opt) override {
        // Handle any color overrides
        if (!opt.readsColor()) {
            this->geoData(0)->fColor = GrColor_ILLEGAL;
        }
        opt.getOverrideColorIfSet(&this->geoData(0)->fColor);

        // setup batch properties
        fBatch.fColorIgnored = !opt.readsColor();
        fBatch.fColor = this->geoData(0)->fColor;
        fBatch.fUsesLocalCoords = opt.readsLocalCoords();
        fBatch.fCoverageIgnored = !opt.readsCoverage();
    }

protected:
    GrTestBatch(uint32_t classID, const GrGeometryProcessor* gp, const SkRect& bounds)
        : INHERITED(classID) {
        fGeometryProcessor.reset(SkRef(gp));

        this->setBounds(bounds);
    }

    const GrGeometryProcessor* geometryProcessor() const { return fGeometryProcessor; }

private:
    void onPrepareDraws(Target* target) override {
        target->initDraw(fGeometryProcessor, this->pipeline());
        this->generateGeometry(target);
    }

    virtual Geometry* geoData(int index) = 0;
    virtual const Geometry* geoData(int index) const = 0;

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override {
        return false;
    }

    virtual void generateGeometry(Target*) = 0;

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    SkAutoTUnref<const GrGeometryProcessor> fGeometryProcessor;
    BatchTracker fBatch;

    typedef GrVertexBatch INHERITED;
};

#endif
