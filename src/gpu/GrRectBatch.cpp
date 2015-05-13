/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRectBatch.h"

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrBatchTest.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPrimitiveProcessor.h"
#include "GrTemplates.h"

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes we
    have explicit local coords and sometimes not. We *could* always provide explicit local coords
    and just duplicate the positions when the caller hasn't provided a local coord rect, but we
    haven't seen a use case which frequently switches between local rect and no local rect draws.

    The color param is used to determine whether the opaque hint can be set on the draw state.
    The caller must populate the vertex colors itself.

    The vertex attrib order is always pos, color, [local coords].
 */
static const GrGeometryProcessor* create_rect_gp(bool hasExplicitLocalCoords,
                                                 GrColor color,
                                                 const SkMatrix* localMatrix) {
    uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType |
                     GrDefaultGeoProcFactory::kColor_GPType;
    flags |= hasExplicitLocalCoords ? GrDefaultGeoProcFactory::kLocalCoord_GPType : 0;
    if (localMatrix) {
        return GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), *localMatrix);
    } else {
        return GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), SkMatrix::I());
    }
}

class RectBatch : public GrBatch {
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
        return SkNEW_ARGS(RectBatch, (geometry));
    }

    const char* name() const override { return "RectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        // Go to device coords to allow batching across matrix changes
        SkMatrix invert = SkMatrix::I();

        // if we have a local rect, then we apply the localMatrix directly to the localRect to
        // generate vertex local coords
        bool hasExplicitLocalCoords = this->hasLocalRect();
        if (!hasExplicitLocalCoords) {
            if (!this->viewMatrix().isIdentity() && !this->viewMatrix().invert(&invert)) {
                SkDebugf("Could not invert\n");
                return;
            }

            if (this->hasLocalMatrix()) {
                invert.preConcat(this->localMatrix());
            }
        }

        SkAutoTUnref<const GrGeometryProcessor> gp(create_rect_gp(hasExplicitLocalCoords,
                                                                  this->color(),
                                                                  &invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(hasExplicitLocalCoords ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));
        QuadHelper helper;
        void* vertices = helper.init(batchTarget, vertexStride, instanceCount);

        if (!vertices) {
            return;
        }


        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            intptr_t offset = GrTCast<intptr_t>(vertices) + kVerticesPerQuad * i * vertexStride;
            SkPoint* positions = GrTCast<SkPoint*>(offset);

            positions->setRectFan(geom.fRect.fLeft, geom.fRect.fTop,
                                  geom.fRect.fRight, geom.fRect.fBottom, vertexStride);
            geom.fViewMatrix.mapPointsWithStride(positions, vertexStride, kVerticesPerQuad);

            if (geom.fHasLocalRect) {
                static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
                SkPoint* coords = GrTCast<SkPoint*>(offset + kLocalOffset);
                coords->setRectFan(geom.fLocalRect.fLeft, geom.fLocalRect.fTop,
                                   geom.fLocalRect.fRight, geom.fLocalRect.fBottom,
                                   vertexStride);
                if (geom.fHasLocalMatrix) {
                    geom.fLocalMatrix.mapPointsWithStride(coords, vertexStride, kVerticesPerQuad);
                }
            }

            static const int kColorOffset = sizeof(SkPoint);
            GrColor* vertColor = GrTCast<GrColor*>(offset + kColorOffset);
            for (int j = 0; j < 4; ++j) {
                *vertColor = geom.fColor;
                vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
            }
        }

        helper.issueDraw(batchTarget);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    RectBatch(const Geometry& geometry) {
        this->initClassID<RectBatch>();
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

    bool onCombineIfPossible(GrBatch* t) override {
        RectBatch* that = t->cast<RectBatch>();

        if (this->hasLocalRect() != that->hasLocalRect()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (!this->hasLocalRect() && this->usesLocalCoords()) {
            if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
                return false;
            }

            if (this->hasLocalMatrix() != that->hasLocalMatrix()) {
                return false;
            }

            if (this->hasLocalMatrix() && !this->localMatrix().cheapEqualTo(that->localMatrix())) {
                return false;
            }
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }
        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

namespace GrRectBatch {

GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkRect& rect,
                const SkRect* localRect,
                const SkMatrix* localMatrix) {
    RectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fViewMatrix = viewMatrix;
    geometry.fRect = rect;

    if (localRect) {
        geometry.fHasLocalRect = true;
        geometry.fLocalRect = *localRect;
    } else {
        geometry.fHasLocalRect = false;
    }

    if (localMatrix) {
        geometry.fHasLocalMatrix = true;
        geometry.fLocalMatrix = *localMatrix;
    } else {
        geometry.fHasLocalMatrix = false;
    }

    return RectBatch::Create(geometry);
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(RectBatch) {
    GrColor color = GrRandomColor(random);

    SkRect rect = GrTest::TestRect(random);
    SkRect localRect;
    bool hasLocalRect = random->nextBool();
    bool hasLocalMatrix = random->nextBool();

    SkMatrix viewMatrix;
    SkMatrix localMatrix;
    if (hasLocalRect) {
        viewMatrix = GrTest::TestMatrixInvertible(random);
        localRect = GrTest::TestRect(random);
    } else {
        viewMatrix = GrTest::TestMatrix(random);
    }

    if (hasLocalMatrix) {
        localMatrix = GrTest::TestMatrix(random);
    }

    return GrRectBatch::Create(color, viewMatrix, rect,
                               hasLocalRect ? &localRect : NULL,
                               hasLocalMatrix ? &localMatrix : NULL);
}

#endif
