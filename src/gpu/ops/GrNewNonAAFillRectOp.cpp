/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNewNonAAFillRectOp.h"

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPrimitiveProcessor.h"
#include "GrQuad.h"
#include "GrResourceProvider.h"

#include "SkMatrixPriv.h"

static const int kVertsPerInstance = 4;
static const int kIndicesPerInstance = 6;

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
        SkMatrixPriv::MapPointsWithStride(*viewMatrix, positions, vertexStride, kVertsPerInstance);
    }

    // Setup local coords
    // TODO we should only do this if local coords are being read
    if (localQuad) {
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        for (int i = 0; i < kVertsPerInstance; i++) {
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

template <bool TRIVIAL, typename DERIVED> class Base;

template<typename DERIVED> class Base<false, DERIVED> : public GrMeshDrawOp {
public:
    Base(GrPaint&& paint, uint32_t classID) : INHERITED(classID), fProcessors(std::move(paint)) {}

    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = static_cast<DERIVED*>(this)->color();
        fAnalysis = fProcessors.finalize(*color, GrProcessorAnalysisCoverage::kNone, clip, false,
                                         caps, color);
        return fAnalysis.requiresDstTexture();
    }

protected:
    using Target = GrMeshDrawOp::Target;
    using FixedFunctionFlags = GrDrawOp::FixedFunctionFlags;

    const GrProcessorSet& processors() const { return fProcessors; }
    bool compatibleProcessors(const Base& that) {
        return this->processors() == that.processors();
    }
    GrProcessorSet::Analysis analysis() const {
        SkASSERT(fAnalysis.isInitialized());
        return fAnalysis;
    }
private:
    GrProcessorSet fProcessors;
    GrProcessorSet::Analysis fAnalysis;

    using INHERITED = GrMeshDrawOp;
};

template<typename DERIVED> class Base<true, DERIVED> : public GrMeshDrawOp {
public:
    Base(GrPaint&& paint, uint32_t classID) : INHERITED(classID) {}

    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) override {
        return false;
    }

protected:
    using Target = GrMeshDrawOp::Target;
    using FixedFunctionFlags = GrDrawOp::FixedFunctionFlags;

    const GrProcessorSet& processors() const { return GrProcessorSet::EmptySet(); }
    bool compatibleProcessors(const Base& that) { return true; }
    GrProcessorSet::Analysis analysis() const { return GrProcessorSet::EmptySetAnalysis(); }

private:
    using INHERITED = GrMeshDrawOp;
};


template <bool TRIVIAL> class NewNonAAFillRectOp final : public Base<TRIVIAL, NewNonAAFillRectOp<TRIVIAL>> {
public:
    DEFINE_OP_CLASS_ID

    NewNonAAFillRectOp() = delete;

    NewNonAAFillRectOp(GrPaint&& paint, const SkMatrix& viewMatrix, const SkRect& rect,
                       const SkRect* localRect, const SkMatrix* localMatrix, bool useHWAA)
            : Base<TRIVIAL, NewNonAAFillRectOp<TRIVIAL>>(std::move(paint), ClassID())
            , fUseHWAA(useHWAA) {
        SkASSERT(!viewMatrix.hasPerspective() && (!localMatrix || !localMatrix->hasPerspective()));
        RectInfo& info = fRects.push_back();
        info.fColor = paint.getColor();
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
        this->setTransformedBounds(fRects[0].fRect, viewMatrix, GrOp::HasAABloat::kNo,
                                   GrOp::IsZeroArea::kNo);
    }

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const override {
        return fUseHWAA ? GrDrawOp::FixedFunctionFlags::kUsesHWAA :
                          GrDrawOp::FixedFunctionFlags::kNone;
    }

    const char* name() const override { return "NonAAFillRectOp"; }

    SkString dumpInfo() const override {
        SkString str;
        str.append(Base<TRIVIAL, NewNonAAFillRectOp<TRIVIAL>>::dumpInfo());
        str.appendf("# combined: %d\n", fRects.count());
        for (int i = 0; i < fRects.count(); ++i) {
            const RectInfo& info = fRects[i];
            str.appendf("%d: Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f]\n", i,
                        info.fColor, info.fRect.fLeft, info.fRect.fTop, info.fRect.fRight,
                        info.fRect.fBottom);
        }
        return str;
    }

    GrColor* color() {
        SkASSERT(fRects.count() == 1);
        return &fRects.front().fColor;
    }

private:
    void onPrepareDraws(GrMeshDrawOp::Target* target) const override {
        sk_sp<GrGeometryProcessor> gp = make_gp();
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor\n");
            return;
        }
        SkASSERT(gp->getVertexStride() ==
                 sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr));

        size_t vertexStride = gp->getVertexStride();
        int instanceCount = fRects.count();

        sk_sp<const GrBuffer> indexBuffer(target->resourceProvider()->refQuadIndexBuffer());
        GrMeshDrawOp::InstancedHelper helper;
        void* vertices =
                helper.init(target, kTriangles_GrPrimitiveType, vertexStride, indexBuffer.get(),
                            kVertsPerInstance, kIndicesPerInstance, instanceCount);
        if (!vertices || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            intptr_t verts =
                    reinterpret_cast<intptr_t>(vertices) + i * kVertsPerInstance * vertexStride;
            tesselate(verts, vertexStride, fRects[i].fColor, &fRects[i].fViewMatrix,
                      fRects[i].fRect, &fRects[i].fLocalQuad);
        }
        GrPipeline* pipeline = target->allocPipeline();
        GrPipeline::InitArgs args;
        args.fFlags = fUseHWAA ? GrPipeline::Flags::kHWAntialias_Flag : 0;
        args.fProcessors = &this->processors();
        args.fRenderTarget = target->renderTarget();
        args.fAppliedClip = target->clip();
        args.fDstTexture = target->dstTexture();
        args.fCaps = &target->caps();
        pipeline->init(args);
        helper.recordDraw(target, gp.get(), pipeline);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        NewNonAAFillRectOp* that = t->cast<NewNonAAFillRectOp>();
        if (!this->compatibleProcessors(*that)) {
            return false;
        }
        if (fUseHWAA != that->fUseHWAA) {
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

    SkSTArray<1, RectInfo, true> fRects;
    bool fUseHWAA : 1;
};

namespace GrNewNonAAFillRectOp {

std::unique_ptr<GrMeshDrawOp> Make(GrPaint&& paint,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkRect* localRect,
                                   const SkMatrix* localMatrix,
                                   bool useHWAA) {
    if (paint.isTrivial()) {
        return std::unique_ptr<GrMeshDrawOp>(
                new NewNonAAFillRectOp<true>(std::move(paint), viewMatrix, rect, localRect, localMatrix,
                                       useHWAA));
    } else {
        return std::unique_ptr<GrMeshDrawOp>(
                new NewNonAAFillRectOp<false>(std::move(paint), viewMatrix, rect, localRect, localMatrix,
                                       useHWAA));
    }
}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

