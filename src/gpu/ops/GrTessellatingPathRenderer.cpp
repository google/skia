/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTessellatingPathRenderer.h"

#include "GrAuditTrail.h"
#include "GrClip.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMesh.h"
#include "GrOpFlushState.h"
#include "GrPathUtils.h"
#include "GrPipelineBuilder.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrTessellator.h"
#include "SkGeometry.h"

#include "ops/GrMeshDrawOp.h"

#include <stdio.h>

/*
 * This path renderer tessellates the path into triangles using GrTessellator, uploads the
 * triangles to a vertex buffer, and renders them with a single draw call. It can do screenspace
 * antialiasing with a one-pixel coverage ramp.
 */
namespace {

struct TessInfo {
    SkScalar  fTolerance;
    int       fCount;
};

// When the SkPathRef genID changes, invalidate a corresponding GrResource described by key.
class PathInvalidator : public SkPathRef::GenIDChangeListener {
public:
    explicit PathInvalidator(const GrUniqueKey& key) : fMsg(key) {}
private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void onChange() override {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg);
    }
};

bool cache_match(GrBuffer* vertexBuffer, SkScalar tol, int* actualCount) {
    if (!vertexBuffer) {
        return false;
    }
    const SkData* data = vertexBuffer->getUniqueKey().getCustomData();
    SkASSERT(data);
    const TessInfo* info = static_cast<const TessInfo*>(data->data());
    if (info->fTolerance == 0 || info->fTolerance < 3.0f * tol) {
        *actualCount = info->fCount;
        return true;
    }
    return false;
}

class StaticVertexAllocator : public GrTessellator::VertexAllocator {
public:
    StaticVertexAllocator(size_t stride, GrResourceProvider* resourceProvider, bool canMapVB)
      : VertexAllocator(stride)
      , fResourceProvider(resourceProvider)
      , fCanMapVB(canMapVB)
      , fVertices(nullptr) {
    }
    void* lock(int vertexCount) override {
        size_t size = vertexCount * stride();
        fVertexBuffer.reset(fResourceProvider->createBuffer(
            size, kVertex_GrBufferType, kStatic_GrAccessPattern, 0));
        if (!fVertexBuffer.get()) {
            return nullptr;
        }
        if (fCanMapVB) {
            fVertices = fVertexBuffer->map();
        } else {
            fVertices = sk_malloc_throw(vertexCount * stride());
        }
        return fVertices;
    }
    void unlock(int actualCount) override {
        if (fCanMapVB) {
            fVertexBuffer->unmap();
        } else {
            fVertexBuffer->updateData(fVertices, actualCount * stride());
            sk_free(fVertices);
        }
        fVertices = nullptr;
    }
    GrBuffer* vertexBuffer() { return fVertexBuffer.get(); }
private:
    sk_sp<GrBuffer> fVertexBuffer;
    GrResourceProvider* fResourceProvider;
    bool fCanMapVB;
    void* fVertices;
};

class DynamicVertexAllocator : public GrTessellator::VertexAllocator {
public:
    DynamicVertexAllocator(size_t stride, GrLegacyMeshDrawOp::Target* target)
            : VertexAllocator(stride)
            , fTarget(target)
            , fVertexBuffer(nullptr)
            , fVertices(nullptr) {}
    void* lock(int vertexCount) override {
        fVertexCount = vertexCount;
        fVertices = fTarget->makeVertexSpace(stride(), vertexCount, &fVertexBuffer, &fFirstVertex);
        return fVertices;
    }
    void unlock(int actualCount) override {
        fTarget->putBackVertices(fVertexCount - actualCount, stride());
        fVertices = nullptr;
    }
    const GrBuffer* vertexBuffer() const { return fVertexBuffer; }
    int firstVertex() const { return fFirstVertex; }
private:
    GrLegacyMeshDrawOp::Target* fTarget;
    const GrBuffer* fVertexBuffer;
    int fVertexCount;
    int fFirstVertex;
    void* fVertices;
};

}  // namespace

GrTessellatingPathRenderer::GrTessellatingPathRenderer() {
}

bool GrTessellatingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This path renderer can draw fill styles, and can do screenspace antialiasing via a
    // one-pixel coverage ramp. It can do convex and concave paths, but we'll leave the convex
    // ones to simpler algorithms. We pass on paths that have styles, though they may come back
    // around after applying the styling information to the geometry to create a filled path. In
    // the non-AA case, We skip paths thta don't have a key since the real advantage of this path
    // renderer comes from caching the tessellated geometry. In the AA case, we do not cache, so we
    // accept paths without keys.
    if (!args.fShape->style().isSimpleFill() || args.fShape->knownToBeConvex()) {
        return false;
    }
    if (GrAAType::kCoverage == args.fAAType) {
#ifdef SK_DISABLE_SCREENSPACE_TESS_AA_PATH_RENDERER
        return false;
#else
        SkPath path;
        args.fShape->asPath(&path);
        if (path.countVerbs() > 10) {
            return false;
        }
#endif
    } else if (!args.fShape->hasUnstyledKey()) {
        return false;
    }
    return true;
}

class TessellatingPathOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(const GrColor& color,
                                                    const GrShape& shape,
                                                    const SkMatrix& viewMatrix,
                                                    SkIRect devClipBounds,
                                                    bool antiAlias) {
        return std::unique_ptr<GrLegacyMeshDrawOp>(
                new TessellatingPathOp(color, shape, viewMatrix, devClipBounds, antiAlias));
    }

    const char* name() const override { return "TessellatingPathOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Color 0x%08x, aa: %d\n", fColor, fAntiAlias);
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);
        fCanTweakAlphaForCoverage = optimizations.canTweakAlphaForCoverage();
        fNeedsLocalCoords = optimizations.readsLocalCoords();
    }

    SkPath getPath() const {
        SkASSERT(!fShape.style().applies());
        SkPath path;
        fShape.asPath(&path);
        return path;
    }

    void draw(Target* target, const GrGeometryProcessor* gp) const {
        SkASSERT(!fAntiAlias);
        GrResourceProvider* rp = target->resourceProvider();
        bool inverseFill = fShape.inverseFilled();
        // construct a cache key from the path's genID and the view matrix
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        static constexpr int kClipBoundsCnt = sizeof(fDevClipBounds) / sizeof(uint32_t);
        int shapeKeyDataCnt = fShape.unstyledKeySize();
        SkASSERT(shapeKeyDataCnt >= 0);
        GrUniqueKey::Builder builder(&key, kDomain, shapeKeyDataCnt + kClipBoundsCnt);
        fShape.writeUnstyledKey(&builder[0]);
        // For inverse fills, the tessellation is dependent on clip bounds.
        if (inverseFill) {
            memcpy(&builder[shapeKeyDataCnt], &fDevClipBounds, sizeof(fDevClipBounds));
        } else {
            memset(&builder[shapeKeyDataCnt], 0, sizeof(fDevClipBounds));
        }
        builder.finish();
        sk_sp<GrBuffer> cachedVertexBuffer(rp->findAndRefTByUniqueKey<GrBuffer>(key));
        int actualCount;
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        tol = GrPathUtils::scaleToleranceToSrc(tol, fViewMatrix, fShape.bounds());
        if (cache_match(cachedVertexBuffer.get(), tol, &actualCount)) {
            this->drawVertices(target, gp, cachedVertexBuffer.get(), 0, actualCount);
            return;
        }

        SkRect clipBounds = SkRect::Make(fDevClipBounds);

        SkMatrix vmi;
        if (!fViewMatrix.invert(&vmi)) {
            return;
        }
        vmi.mapRect(&clipBounds);
        bool isLinear;
        bool canMapVB = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
        StaticVertexAllocator allocator(gp->getVertexStride(), rp, canMapVB);
        int count = GrTessellator::PathToTriangles(getPath(), tol, clipBounds, &allocator,
                                                   false, GrColor(), false, &isLinear);
        if (count == 0) {
            return;
        }
        this->drawVertices(target, gp, allocator.vertexBuffer(), 0, count);
        TessInfo info;
        info.fTolerance = isLinear ? 0 : tol;
        info.fCount = count;
        key.setCustomData(SkData::MakeWithCopy(&info, sizeof(info)));
        rp->assignUniqueKeyToResource(key, allocator.vertexBuffer());
    }

    void drawAA(Target* target, const GrGeometryProcessor* gp) const {
        SkASSERT(fAntiAlias);
        SkPath path = getPath();
        if (path.isEmpty()) {
            return;
        }
        SkRect clipBounds = SkRect::Make(fDevClipBounds);
        path.transform(fViewMatrix);
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        bool isLinear;
        DynamicVertexAllocator allocator(gp->getVertexStride(), target);
        int count = GrTessellator::PathToTriangles(path, tol, clipBounds, &allocator,
                                                   true, fColor, fCanTweakAlphaForCoverage,
                                                   &isLinear);
        if (count == 0) {
            return;
        }
        drawVertices(target, gp, allocator.vertexBuffer(), allocator.firstVertex(), count);
    }

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;

            Color color(fColor);
            LocalCoords::Type localCoordsType = fNeedsLocalCoords
                                                        ? LocalCoords::kUsePosition_Type
                                                        : LocalCoords::kUnused_Type;
            Coverage::Type coverageType;
            if (fAntiAlias) {
                color = Color(Color::kPremulGrColorAttribute_Type);
                if (fCanTweakAlphaForCoverage) {
                    coverageType = Coverage::kSolid_Type;
                } else {
                    coverageType = Coverage::kAttribute_Type;
                }
            } else {
                coverageType = Coverage::kSolid_Type;
            }
            if (fAntiAlias) {
                gp = GrDefaultGeoProcFactory::MakeForDeviceSpace(color, coverageType,
                                                                 localCoordsType, fViewMatrix);
            } else {
                gp = GrDefaultGeoProcFactory::Make(color, coverageType, localCoordsType,
                                                   fViewMatrix);
            }
        }
        if (!gp.get()) {
            return;
        }
        if (fAntiAlias) {
            this->drawAA(target, gp.get());
        } else {
            this->draw(target, gp.get());
        }
    }

    void drawVertices(Target* target, const GrGeometryProcessor* gp, const GrBuffer* vb,
                      int firstVertex, int count) const {
        GrPrimitiveType primitiveType = TESSELLATOR_WIREFRAME ? kLines_GrPrimitiveType
                                                              : kTriangles_GrPrimitiveType;
        GrMesh mesh;
        mesh.init(primitiveType, vb, firstVertex, count);
        target->draw(gp, this->pipeline(), mesh);
    }

    bool onCombineIfPossible(GrOp*, const GrCaps&) override { return false; }

    TessellatingPathOp(const GrColor& color,
                       const GrShape& shape,
                       const SkMatrix& viewMatrix,
                       const SkIRect& devClipBounds,
                       bool antiAlias)
            : INHERITED(ClassID())
            , fColor(color)
            , fShape(shape)
            , fViewMatrix(viewMatrix)
            , fDevClipBounds(devClipBounds)
            , fAntiAlias(antiAlias) {
        SkRect devBounds;
        viewMatrix.mapRect(&devBounds, shape.bounds());
        if (shape.inverseFilled()) {
            // Because the clip bounds are used to add a contour for inverse fills, they must also
            // include the path bounds.
            devBounds.join(SkRect::Make(fDevClipBounds));
        }
        this->setBounds(devBounds, HasAABloat::kNo, IsZeroArea::kNo);
    }

    GrColor                 fColor;
    GrShape                 fShape;
    SkMatrix                fViewMatrix;
    SkIRect                 fDevClipBounds;
    bool                    fAntiAlias;
    bool                    fCanTweakAlphaForCoverage;
    bool                    fNeedsLocalCoords;

    typedef GrLegacyMeshDrawOp INHERITED;
};

bool GrTessellatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrTessellatingPathRenderer::onDrawPath");
    SkIRect clipBoundsI;
    args.fClip->getConservativeBounds(args.fRenderTargetContext->width(),
                                      args.fRenderTargetContext->height(),
                                      &clipBoundsI);
    std::unique_ptr<GrLegacyMeshDrawOp> op =
            TessellatingPathOp::Make(args.fPaint.getColor(),
                                     *args.fShape,
                                     *args.fViewMatrix,
                                     clipBoundsI,
                                     GrAAType::kCoverage == args.fAAType);
    GrPipelineBuilder pipelineBuilder(std::move(args.fPaint), args.fAAType);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);
    args.fRenderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), *args.fClip,
                                                   std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

DRAW_OP_TEST_DEFINE(TesselatingPathOp) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPath path = GrTest::TestPath(random);
    SkIRect devClipBounds = SkIRect::MakeLTRB(
        random->nextU(), random->nextU(), random->nextU(), random->nextU());
    devClipBounds.sort();
    bool antiAlias = random->nextBool();
    GrStyle style;
    do {
        GrTest::TestStyle(random, &style);
    } while (!style.isSimpleFill());
    GrShape shape(path, style);
    return TessellatingPathOp::Make(color, shape, viewMatrix, devClipBounds, antiAlias);
}

#endif
