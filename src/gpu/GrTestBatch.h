/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTestBatch_DEFINED
#define GrTestBatch_DEFINED

#include "GrBatch.h"
#include "GrVertexBuffer.h"

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
        if (init.fColorIgnored) {
            this->geoData(0)->fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            this->geoData(0)->fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = this->geoData(0)->fColor;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        batchTarget->initDraw(fGeometryProcessor, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = fBatch.fUsesLocalCoords;
        fGeometryProcessor->initBatchTracker(batchTarget->currentBatchTracker(), init);

        this->onGenerateGeometry(batchTarget, pipeline);
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

    virtual void onGenerateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) = 0;

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
