/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAFillRectBatch.h"

#include "GrBatch.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrTypes.h"
#include "SkMatrix.h"
#include "SkRect.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

static const GrGeometryProcessor* create_fill_rect_gp(bool tweakAlphaForCoverage,
                                                      const SkMatrix& viewMatrix,
                                                      bool usesLocalCoords,
                                                      bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;

    Color color(Color::kAttribute_Type);
    Coverage::Type coverageType;
    // TODO remove coverage if coverage is ignored
    /*if (coverageIgnored) {
        coverageType = Coverage::kNone_Type;
    } else*/ if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    Coverage coverage(coverageType);
    LocalCoords localCoords(usesLocalCoords ? LocalCoords::kUsePosition_Type :
                                              LocalCoords::kUnused_Type);
    return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

class AAFillRectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    static GrBatch* Create(GrColor color,
                           const SkMatrix& viewMatrix,
                           const SkRect& rect,
                           const SkRect& devRect) {
        return SkNEW_ARGS(AAFillRectBatch, (color, viewMatrix, rect, devRect));
    }

    const char* name() const override { return "AAFillRectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineOptimizations& opt) override {
        // Handle any color overrides
        if (!opt.readsColor()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        opt.getOverrideColorIfSet(&fGeoData[0].fColor);

        // setup batch properties
        fBatch.fColorIgnored = !opt.readsColor();
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = opt.readsLocalCoords();
        fBatch.fCoverageIgnored = !opt.readsCoverage();
        fBatch.fCanTweakAlphaForCoverage = opt.canTweakAlphaForCoverage();
    }

    void generateGeometry(GrBatchTarget* batchTarget) override {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        SkAutoTUnref<const GrGeometryProcessor> gp(create_fill_rect_gp(canTweakAlphaForCoverage,
                                                                       this->viewMatrix(),
                                                                       this->usesLocalCoords(),
                                                                       this->coverageIgnored()));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        batchTarget->initDraw(gp, this->pipeline());

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
        int instanceCount = fGeoData.count();

        SkAutoTUnref<const GrIndexBuffer> indexBuffer(this->getIndexBuffer(
            batchTarget->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(batchTarget, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerAAFillRect, kIndicesPerAAFillRect,
                                     instanceCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];
            this->generateAAFillRectGeometry(vertices,
                                             i * kVertsPerAAFillRect * vertexStride,
                                             vertexStride,
                                             args.fColor,
                                             args.fViewMatrix,
                                             args.fRect,
                                             args.fDevRect,
                                             canTweakAlphaForCoverage);
        }

        helper.issueDraw(batchTarget);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    AAFillRectBatch(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                      const SkRect& devRect) {
        this->initClassID<AAFillRectBatch>();
        Geometry& geometry = fGeoData.push_back();
        geometry.fRect = rect;
        geometry.fViewMatrix = viewMatrix;
        geometry.fDevRect = devRect;
        geometry.fColor = color;

        this->setBounds(geometry.fDevRect);
    }

    static const int kNumAAFillRectsInIndexBuffer = 256;
    static const int kVertsPerAAFillRect = 8;
    static const int kIndicesPerAAFillRect = 30;

    const GrIndexBuffer* getIndexBuffer(GrResourceProvider* resourceProvider) {
        GR_DEFINE_STATIC_UNIQUE_KEY(gAAFillRectIndexBufferKey);

        static const uint16_t gFillAARectIdx[] = {
            0, 1, 5, 5, 4, 0,
            1, 2, 6, 6, 5, 1,
            2, 3, 7, 7, 6, 2,
            3, 0, 4, 4, 7, 3,
            4, 5, 6, 6, 7, 4,
        };
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(gFillAARectIdx) == kIndicesPerAAFillRect);
        return resourceProvider->findOrCreateInstancedIndexBuffer(gFillAARectIdx,
            kIndicesPerAAFillRect, kNumAAFillRectsInIndexBuffer, kVertsPerAAFillRect,
            gAAFillRectIndexBufferKey);
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override {
        if (!this->pipeline()->isEqual(*t->pipeline())) {
            return false;
        }

        AAFillRectBatch* that = t->cast<AAFillRectBatch>();

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
            fBatch.fCanTweakAlphaForCoverage = false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    void generateAAFillRectGeometry(void* vertices,
                                    size_t offset,
                                    size_t vertexStride,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    bool tweakAlphaForCoverage) const {
        intptr_t verts = reinterpret_cast<intptr_t>(vertices) + offset;

        SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
        SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

        SkScalar inset = SkMinScalar(devRect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

        if (viewMatrix.rectStaysRect()) {
            set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
            set_inset_fan(fan1Pos, vertexStride, devRect, inset,  inset);
        } else {
            // compute transformed (1, 0) and (0, 1) vectors
            SkVector vec[2] = {
              { viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY] },
              { viewMatrix[SkMatrix::kMSkewX],  viewMatrix[SkMatrix::kMScaleY] }
            };

            vec[0].normalize();
            vec[0].scale(SK_ScalarHalf);
            vec[1].normalize();
            vec[1].scale(SK_ScalarHalf);

            // create the rotated rect
            fan0Pos->setRectFan(rect.fLeft, rect.fTop,
                                rect.fRight, rect.fBottom, vertexStride);
            viewMatrix.mapPointsWithStride(fan0Pos, vertexStride, 4);

            // Now create the inset points and then outset the original
            // rotated points

            // TL
            *((SkPoint*)((intptr_t)fan1Pos + 0 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) + vec[0] + vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) -= vec[0] + vec[1];
            // BL
            *((SkPoint*)((intptr_t)fan1Pos + 1 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) + vec[0] - vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) -= vec[0] - vec[1];
            // BR
            *((SkPoint*)((intptr_t)fan1Pos + 2 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) - vec[0] - vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) += vec[0] + vec[1];
            // TR
            *((SkPoint*)((intptr_t)fan1Pos + 3 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) - vec[0] + vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) += vec[0] - vec[1];
        }

        // Make verts point to vertex color and then set all the color and coverage vertex attrs
        // values.
        verts += sizeof(SkPoint);
        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = 0;
            }
        }

        int scale;
        if (inset < SK_ScalarHalf) {
            scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
            SkASSERT(scale >= 0 && scale <= 255);
        } else {
            scale = 0xff;
        }

        verts += 4 * vertexStride;

        float innerCoverage = GrNormalizeByteToFloat(scale);
        GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride +
                                          sizeof(GrColor)) = innerCoverage;
            }
        }
    }

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

namespace GrAAFillRectBatch {

GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkRect& rect,
                const SkRect& devRect) {
    return AAFillRectBatch::Create(color, viewMatrix, rect, devRect);
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

BATCH_TEST_DEFINE(AAFillRectBatch) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect devRect = GrTest::TestRect(random);
    return AAFillRectBatch::Create(color, viewMatrix, rect, devRect);
}

#endif
