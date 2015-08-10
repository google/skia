/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAFillRectBatch_DEFINED
#define GrAAFillRectBatch_DEFINED

#include "GrBatch.h"
#include "GrColor.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"

class GrAAFillRectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(GrAAFillRectBatch, (geometry));
    }

    const char* name() const override { return "AAFillRectBatch"; }

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
    GrAAFillRectBatch(const Geometry& geometry) {
        this->initClassID<GrAAFillRectBatch>();
        fGeoData.push_back(geometry);

        this->setBounds(geometry.fDevRect);
    }

    static const int kNumAAFillRectsInIndexBuffer = 256;
    static const int kVertsPerAAFillRect = 8;
    static const int kIndicesPerAAFillRect = 30;

    const GrIndexBuffer* getIndexBuffer(GrResourceProvider* resourceProvider);

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override;

    void generateAAFillRectGeometry(void* vertices,
                                    size_t offset,
                                    size_t vertexStride,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    bool tweakAlphaForCoverage) const;

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

#endif
