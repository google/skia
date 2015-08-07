/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesBatch_DEFINED
#define GrDrawVerticesBatch_DEFINED

#include "GrBatch.h"
#include "GrColor.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkTDArray.h"

class GrBatch;
class GrBatchTarget;
struct GrInitInvariantOutput;

class GrDrawVerticesBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkTDArray<SkPoint> fPositions;
        SkTDArray<uint16_t> fIndices;
        SkTDArray<GrColor> fColors;
        SkTDArray<SkPoint> fLocalCoords;
    };

    static GrBatch* Create(const Geometry& geometry, GrPrimitiveType primitiveType,
                           const SkMatrix& viewMatrix,
                           const SkPoint* positions, int vertexCount,
                           const uint16_t* indices, int indexCount,
                           const GrColor* colors, const SkPoint* localCoords,
                           const SkRect& bounds) {
        return SkNEW_ARGS(GrDrawVerticesBatch, (geometry, primitiveType, viewMatrix, positions,
                                                vertexCount, indices, indexCount, colors,
                                                localCoords, bounds));
    }

    const char* name() const override { return "DrawVerticesBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override;

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override;

    void initBatchTracker(const GrPipelineInfo& init) override;

    void generateGeometry(GrBatchTarget* batchTarget) override;

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    GrDrawVerticesBatch(const Geometry& geometry, GrPrimitiveType primitiveType,
                        const SkMatrix& viewMatrix,
                        const SkPoint* positions, int vertexCount,
                        const uint16_t* indices, int indexCount,
                        const GrColor* colors, const SkPoint* localCoords, const SkRect& bounds);

    GrPrimitiveType primitiveType() const { return fBatch.fPrimitiveType; }
    bool batchablePrimitiveType() const {
        return kTriangles_GrPrimitiveType == fBatch.fPrimitiveType ||
               kLines_GrPrimitiveType == fBatch.fPrimitiveType ||
               kPoints_GrPrimitiveType == fBatch.fPrimitiveType;
    }
    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool hasColors() const { return fBatch.fHasColors; }
    bool hasIndices() const { return fBatch.fHasIndices; }
    bool hasLocalCoords() const { return fBatch.fHasLocalCoords; }
    int vertexCount() const { return fBatch.fVertexCount; }
    int indexCount() const { return fBatch.fIndexCount; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override;

    struct BatchTracker {
        GrPrimitiveType fPrimitiveType;
        SkMatrix fViewMatrix;
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fHasColors;
        bool fHasIndices;
        bool fHasLocalCoords;
        int fVertexCount;
        int fIndexCount;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

#endif
