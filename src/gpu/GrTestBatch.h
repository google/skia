/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestBatch_DEFINED
#define GrTestBatch_DEFINED

#include "GrVertexBuffer.h"

#include "batches/GrBatch.h"

/*
 * A simple batch only for testing purposes which actually doesn't batch at all, but can fit into
 * the batch pipeline and generate arbitrary geometry
 */
class GrTestBatch : public GrBatch {
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

    void initBatchTracker(const GrPipelineInfo& init) override {
        // Handle any color overrides
        if (!init.readsColor()) {
            this->geoData(0)->fColor = GrColor_ILLEGAL;
        }
        init.getOverrideColorIfSet(&this->geoData(0)->fColor);

        // setup batch properties
        fBatch.fColorIgnored = !init.readsColor();
        fBatch.fColor = this->geoData(0)->fColor;
        fBatch.fUsesLocalCoords = init.readsLocalCoords();
        fBatch.fCoverageIgnored = !init.readsCoverage();
    }

    void generateGeometry(GrBatchTarget* batchTarget) override {
        batchTarget->initDraw(fGeometryProcessor, this->pipeline());

        this->onGenerateGeometry(batchTarget);
    }

protected:
    GrTestBatch(const GrGeometryProcessor* gp, const SkRect& bounds) {
        fGeometryProcessor.reset(SkRef(gp));

        this->setBounds(bounds);
    }

    const GrGeometryProcessor* geometryProcessor() const { return fGeometryProcessor; }

private:
    virtual Geometry* geoData(int index) = 0;
    virtual const Geometry* geoData(int index) const = 0;

    bool onCombineIfPossible(GrBatch* t) override {
        return false;
    }

    virtual void onGenerateGeometry(GrBatchTarget* batchTarget) = 0;

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    SkAutoTUnref<const GrGeometryProcessor> fGeometryProcessor;
    BatchTracker fBatch;
};

#endif
