/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatch_DEFINED
#define GrRectBatch_DEFINED

#include "GrBatch.h"
#include "GrColor.h"

class GrBatchTarget;
class SkMatrix;
struct SkRect;

class GrRectBatch : public GrBatch {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fLocalRect;
        SkMatrix fLocalMatrix;
        GrColor fColor;
        bool fHasLocalRect;
        bool fHasLocalMatrix;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(GrRectBatch, (geometry));
    }

    const char* name() const override { return "RectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrPipelineOptimizations&) override;

    void generateGeometry(GrBatchTarget* batchTarget) override;

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    GrRectBatch(const Geometry& geometry) {
        this->initClassID<GrRectBatch>();
        fGeoData.push_back(geometry);

        fBounds = geometry.fRect;
        geometry.fViewMatrix.mapRect(&fBounds);
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    const SkMatrix& localMatrix() const { return fGeoData[0].fLocalMatrix; }
    bool hasLocalRect() const { return fGeoData[0].fHasLocalRect; }
    bool hasLocalMatrix() const { return fGeoData[0].fHasLocalMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override;

    const GrGeometryProcessor* createRectGP();

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

#endif
