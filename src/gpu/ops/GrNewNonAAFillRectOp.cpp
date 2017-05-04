/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNewNonAAFillRectOp.h"
#include "GrAppliedClip.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPrimitiveProcessor.h"
#include "GrQuad.h"
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

class NewNonAAFillRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID
    NewNonAAFillRectOp() = delete;

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& rect, const SkRect* localRect,
                                          const SkMatrix* localMatrix, GrAAType aaType,
                                          const GrUserStencilSettings* stencilSettings) {
        SkASSERT(GrAAType::kCoverage != aaType);
        return Helper::FactoryHelper<NewNonAAFillRectOp>(std::move(paint), viewMatrix, rect,
                                                         localRect, localMatrix, aaType,
                                                         stencilSettings);
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
        return str;
    }

    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fRects.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone, color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    NewNonAAFillRectOp(const Helper::MakeArgs& args, GrColor color, const SkMatrix& viewMatrix,
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
        PatternHelper helper;
        void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                     indexBuffer.get(), kVertsPerRect, kIndicesPerRect, rectCount);
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
        NewNonAAFillRectOp* that = t->cast<NewNonAAFillRectOp>();
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

namespace GrNewNonAAFillRectOp {

std::unique_ptr<GrDrawOp> Make(GrPaint&& paint,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const SkRect* localRect,
                               const SkMatrix* localMatrix,
                               GrAAType aaType,
                               const GrUserStencilSettings* stencilSettings) {
    return NewNonAAFillRectOp::Make(std::move(paint), viewMatrix, rect, localRect, localMatrix,
                                    aaType, stencilSettings);
}
};  // namespace GrNewNonAAFillRectOp

///////////////////////////////////////////////////////////////////////////////////////////////////
