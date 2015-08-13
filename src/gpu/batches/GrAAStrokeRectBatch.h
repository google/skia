/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAStrokeRectBatch_DEFINED
#define GrAAStrokeRectBatch_DEFINED

#include "GrColor.h"
#include "GrTypes.h"
#include "GrVertexBatch.h"
#include "SkMatrix.h"
#include "SkRect.h"

class GrAAStrokeRectBatch : public GrVertexBatch {
public:
    // TODO support AA rotated stroke rects by copying around view matrices
    struct Geometry {
        GrColor fColor;
        SkRect fDevOutside;
        SkRect fDevOutsideAssist;
        SkRect fDevInside;
        bool fMiterStroke;
    };

    static GrDrawBatch* Create(const Geometry& geometry, const SkMatrix& viewMatrix) {
        return SkNEW_ARGS(GrAAStrokeRectBatch, (geometry, viewMatrix));
    }

    const char* name() const override { return "AAStrokeRect"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineOptimizations&) override;

    void generateGeometry(GrBatchTarget* batchTarget) override;

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    GrAAStrokeRectBatch(const Geometry& geometry, const SkMatrix& viewMatrix)  {
        this->initClassID<GrAAStrokeRectBatch>();
        fBatch.fViewMatrix = viewMatrix;
        fGeoData.push_back(geometry);

        // If we have miterstroke then we inset devOutside and outset devOutsideAssist, so we need
        // the join for proper bounds
        fBounds = geometry.fDevOutside;
        fBounds.join(geometry.fDevOutsideAssist);
    }


    static const int kMiterIndexCnt = 3 * 24;
    static const int kMiterVertexCnt = 16;
    static const int kNumMiterRectsInIndexBuffer = 256;

    static const int kBevelIndexCnt = 48 + 36 + 24;
    static const int kBevelVertexCnt = 24;
    static const int kNumBevelRectsInIndexBuffer = 256;

    static const GrIndexBuffer* GetIndexBuffer(GrResourceProvider* resourceProvider,
                                               bool miterStroke);

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool miterStroke() const { return fBatch.fMiterStroke; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override;

    void generateAAStrokeRectGeometry(void* vertices,
                                      size_t offset,
                                      size_t vertexStride,
                                      int outerVertexNum,
                                      int innerVertexNum,
                                      GrColor color,
                                      const SkRect& devOutside,
                                      const SkRect& devOutsideAssist,
                                      const SkRect& devInside,
                                      bool miterStroke,
                                      bool tweakAlphaForCoverage) const;

    struct BatchTracker {
        SkMatrix fViewMatrix;
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fMiterStroke;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};


#endif
