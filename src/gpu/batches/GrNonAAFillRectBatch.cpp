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

#include "SkMatrixPriv.h"
#include "SkExchange.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes
    we  have explicit local coords and sometimes not. We *could* always provide explicit local
    coords and just duplicate the positions when the caller hasn't provided a local coord rect,
    but we haven't seen a use case which frequently switches between local rect and no local
    rect draws.

    The vertex attrib order is always pos, color, [local coords].
 */
static sk_sp<GrGeometryProcessor> make_gp(bool readsCoverage) {
    using namespace GrDefaultGeoProcFactory;
    Color color(Color::kAttribute_Type);
    Coverage coverage(readsCoverage ? Coverage::kSolid_Type : Coverage::kNone_Type);

    LocalCoords localCoords(LocalCoords::kHasExplicit_Type);
    return GrDefaultGeoProcFactory::Make(color, coverage, localCoords, SkMatrix::I());
}

static void tesselate(intptr_t vertices,
                      size_t vertexStride,
                      GrColor color,
                      const SkMatrix* viewMatrix,
                      const SkRect& rect,
                      const GrQuad* localQuad) {
    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);

    positions->setRectFan(rect.fLeft, rect.fTop,
                          rect.fRight, rect.fBottom, vertexStride);

    if (viewMatrix) {
        SkMatrixPriv::MapPointsWithStride(*viewMatrix, positions, vertexStride, kVertsPerInstance);
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

    NonAAFillRectBatch(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                       const SkRect* localRect, const SkMatrix* localMatrix)
            : INHERITED(ClassID())
            , fHead(this->makeT<RectInfo>())
            , fTail(fHead)
            , fCount(1) {
        SkASSERT(!viewMatrix.hasPerspective() && (!localMatrix ||
                                                  !localMatrix->hasPerspective()));
        fHead->fColor = color;
        fHead->fViewMatrix = viewMatrix;
        fHead->fRect = rect;
        if (localRect && localMatrix) {
            fHead->fLocalQuad.setFromMappedRect(*localRect, *localMatrix);
        } else if (localRect) {
            fHead->fLocalQuad.set(*localRect);
        } else if (localMatrix) {
            fHead->fLocalQuad.setFromMappedRect(rect, *localMatrix);
        } else {
            fHead->fLocalQuad.set(rect);
        }
        this->setTransformedBounds(fHead->fRect, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    ~NonAAFillRectBatch() override {
        while(fHead) {
            this->freeSpace(skstd::exchange(fHead, fHead->fNext));
        }
    }

    const char* name() const override { return "NonAAFillRectBatch"; }

    SkString dumpInfo() const override {
        SkString str;
        const RectInfo* r = fHead;
        int i = 0;
        str.printf("# batched: %d\n", fCount);
        do {
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n",
                        i, r->fColor,
                        r->fRect.fLeft, r->fRect.fTop, r->fRect.fRight, r->fRect.fBottom);
            ++i;

        } while ((r = r->fNext));
        SkASSERT(i == fCount - 1);
        str.append(INHERITED::dumpInfo());
        return str;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fHead->fColor);
        coverage->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        overrides.getOverrideColorIfSet(&fHead->fColor);
        fOverrides = overrides;
    }

private:
    NonAAFillRectBatch() : INHERITED(ClassID()) {}

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_gp(fOverrides.readsCoverage());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(gp->getVertexStride() ==
                 sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));

        size_t vertexStride = gp->getVertexStride();

        SkAutoTUnref<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        InstancedHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer, kVertsPerInstance,
                                     kIndicesPerInstance, fCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const RectInfo* r = fHead;
        int i = 0;
        do {
            intptr_t verts = reinterpret_cast<intptr_t>(vertices) +
                             i * kVertsPerInstance * vertexStride;
            tesselate(verts, vertexStride, r->fColor, &r->fViewMatrix, r->fRect, &r->fLocalQuad);
            ++i;
        } while ((r = r->fNext));
        helper.recordDraw(target, gp.get());
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        NonAAFillRectBatch* that = t->cast<NonAAFillRectBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (fOverrides.canTweakAlphaForCoverage() && !that->fOverrides.canTweakAlphaForCoverage()) {
            fOverrides = that->fOverrides;
        }

        SkASSERT(!fTail->fNext);
        fTail->fNext = that->fHead;
        fTail = that->fTail;
        fCount += that->fCount;
        that->fHead = nullptr;
        SkASSERT(!fTail->fNext);
        this->joinBounds(*that);
        return true;
    }

    struct RectInfo {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        GrQuad fLocalQuad;
        RectInfo* fNext = nullptr;
    };

    RectInfo* fHead;
    RectInfo* fTail;
    int fCount;

    GrXPOverridesForBatch fOverrides;

    typedef GrVertexBatch INHERITED;
};

namespace GrNonAAFillRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) {
    return new NonAAFillRectBatch(color, viewMatrix, rect, localRect, localMatrix);
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
