/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAppliedClip.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPrimitiveProcessor.h"
#include "GrQuad.h"
#include "GrRectOpFactory.h"
#include "GrResourceProvider.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkMatrixPriv.h"

static const int kVertsPerRect = 4;
static const int kIndicesPerRect = 6;

/** We always use per-vertex colors so that rects can be combined across color changes. Sometimes
    we  have explicit local coords and sometimes not. We *could* always provide explicit local
    coords and just duplicate the positions when the caller hasn't provided a local coord rect,
    but we haven't seen a use case which frequently switches between local rect and no local
    rect draws.

    The vertex attrib order is always pos, color, [local coords].
 */
static sk_sp<GrGeometryProcessor> make_gp() {
    using namespace GrDefaultGeoProcFactory;
    return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type, Coverage::kSolid_Type,
                                         LocalCoords::kHasExplicit_Type, SkMatrix::I());
}

static sk_sp<GrGeometryProcessor> make_perspective_gp(const SkMatrix& viewMatrix,
                                                      bool hasExplicitLocalCoords,
                                                      const SkMatrix* localMatrix) {
    SkASSERT(viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective()));

    using namespace GrDefaultGeoProcFactory;

    // If we have perspective on the viewMatrix then we won't map on the CPU, nor will we map
    // the local rect on the cpu (in case the localMatrix also has perspective).
    // Otherwise, if we have a local rect, then we apply the localMatrix directly to the localRect
    // to generate vertex local coords
    if (viewMatrix.hasPerspective()) {
        LocalCoords localCoords(hasExplicitLocalCoords ? LocalCoords::kHasExplicit_Type
                                                       : LocalCoords::kUsePosition_Type,
                                localMatrix);
        return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type,
                                             Coverage::kSolid_Type, localCoords, viewMatrix);
    } else if (hasExplicitLocalCoords) {
        LocalCoords localCoords(LocalCoords::kHasExplicit_Type, localMatrix);
        return GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type,
                                             Coverage::kSolid_Type, localCoords, SkMatrix::I());
    } else {
        LocalCoords localCoords(LocalCoords::kUsePosition_Type, localMatrix);
        return GrDefaultGeoProcFactory::MakeForDeviceSpace(Color::kPremulGrColorAttribute_Type,
                                                           Coverage::kSolid_Type, localCoords,
                                                           viewMatrix);
    }
}

static void tesselate(intptr_t vertices,
                      size_t vertexStride,
                      GrColor color,
                      const SkMatrix* viewMatrix,
                      const SkRect& rect,
                      const GrQuad* localQuad) {
    SkPoint* positions = reinterpret_cast<SkPoint*>(vertices);

    positions->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vertexStride);

    if (viewMatrix) {
        SkMatrixPriv::MapPointsWithStride(*viewMatrix, positions, vertexStride, kVertsPerRect);
    }

    // Setup local coords
    // TODO we should only do this if local coords are being read
    if (localQuad) {
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        for (int i = 0; i < kVertsPerRect; i++) {
            SkPoint* coords =
                    reinterpret_cast<SkPoint*>(vertices + kLocalOffset + i * vertexStride);
            *coords = localQuad->point(i);
        }
    }

    static const int kColorOffset = sizeof(SkPoint);
    GrColor* vertColor = reinterpret_cast<GrColor*>(vertices + kColorOffset);
    for (int j = 0; j < 4; ++j) {
        *vertColor = color;
        vertColor = (GrColor*)((intptr_t)vertColor + vertexStride);
    }
}

namespace {

class NonAAFillRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& rect, const SkRect* localRect,
                                          const SkMatrix* localMatrix, GrAAType aaType,
                                          const GrUserStencilSettings* stencilSettings) {
        SkASSERT(GrAAType::kCoverage != aaType);
        return Helper::FactoryHelper<NonAAFillRectOp>(std::move(paint), viewMatrix, rect, localRect,
                                                      localMatrix, aaType, stencilSettings);
    }

    NonAAFillRectOp() = delete;

    NonAAFillRectOp(const Helper::MakeArgs& args, GrColor color, const SkMatrix& viewMatrix,
                    const SkRect& rect, const SkRect* localRect, const SkMatrix* localMatrix,
                    GrAAType aaType, const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID()), fHelper(args, aaType, stencilSettings) {

        SkASSERT(!viewMatrix.hasPerspective() && (!localMatrix || !localMatrix->hasPerspective()));
        RectInfo& info = fRects.push_back();
        info.fColor = color;
        info.fViewMatrix = viewMatrix;
        info.fRect = rect;
        if (localRect && localMatrix) {
            info.fLocalQuad.setFromMappedRect(*localRect, *localMatrix);
        } else if (localRect) {
            info.fLocalQuad.set(*localRect);
        } else if (localMatrix) {
            info.fLocalQuad.setFromMappedRect(rect, *localMatrix);
        } else {
            info.fLocalQuad.set(rect);
        }
        this->setTransformedBounds(fRects[0].fRect, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "NonAAFillRectOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.append(GrMeshDrawOp::dumpInfo());
        str.appendf("# combined: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& info = fRects[i];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        info.fColor, info.fRect.fLeft, info.fRect.fTop, info.fRect.fRight,
                        info.fRect.fBottom);
        }
        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fRects.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone, color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    DEFINE_OP_CLASS_ID

private:
    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_gp();
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(gp->getVertexStride() ==
                 sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));

        size_t vertexStride = gp->getVertexStride();
        int rectCount = fRects.count();

        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        PatternHelper helper(GrPrimitiveType::kTriangles);
        void* vertices = helper.init(target, vertexStride, indexBuffer.get(), kVertsPerRect,
                                     kIndicesPerRect, rectCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < rectCount; i++) {
            intptr_t verts =
                    reinterpret_cast<intptr_t>(vertices) + i * kVertsPerRect * vertexStride;
            tesselate(verts, vertexStride, fRects[i].fColor, &fRects[i].fViewMatrix,
                      fRects[i].fRect, &fRects[i].fLocalQuad);
        }
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAAFillRectOp* that = t->cast<NonAAFillRectOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }
        fRects.push_back_n(that->fRects.count(), that->fRects.begin());
        this->joinBounds(*that);
        return true;
    }

    struct RectInfo {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        GrQuad fLocalQuad;
    };

    Helper fHelper;
    SkSTArray<1, RectInfo, true> fRects;
    typedef GrMeshDrawOp INHERITED;
};

// We handle perspective in the local matrix or viewmatrix with special ops.
class NonAAFillRectPerspectiveOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& rect, const SkRect* localRect,
                                          const SkMatrix* localMatrix, GrAAType aaType,
                                          const GrUserStencilSettings* stencilSettings) {
        SkASSERT(GrAAType::kCoverage != aaType);
        return Helper::FactoryHelper<NonAAFillRectPerspectiveOp>(std::move(paint), viewMatrix, rect,
                                                                 localRect, localMatrix, aaType,
                                                                 stencilSettings);
    }

    NonAAFillRectPerspectiveOp() = delete;

    NonAAFillRectPerspectiveOp(const Helper::MakeArgs& args, GrColor color,
                               const SkMatrix& viewMatrix, const SkRect& rect,
                               const SkRect* localRect, const SkMatrix* localMatrix,
                               GrAAType aaType, const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(args, aaType, stencilSettings)
            , fViewMatrix(viewMatrix) {
        SkASSERT(viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective()));
        RectInfo& info = fRects.push_back();
        info.fColor = color;
        info.fRect = rect;
        fHasLocalRect = SkToBool(localRect);
        fHasLocalMatrix = SkToBool(localMatrix);
        if (fHasLocalMatrix) {
            fLocalMatrix = *localMatrix;
        }
        if (fHasLocalRect) {
            info.fLocalRect = *localRect;
        }
        this->setTransformedBounds(rect, viewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
    }

    const char* name() const override { return "NonAAFillRectPerspectiveOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# combined: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& geo = fRects[0];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        geo.fColor, geo.fRect.fLeft, geo.fRect.fTop, geo.fRect.fRight,
                        geo.fRect.fBottom);
        }
        str += fHelper.dumpInfo();
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fRects.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone, color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    DEFINE_OP_CLASS_ID

private:
    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_perspective_gp(
                fViewMatrix, fHasLocalRect, fHasLocalMatrix ? &fLocalMatrix : nullptr);
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(fHasLocalRect
                         ? gp->getVertexStride() ==
                                   sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr)
                         : gp->getVertexStride() ==
                                   sizeof(GrDefaultGeoProcFactory::PositionColorAttr));

        size_t vertexStride = gp->getVertexStride();
        int rectCount = fRects.count();

        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        PatternHelper helper(GrPrimitiveType::kTriangles);
        void* vertices = helper.init(target, vertexStride, indexBuffer.get(), kVertsPerRect,
                                     kIndicesPerRect, rectCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < rectCount; i++) {
            const RectInfo& info = fRects[i];
            intptr_t verts =
                    reinterpret_cast<intptr_t>(vertices) + i * kVertsPerRect * vertexStride;
            if (fHasLocalRect) {
                GrQuad quad(info.fLocalRect);
                tesselate(verts, vertexStride, info.fColor, nullptr, info.fRect, &quad);
            } else {
                tesselate(verts, vertexStride, info.fColor, nullptr, info.fRect, nullptr);
            }
        }
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NonAAFillRectPerspectiveOp* that = t->cast<NonAAFillRectPerspectiveOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        // We could combine across perspective vm changes if we really wanted to.
        if (!fViewMatrix.cheapEqualTo(that->fViewMatrix)) {
            return false;
        }
        if (fHasLocalRect != that->fHasLocalRect) {
            return false;
        }
        if (fHasLocalMatrix && !fLocalMatrix.cheapEqualTo(that->fLocalMatrix)) {
            return false;
        }

        fRects.push_back_n(that->fRects.count(), that->fRects.begin());
        this->joinBounds(*that);
        return true;
    }

    struct RectInfo {
        SkRect fRect;
        GrColor fColor;
        SkRect fLocalRect;
    };

    SkSTArray<1, RectInfo, true> fRects;
    Helper fHelper;
    bool fHasLocalMatrix;
    bool fHasLocalRect;
    SkMatrix fLocalMatrix;
    SkMatrix fViewMatrix;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrRectOpFactory {

std::unique_ptr<GrDrawOp> MakeNonAAFill(GrPaint&& paint, const SkMatrix& viewMatrix,
                                        const SkRect& rect, GrAAType aaType,
                                        const GrUserStencilSettings* stencilSettings) {
    if (viewMatrix.hasPerspective()) {
        return NonAAFillRectPerspectiveOp::Make(std::move(paint), viewMatrix, rect, nullptr,
                                                nullptr, aaType, stencilSettings);
    } else {
        return NonAAFillRectOp::Make(std::move(paint), viewMatrix, rect, nullptr, nullptr, aaType,
                                     stencilSettings);
    }
}

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalMatrix(
        GrPaint&& paint, const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
        const SkRect& rect, GrAAType aaType, const GrUserStencilSettings* stencilSettings) {
    if (viewMatrix.hasPerspective() || localMatrix.hasPerspective()) {
        return NonAAFillRectPerspectiveOp::Make(std::move(paint), viewMatrix, rect, nullptr,
                                                &localMatrix, aaType, stencilSettings);
    } else {
        return NonAAFillRectOp::Make(std::move(paint), viewMatrix, rect, nullptr, &localMatrix,
                                     aaType, stencilSettings);
    }
}

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalRect(GrPaint&& paint, const SkMatrix& viewMatrix,
                                                     const SkRect& rect, const SkRect& localRect,
                                                     GrAAType aaType) {
    if (viewMatrix.hasPerspective()) {
        return NonAAFillRectPerspectiveOp::Make(std::move(paint), viewMatrix, rect, &localRect,
                                                nullptr, aaType, nullptr);
    } else {
        return NonAAFillRectOp::Make(std::move(paint), viewMatrix, rect, &localRect, nullptr,
                                     aaType, nullptr);
    }
}

}  // namespace GrRectOpFactory

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(NonAAFillRectOp) {
    SkRect rect = GrTest::TestRect(random);
    SkRect localRect = GrTest::TestRect(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkMatrix localMatrix = GrTest::TestMatrix(random);
    const GrUserStencilSettings* stencil = GrGetRandomStencil(random, context);
    GrAAType aaType = GrAAType::kNone;
    if (fsaaType == GrFSAAType::kUnifiedMSAA) {
        aaType = random->nextBool() ? GrAAType::kMSAA : GrAAType::kNone;
    }
    const SkRect* lr = random->nextBool() ? &localRect : nullptr;
    const SkMatrix* lm = random->nextBool() ? &localMatrix : nullptr;
    if (viewMatrix.hasPerspective() || (lm && lm->hasPerspective())) {
        return NonAAFillRectPerspectiveOp::Make(std::move(paint), viewMatrix, rect, lr, lm, aaType,
                                                stencil);
    } else {
        return NonAAFillRectOp::Make(std::move(paint), viewMatrix, rect, lr, lm, aaType, stencil);
    }
}

#endif
