/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesBatch_DEFINED
#define GrDrawVerticesBatch_DEFINED

#include "GrColor.h"
#include "GrTypes.h"
#include "GrVertexBatch.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkTDArray.h"

class GrBatchFlushState;
struct GrInitInvariantOutput;

class GrDrawVerticesBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID


    GrDrawVerticesBatch(GrColor color, GrPrimitiveType primitiveType,
                        const SkMatrix& viewMatrix,
                        const SkPoint* positions, int vertexCount,
                        const uint16_t* indices, int indexCount,
                        const GrColor* colors, const SkPoint* localCoords, const SkRect& bounds);

    const char* name() const override { return "DrawVerticesBatch"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("PrimType: %d, VarColor: %d, VCount: %d, ICount: %d\n",
                       fPrimitiveType, fVariableColor, fVertexCount, fIndexCount);
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override;

private:
    void onPrepareDraws(Target*) const override;
    void initBatchTracker(const GrXPOverridesForBatch&) override;

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool batchablePrimitiveType() const {
        return kTriangles_GrPrimitiveType == fPrimitiveType ||
               kLines_GrPrimitiveType == fPrimitiveType ||
               kPoints_GrPrimitiveType == fPrimitiveType;
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override;

    struct Mesh {
        GrColor fColor; // Only used if there are no per-vertex colors
        SkTDArray<SkPoint> fPositions;
        SkTDArray<uint16_t> fIndices;
        SkTDArray<GrColor> fColors;
        SkTDArray<SkPoint> fLocalCoords;
    };

    GrPrimitiveType     fPrimitiveType;
    SkMatrix            fViewMatrix;
    bool                fVariableColor;
    int                 fVertexCount;
    int                 fIndexCount;
    bool                fCoverageIgnored; // comes from initBatchTracker.

    SkSTArray<1, Mesh, true> fMeshes;

    typedef GrVertexBatch INHERITED;
};

#endif
