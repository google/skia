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

/*
 * AAFillRectBatch is templated to optionally allow the insertion of an additional
 * attribute for explicit local coordinates.
 * To use this template, an implementation must define the following static functions:
 *     A Geometry struct
 *
 *     bool CanCombineLocalCoords(const SkMatrix& mine, const SkMatrix& theirs,
 *                                bool usesLocalCoords)
 *
 *     GrDefaultGeoProcFactory::LocalCoords::Type LocalCoordsType()
 *
 *     bool StrideCheck(size_t vertexStride, bool canTweakAlphaForCoverage,
 *                      bool usesLocalCoords)
 *
 *     void FillInAttributes(intptr_t startVertex, size_t vertexStride,
 *                           SkPoint* fan0Position, const Geometry&)
 */
template <typename Base>
class AAFillRectBatch : public GrBatch {
public:
    typedef typename Base::Geometry Geometry;

    static AAFillRectBatch* Create() {
        return SkNEW(AAFillRectBatch);
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

        SkAutoTUnref<const GrGeometryProcessor> gp(CreateFillRectGP(canTweakAlphaForCoverage,
                                                                    this->viewMatrix(),
                                                                    this->usesLocalCoords(),
                                                                    Base::LocalCoordsType(),
                                                                    this->coverageIgnored()));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        batchTarget->initDraw(gp, this->pipeline());

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(Base::StrideCheck(vertexStride, canTweakAlphaForCoverage,
                                   this->usesLocalCoords()));
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
            this->generateAAFillRectGeometry(vertices,
                                             i * kVertsPerAAFillRect * vertexStride,
                                             vertexStride,
                                             fGeoData[i],
                                             canTweakAlphaForCoverage);
        }
        helper.issueDraw(batchTarget);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    // to avoid even the initial copy of the struct, we have a getter for the first item which
    // is used to seed the batch with its initial geometry.  After seeding, the client should call
    // init() so the Batch can initialize itself
    Geometry* geometry() { return &fGeoData[0]; }
    void init() {
        const Geometry& geo = fGeoData[0];
        this->setBounds(geo.fDevRect);
    }


private:
    AAFillRectBatch() {
        this->initClassID<AAFillRectBatch<Base>>();

        // Push back an initial geometry
        fGeoData.push_back();
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

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *t->pipeline(), t->bounds(),
                                caps)) {
            return false;
        }

        AAFillRectBatch* that = t->cast<AAFillRectBatch>();
        if (!Base::CanCombineLocalCoords(this->viewMatrix(), that->viewMatrix(),
                                         this->usesLocalCoords())) {
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
                                    const Geometry& args,
                                    bool tweakAlphaForCoverage) const {
        intptr_t verts = reinterpret_cast<intptr_t>(vertices) + offset;

        SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
        SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

        SkScalar inset = SkMinScalar(args.fDevRect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, args.fDevRect.height());

        if (args.fViewMatrix.rectStaysRect()) {
            set_inset_fan(fan0Pos, vertexStride, args.fDevRect, -SK_ScalarHalf, -SK_ScalarHalf);
            set_inset_fan(fan1Pos, vertexStride, args.fDevRect, inset,  inset);
        } else {
            // compute transformed (1, 0) and (0, 1) vectors
            SkVector vec[2] = {
              { args.fViewMatrix[SkMatrix::kMScaleX], args.fViewMatrix[SkMatrix::kMSkewY] },
              { args.fViewMatrix[SkMatrix::kMSkewX],  args.fViewMatrix[SkMatrix::kMScaleY] }
            };

            vec[0].normalize();
            vec[0].scale(SK_ScalarHalf);
            vec[1].normalize();
            vec[1].scale(SK_ScalarHalf);

            // create the rotated rect
            fan0Pos->setRectFan(args.fRect.fLeft, args.fRect.fTop,
                                args.fRect.fRight, args.fRect.fBottom, vertexStride);
            args.fViewMatrix.mapPointsWithStride(fan0Pos, vertexStride, 4);

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

        Base::FillInAttributes(verts, vertexStride, fan0Pos, args);

        // Make verts point to vertex color and then set all the color and coverage vertex attrs
        // values.
        verts += sizeof(SkPoint);
        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = args.fColor;
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
        GrColor scaledColor = (0xff == scale) ? args.fColor : SkAlphaMulQ(args.fColor, scale);

        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = args.fColor;
                *reinterpret_cast<float*>(verts + i * vertexStride +
                                          sizeof(GrColor)) = innerCoverage;
            }
        }
    }

    static const GrGeometryProcessor* CreateFillRectGP(
                                         bool tweakAlphaForCoverage,
                                         const SkMatrix& viewMatrix,
                                         bool usesLocalCoords,
                                         GrDefaultGeoProcFactory::LocalCoords::Type localCoordsType,
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

        // We assume the caller has inverted the viewmatrix
        LocalCoords localCoords(usesLocalCoords ? localCoordsType : LocalCoords::kUnused_Type);
        if (LocalCoords::kHasExplicit_Type == localCoordsType) {
            return GrDefaultGeoProcFactory::Create(color, coverage, localCoords, SkMatrix::I());
        } else {
            return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
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

class AAFillRectBatchNoLocalMatrixImp {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
        GrColor fColor;
    };

    inline static bool CanCombineLocalCoords(const SkMatrix& mine, const SkMatrix& theirs,
                                             bool usesLocalCoords) {
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        return !usesLocalCoords || mine.cheapEqualTo(theirs);
    }

    inline static GrDefaultGeoProcFactory::LocalCoords::Type LocalCoordsType() {
        return GrDefaultGeoProcFactory::LocalCoords::kUsePosition_Type;
    }

    inline static bool StrideCheck(size_t vertexStride, bool canTweakAlphaForCoverage,
                                   bool usesLocalCoords) {
        return canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr);
    }

    inline static void FillInAttributes(intptr_t, size_t, SkPoint*, const Geometry&) {}
};

class AAFillRectBatchLocalMatrixImp {
public:
    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        SkRect fDevRect;
        GrColor fColor;
    };

    inline static bool CanCombineLocalCoords(const SkMatrix& mine, const SkMatrix& theirs,
                                             bool usesLocalCoords) {
        return true;
    }

    inline static GrDefaultGeoProcFactory::LocalCoords::Type LocalCoordsType() {
        return GrDefaultGeoProcFactory::LocalCoords::kHasExplicit_Type;
    }

    inline static bool StrideCheck(size_t vertexStride, bool canTweakAlphaForCoverage,
                                   bool usesLocalCoords) {
        // Whomever created us should not have done so if there are no local coords
        SkASSERT(usesLocalCoords);
        return canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordCoverage);
    }

    inline static void FillInAttributes(intptr_t vertices, size_t vertexStride,
                                        SkPoint* fan0Pos, const Geometry& args) {
        SkMatrix invViewMatrix;
        if (!args.fViewMatrix.invert(&invViewMatrix)) {
            SkASSERT(false);
            invViewMatrix = SkMatrix::I();
        }
        SkMatrix localCoordMatrix;
        localCoordMatrix.setConcat(args.fLocalMatrix, invViewMatrix);
        SkPoint* fan0Loc = reinterpret_cast<SkPoint*>(vertices + vertexStride - sizeof(SkPoint));
        localCoordMatrix.mapPointsWithStride(fan0Loc, fan0Pos, vertexStride, 8);
    }
};

typedef AAFillRectBatch<AAFillRectBatchNoLocalMatrixImp> AAFillRectBatchNoLocalMatrix;
typedef AAFillRectBatch<AAFillRectBatchLocalMatrixImp> AAFillRectBatchLocalMatrix;

namespace GrAAFillRectBatch {

GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkRect& rect,
                const SkRect& devRect) {
    AAFillRectBatchNoLocalMatrix* batch = AAFillRectBatchNoLocalMatrix::Create();
    AAFillRectBatchNoLocalMatrix::Geometry& geo = *batch->geometry();
    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fRect = rect;
    geo.fDevRect = devRect;
    batch->init();
    return batch;
}

GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkMatrix& localMatrix,
                const SkRect& rect,
                const SkRect& devRect) {
    AAFillRectBatchLocalMatrix* batch = AAFillRectBatchLocalMatrix::Create();
    AAFillRectBatchLocalMatrix::Geometry& geo = *batch->geometry();
    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fLocalMatrix = localMatrix;
    geo.fRect = rect;
    geo.fDevRect = devRect;
    batch->init();
    return batch;
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

BATCH_TEST_DEFINE(AAFillRectBatch) {
    AAFillRectBatchNoLocalMatrix* batch = AAFillRectBatchNoLocalMatrix::Create();
    AAFillRectBatchNoLocalMatrix::Geometry& geo = *batch->geometry();
    geo.fColor = GrRandomColor(random);
    geo.fViewMatrix = GrTest::TestMatrix(random);
    geo.fRect = GrTest::TestRect(random);
    geo.fDevRect = GrTest::TestRect(random);
    batch->init();
    return batch;
}

BATCH_TEST_DEFINE(AAFillRectBatchLocalMatrix) {
    AAFillRectBatchLocalMatrix* batch = AAFillRectBatchLocalMatrix::Create();
    AAFillRectBatchLocalMatrix::Geometry& geo = *batch->geometry();
    geo.fColor = GrRandomColor(random);
    geo.fViewMatrix = GrTest::TestMatrix(random);
    geo.fLocalMatrix = GrTest::TestMatrix(random);
    geo.fRect = GrTest::TestRect(random);
    geo.fDevRect = GrTest::TestRect(random);
    batch->init();
    return batch;
}

#endif
