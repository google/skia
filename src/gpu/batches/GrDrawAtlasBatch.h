/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasBatch_DEFINED
#define GrDrawAtlasBatch_DEFINED

#include "GrBatch.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"

class GrDrawAtlasBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkTDArray<SkPoint> fPositions;
        SkTDArray<GrColor> fColors;
        SkTDArray<SkPoint> fLocalCoords;
    };
    
    static GrBatch* Create(const Geometry& geometry, const SkMatrix& viewMatrix,
                           const SkPoint* positions, int vertexCount,
                           const GrColor* colors, const SkPoint* localCoords,
                           const SkRect& bounds) {
        return SkNEW_ARGS(GrDrawAtlasBatch, (geometry, viewMatrix, positions,
                                           vertexCount, colors, localCoords, bounds));
    }
    
    const char* name() const override { return "DrawAtlasBatch"; }
    
    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        if (this->hasColors()) {
            out->setUnknownFourComponents();
        } else {
            out->setKnownFourComponents(fGeoData[0].fColor);
        }
    }
    
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
    }
    
    void initBatchTracker(const GrPipelineInfo& init) override;
    void generateGeometry(GrBatchTarget* batchTarget) override;
    
    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }
    
private:
    GrDrawAtlasBatch(const Geometry& geometry, const SkMatrix& viewMatrix,
                     const SkPoint* positions, int vertexCount,
                     const GrColor* colors, const SkPoint* localCoords, const SkRect& bounds);
    
    GrColor color() const { return fColor; }
    bool colorIgnored() const { return fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int vertexCount() const { return fVertexCount; }
    bool coverageIgnored() const { return fCoverageIgnored; }
    
    bool onCombineIfPossible(GrBatch* t) override;
    SkSTArray<1, Geometry, true> fGeoData;
    
    SkMatrix fViewMatrix;
    GrColor  fColor;
    int      fVertexCount;
    bool     fColorIgnored;
    bool     fCoverageIgnored;
    bool     fHasColors;
};

#endif
