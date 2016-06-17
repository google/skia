/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasBatch_DEFINED
#define GrDrawAtlasBatch_DEFINED

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrVertexBatch.h"

class GrDrawAtlasBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        GrColor                 fColor;
        SkTArray<uint8_t, true> fVerts;
    };

    static GrDrawBatch* Create(const Geometry& geometry, const SkMatrix& viewMatrix,
                               int spriteCount, const SkRSXform* xforms, const SkRect* rects,
                               const SkColor* colors) {
        return new GrDrawAtlasBatch(geometry, viewMatrix, spriteCount, xforms, rects, colors);
    }

    const char* name() const override { return "DrawAtlasBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        if (this->hasColors()) {
            color->setUnknownFourComponents();
        } else {
            color->setKnownFourComponents(fGeoData[0].fColor);
        }
        coverage->setKnownSingleComponent(0xff);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    void onPrepareDraws(Target*) const override;

    void initBatchTracker(const GrXPOverridesForBatch&) override;

    GrDrawAtlasBatch(const Geometry& geometry, const SkMatrix& viewMatrix, int spriteCount,
                     const SkRSXform* xforms, const SkRect* rects, const SkColor* colors);

    GrColor color() const { return fColor; }
    bool colorIgnored() const { return fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool hasColors() const { return fHasColors; }
    int quadCount() const { return fQuadCount; }
    bool coverageIgnored() const { return fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override;
    SkSTArray<1, Geometry, true> fGeoData;

    SkMatrix fViewMatrix;
    GrColor  fColor;
    int      fQuadCount;
    bool     fColorIgnored;
    bool     fCoverageIgnored;
    bool     fHasColors;

    typedef GrVertexBatch INHERITED;
};

#endif
