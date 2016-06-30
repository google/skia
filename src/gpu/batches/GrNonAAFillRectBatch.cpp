/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNonAAFillRectBatch.h"

#include "GrBatchFlushState.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPrimitiveProcessor.h"
#include "GrResourceProvider.h"
#include "GrQuad.h"
#include "GrVertexBatch.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes
    we  have explicit local coords and sometimes not. We *could* always provide explicit local
    coords and just duplicate the positions when the caller hasn't provided a local coord rect,
    but we haven't seen a use case which frequently switches between local rect and no local
    rect draws.

    The vertex attrib order is always pos, color, [local coords].
 */
static sk_sp<GrGeometryProcessor> make_gp(const SkMatrix& viewMatrix,
                                          bool readsCoverage,
                                          bool hasExplicitLocalCoords,
                                          const SkMatrix* localMatrix) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);

    // If we have perspective on the viewMatrix then we won't map on the CPU, nor will we map
    // the local rect on the cpu (in case the localMatrix also has perspective).
    // Otherwise, if we have a local rect, then we apply the localMatrix directly to the localRect
    // to generate vertex local coords
    if (viewMatrix.hasPerspective()) {
        LocalCoords localCoords(hasExplicitLocalCoords ? LocalCoords::kHasExplicit_Type :
                                                         LocalCoords::kUsePosition_Type,
                                localMatrix);
        return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, viewMatrix);
    } else if (hasExplicitLocalCoords) {
        LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
        return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I());
    } else {
        LocalCoords localCoords(LocalCoords::kUsePosition_Type, localMatrix);
        return GrDefaultGeoProcFactory::MakeForDeviceSpace(color, coverage, localCoords,
                                                           viewMatrix);
    }
}

static void tesselate(intptr_t vertices,
                      size_t vertexStride,
                      GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const GrQuad* localQuad) {
    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);

    positions->setRectFan(rect.fLeft, rect.fTop,
                          rect.fRight, rect.fBottom, vertexStride);

    if (!viewMatrix.hasPerspective()) {
        viewMatrix.mapPointsWithStride(positions, vertexStride, kVertsPerInstance);
    }

    // Setup local coords
    // TODO we should only do this if local coords are being read
    if (localQuad) {
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        for (int i = 0; i < kVertsPerInstance; i++) {
            SkPoint* coords = reinterpret_cast<SkPoint*>(vertices + kLocalOffset +
                              i * vertexStride);
            *coords = localQuad->point(i);
        }
    }

    static const int kColorOffset = sizeof(SkPoint);
    GrColor* vertColor = reinterpret_cast<GrColor*>(vertices + kColorOffset);
    for (int j = 0; j < 4; ++j) {
        *vertColor = color;
        vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
    }
}

class NonAAFillRectBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fRect;
        GrQuad fLocalQuad;
        GrColor fColor;
    };

    static NonAAFillRectBatch* Create() { return new NonAAFillRectBatch; }

    const char* name() const override { return Name(); }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# batched: %d\n", fGeoData.count());
        for (int i = 0; i < fGeoData.count(); ++i) {
            str.append(DumpInfo(fGeoData[i], i));
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        InitInvariantOutputCoverage(coverage);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        fOverrides = overrides;
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    // After seeding, the client should call init() so the Batch can initialize itself
    void init() {
        const Geometry& geo = fGeoData[0];
        SetBounds(geo, &fBounds);
    }

    void updateBoundsAfterAppend() {
        const Geometry& geo = fGeoData.back();
        UpdateBoundsAfterAppend(geo, &fBounds);
    }

private:
    static const char* Name() { return "NonAAFillRectBatch"; }

    static SkString DumpInfo(const Geometry& geo, int index) {
        SkString str;
        str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                    index,
                    geo.fColor,
                    geo.fRect.fLeft, geo.fRect.fTop, geo.fRect.fRight, geo.fRect.fBottom);
        return str;
    }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrXPOverridesForBatch& overrides) {
        return true;
    }

    static sk_sp<GrGeometryProcessor> MakeGP(const Geometry& geo,
                                             const GrXPOverridesForBatch& overrides) {
        sk_sp<GrGeometryProcessor> gp = make_gp(geo.fViewMatrix, overrides.readsCoverage(), true,
                                                nullptr);

        SkASSERT(gp->getVertexStride() ==
                sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrXPOverridesForBatch& overrides) {
        tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, &geo.fLocalQuad);
    }

    static void InitInvariantOutputCoverage(GrInitInvariantOutput* out) {
        out->setKnownSingleComponent(0xff);
    }

    static const GrBuffer* GetIndexBuffer(GrResourceProvider* rp) {
        return rp->refQuadIndexBuffer();
    }

    static void SetBounds(const Geometry& geo, SkRect* outBounds) {
        geo.fViewMatrix.mapRect(outBounds, geo.fRect);
    }

    static void UpdateBoundsAfterAppend(const Geometry& geo, SkRect* outBounds) {
        SkRect bounds = geo.fRect;
        geo.fViewMatrix.mapRect(&bounds);
        outBounds->join(bounds);
    }

    NonAAFillRectBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp(MakeGP(this->seedGeometry(), fOverrides));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();
        int instanceCount = fGeoData.count();

        SkAutoTUnref<const GrBuffer> indexBuffer(GetIndexBuffer(target->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerInstance,
                                     kIndicesPerInstance, instanceCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kVertsPerInstance * vertexStride;
            Tesselate(verts, vertexStride, fGeoData[i], fOverrides);
        }
        helper.recordDraw(target, gp.get());
    }

    const Geometry& seedGeometry() const { return fGeoData[0]; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        NonAAFillRectBatch* that = t->cast<NonAAFillRectBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!CanCombine(this->seedGeometry(), that->seedGeometry(), fOverrides)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

// We handle perspective in the local matrix or viewmatrix with special batches
class NonAAFillRectPerspectiveBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkMatrix fViewMatrix;
        SkMatrix fLocalMatrix;
        SkRect fRect;
        SkRect fLocalRect;
        GrColor fColor;
        bool fHasLocalMatrix;
        bool fHasLocalRect;
    };

    static NonAAFillRectPerspectiveBatch* Create() { return new NonAAFillRectPerspectiveBatch; }

    const char* name() const override { return Name(); }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# batched: %d\n", fGeoData.count());
        for (int i = 0; i < fGeoData.count(); ++i) {
            str.append(DumpInfo(fGeoData[i], i));
        }
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        InitInvariantOutputCoverage(coverage);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        fOverrides = overrides;
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    // After seeding, the client should call init() so the Batch can initialize itself
    void init() {
        const Geometry& geo = fGeoData[0];
        SetBounds(geo, &fBounds);
    }

    void updateBoundsAfterAppend() {
        const Geometry& geo = fGeoData.back();
        UpdateBoundsAfterAppend(geo, &fBounds);
    }

private:
    static const char* Name() { return "NonAAFillRectPerspectiveBatch"; }

    static SkString DumpInfo(const Geometry& geo, int index) {
        SkString str;
        str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                    index,
                    geo.fColor,
                    geo.fRect.fLeft, geo.fRect.fTop, geo.fRect.fRight, geo.fRect.fBottom);
        return str;
    }

    static bool CanCombine(const Geometry& mine, const Geometry& theirs,
                           const GrXPOverridesForBatch& overrides) {
        // We could batch across perspective vm changes if we really wanted to
        return mine.fViewMatrix.cheapEqualTo(theirs.fViewMatrix) &&
               mine.fHasLocalRect == theirs.fHasLocalRect &&
               (!mine.fHasLocalMatrix || mine.fLocalMatrix.cheapEqualTo(theirs.fLocalMatrix));
    }

    static sk_sp<GrGeometryProcessor> MakeGP(const Geometry& geo,
                                             const GrXPOverridesForBatch& overrides) {
        sk_sp<GrGeometryProcessor> gp = make_gp(geo.fViewMatrix, overrides.readsCoverage(),
                                                geo.fHasLocalRect,
                                                geo.fHasLocalMatrix ? &geo.fLocalMatrix
                                                                    : nullptr);

        SkASSERT(geo.fHasLocalRect ?
             gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
             gp->getVertexStride() == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));
        return gp;
    }

    static void Tesselate(intptr_t vertices, size_t vertexStride, const Geometry& geo,
                          const GrXPOverridesForBatch& overrides) {
        if (geo.fHasLocalRect) {
            GrQuad quad(geo.fLocalRect);
            tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, &quad);
        } else {
            tesselate(vertices, vertexStride, geo.fColor, geo.fViewMatrix, geo.fRect, nullptr);
        }
    }

    static void InitInvariantOutputCoverage(GrInitInvariantOutput* out) {
        out->setKnownSingleComponent(0xff);
    }

    static const GrBuffer* GetIndexBuffer(GrResourceProvider* rp) {
        return rp->refQuadIndexBuffer();
    }

    static void SetBounds(const Geometry& geo, SkRect* outBounds) {
        geo.fViewMatrix.mapRect(outBounds, geo.fRect);
    }

    static void UpdateBoundsAfterAppend(const Geometry& geo, SkRect* outBounds) {
        SkRect bounds = geo.fRect;
        geo.fViewMatrix.mapRect(&bounds);
        outBounds->join(bounds);
    }

    NonAAFillRectPerspectiveBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp(MakeGP(this->seedGeometry(), fOverrides));
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();
        int instanceCount = fGeoData.count();

        SkAutoTUnref<const GrBuffer> indexBuffer(GetIndexBuffer(target->resourceProvider()));
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerInstance,
                                     kIndicesPerInstance, instanceCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kVertsPerInstance * vertexStride;
            Tesselate(verts, vertexStride, fGeoData[i], fOverrides);
        }
        helper.recordDraw(target, gp.get());
    }

    const Geometry& seedGeometry() const { return fGeoData[0]; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        NonAAFillRectPerspectiveBatch* that = t->cast<NonAAFillRectPerspectiveBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!CanCombine(this->seedGeometry(), that->seedGeometry(), fOverrides)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        this->joinBounds(that->bounds());
        return true;
    }

    GrXPOverridesForBatch fOverrides;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

namespace GrNonAAFillRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) {
    NonAAFillRectBatch* batch = NonAAFillRectBatch::Create();
    SkASSERT(!viewMatrix.hasPerspective() && (!localMatrix || !localMatrix->hasPerspective()));
    NonAAFillRectBatch::Geometry& geo = batch->geoData()->push_back();

    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fRect = rect;

    if (localRect && localMatrix) {
        geo.fLocalQuad.setFromMappedRect(*localRect, *localMatrix);
    } else if (localRect) {
        geo.fLocalQuad.set(*localRect);
    } else if (localMatrix) {
        geo.fLocalQuad.setFromMappedRect(rect, *localMatrix);
    } else {
        geo.fLocalQuad.set(rect);
    }
    batch->init();
    return batch;
}

GrDrawBatch* CreateWithPerspective(GrColor color,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkRect* localRect,
                                   const SkMatrix* localMatrix) {
    NonAAFillRectPerspectiveBatch* batch = NonAAFillRectPerspectiveBatch::Create();
    SkASSERT(viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective()));
    NonAAFillRectPerspectiveBatch::Geometry& geo = batch->geoData()->push_back();

    geo.fColor = color;
    geo.fViewMatrix = viewMatrix;
    geo.fRect = rect;
    geo.fHasLocalRect = SkToBool(localRect);
    geo.fHasLocalMatrix = SkToBool(localMatrix);
    if (localMatrix) {
        geo.fLocalMatrix = *localMatrix;
    }
    if (localRect) {
        geo.fLocalRect = *localRect;
    }
    batch->init();
    return batch;
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

DRAW_BATCH_TEST_DEFINE(RectBatch) {
    GrColor color = GrRandomColor(random);
    SkRect rect = GrTest::TestRect(random);
    SkRect localRect = GrTest::TestRect(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkMatrix localMatrix = GrTest::TestMatrix(random);

    bool hasLocalRect = random->nextBool();
    bool hasLocalMatrix = random->nextBool();
    return GrNonAAFillRectBatch::Create(color, viewMatrix, rect,
                                        hasLocalRect ? &localRect : nullptr,
                                        hasLocalMatrix ? &localMatrix : nullptr);
}

#endif
