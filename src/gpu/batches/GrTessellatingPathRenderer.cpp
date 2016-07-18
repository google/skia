/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTessellatingPathRenderer.h"

#include "GrAuditTrail.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrClip.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrMesh.h"
#include "GrPathUtils.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrTessellator.h"
#include "SkGeometry.h"

#include "batches/GrVertexBatch.h"

#include <stdio.h>

/*
 * This path renderer tessellates the path into triangles using GrTessellator, uploads the triangles
 * to a vertex buffer, and renders them with a single draw call. It does not currently do
 * antialiasing, so it must be used in conjunction with multisampling.
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
    StaticVertexAllocator(GrResourceProvider* resourceProvider, bool canMapVB)
      : fResourceProvider(resourceProvider)
      , fCanMapVB(canMapVB)
      , fVertices(nullptr) {
    }
    SkPoint* lock(int vertexCount) override {
        size_t size = vertexCount * sizeof(SkPoint);
        fVertexBuffer.reset(fResourceProvider->createBuffer(
            size, kVertex_GrBufferType, kStatic_GrAccessPattern, 0));
        if (!fVertexBuffer.get()) {
            return nullptr;
        }
        if (fCanMapVB) {
            fVertices = static_cast<SkPoint*>(fVertexBuffer->map());
        } else {
            fVertices = new SkPoint[vertexCount];
        }
        return fVertices;
    }
    void unlock(int actualCount) override {
        if (fCanMapVB) {
            fVertexBuffer->unmap();
        } else {
            fVertexBuffer->updateData(fVertices, actualCount * sizeof(SkPoint));
            delete[] fVertices;
        }
        fVertices = nullptr;
    }
    GrBuffer* vertexBuffer() { return fVertexBuffer.get(); }
private:
    SkAutoTUnref<GrBuffer> fVertexBuffer;
    GrResourceProvider* fResourceProvider;
    bool fCanMapVB;
    SkPoint* fVertices;
};

}  // namespace

GrTessellatingPathRenderer::GrTessellatingPathRenderer() {
}

bool GrTessellatingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This path renderer can draw fill styles but does not do antialiasing. It can do convex and
    // concave paths, but we'll leave the convex ones to simpler algorithms. We pass on paths that
    // have styles, though they may come back around after applying the styling information to the
    // geometry to create a filled path. We also skip paths that don't have a key since the real
    // advantage of this path renderer comes from caching the tessellated geometry.
    return !args.fShape->style().applies() && args.fShape->style().isSimpleFill() &&
           !args.fAntiAlias && args.fShape->hasUnstyledKey() && !args.fShape->knownToBeConvex();
}

class TessellatingPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static GrDrawBatch* Create(const GrColor& color,
                               const GrShape& shape,
                               const SkMatrix& viewMatrix,
                               SkRect clipBounds) {
        return new TessellatingPathBatch(color, shape, viewMatrix, clipBounds);
    }

    const char* name() const override { return "TessellatingPathBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any color overrides
        if (!overrides.readsColor()) {
            fColor = GrColor_ILLEGAL;
        }
        overrides.getOverrideColorIfSet(&fColor);
        fPipelineInfo = overrides;
    }

    void draw(Target* target, const GrGeometryProcessor* gp) const {
        GrResourceProvider* rp = target->resourceProvider();
        SkScalar screenSpaceTol = GrPathUtils::kDefaultTolerance;
        SkScalar tol = GrPathUtils::scaleToleranceToSrc(screenSpaceTol, fViewMatrix,
                                                        fShape.bounds());

        SkPath path;
        fShape.asPath(&path);
        bool inverseFill = path.isInverseFillType();
        // construct a cache key from the path's genID and the view matrix
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        static constexpr int kClipBoundsCnt = sizeof(fClipBounds) / sizeof(uint32_t);
        int shapeKeyDataCnt = fShape.unstyledKeySize();
        SkASSERT(shapeKeyDataCnt >= 0);
        GrUniqueKey::Builder builder(&key, kDomain, shapeKeyDataCnt + kClipBoundsCnt);
        fShape.writeUnstyledKey(&builder[0]);
        // For inverse fills, the tessellation is dependent on clip bounds.
        if (inverseFill) {
            memcpy(&builder[shapeKeyDataCnt], &fClipBounds, sizeof(fClipBounds));
        } else {
            memset(&builder[shapeKeyDataCnt], 0, sizeof(fClipBounds));
        }
        builder.finish();
        SkAutoTUnref<GrBuffer> cachedVertexBuffer(rp->findAndRefTByUniqueKey<GrBuffer>(key));
        int actualCount;
        if (cache_match(cachedVertexBuffer.get(), tol, &actualCount)) {
            this->drawVertices(target, gp, cachedVertexBuffer.get(), 0, actualCount);
            return;
        }

        bool isLinear;
        bool canMapVB = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
        StaticVertexAllocator allocator(rp, canMapVB);
        int count = GrTessellator::PathToTriangles(path, tol, fClipBounds, &allocator, &isLinear);
        if (count == 0) {
            return;
        }
        this->drawVertices(target, gp, allocator.vertexBuffer(), 0, count);
        TessInfo info;
        info.fTolerance = isLinear ? 0 : tol;
        info.fCount = count;
        SkAutoTUnref<SkData> data(SkData::NewWithCopy(&info, sizeof(info)));
        key.setCustomData(data.get());
        rp->assignUniqueKeyToResource(key, allocator.vertexBuffer());
    }

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;

            Color color(fColor);
            LocalCoords localCoords(fPipelineInfo.readsLocalCoords() ?
                                    LocalCoords::kUsePosition_Type :
                                    LocalCoords::kUnused_Type);
            Coverage::Type coverageType;
            if (fPipelineInfo.readsCoverage()) {
                coverageType = Coverage::kSolid_Type;
            } else {
                coverageType = Coverage::kNone_Type;
            }
            Coverage coverage(coverageType);
            gp = GrDefaultGeoProcFactory::Make(color, coverage, localCoords, fViewMatrix);
        }
        this->draw(target, gp.get());
    }

    void drawVertices(Target* target, const GrGeometryProcessor* gp, const GrBuffer* vb,
                      int firstVertex, int count) const {
        SkASSERT(gp->getVertexStride() == sizeof(SkPoint));

        GrPrimitiveType primitiveType = TESSELLATOR_WIREFRAME ? kLines_GrPrimitiveType
                                                              : kTriangles_GrPrimitiveType;
        GrMesh mesh;
        mesh.init(primitiveType, vb, firstVertex, count);
        target->draw(gp, mesh);
    }

    bool onCombineIfPossible(GrBatch*, const GrCaps&) override { return false; }

    TessellatingPathBatch(const GrColor& color,
                          const GrShape& shape,
                          const SkMatrix& viewMatrix,
                          const SkRect& clipBounds)
      : INHERITED(ClassID())
      , fColor(color)
      , fShape(shape)
      , fViewMatrix(viewMatrix) {
        const SkRect& pathBounds = shape.bounds();
        fClipBounds = clipBounds;
        // Because the clip bounds are used to add a contour for inverse fills, they must also
        // include the path bounds.
        fClipBounds.join(pathBounds);
        if (shape.inverseFilled()) {
            fBounds = fClipBounds;
        } else {
            fBounds = pathBounds;
        }
        viewMatrix.mapRect(&fBounds);
    }

    GrColor                 fColor;
    GrShape                 fShape;
    SkMatrix                fViewMatrix;
    SkRect                  fClipBounds; // in source space
    GrXPOverridesForBatch   fPipelineInfo;

    typedef GrVertexBatch INHERITED;
};

bool GrTessellatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrTessellatingPathRenderer::onDrawPath");
    SkASSERT(!args.fAntiAlias);

    SkIRect clipBoundsI;
    args.fClip->getConservativeBounds(args.fDrawContext->width(), args.fDrawContext->height(),
                                      &clipBoundsI);
    SkRect clipBounds = SkRect::Make(clipBoundsI);
    SkMatrix vmi;
    if (!args.fViewMatrix->invert(&vmi)) {
        return false;
    }
    vmi.mapRect(&clipBounds);
    SkPath path;
    args.fShape->asPath(&path);
    SkAutoTUnref<GrDrawBatch> batch(TessellatingPathBatch::Create(args.fColor, *args.fShape,
                                                                  *args.fViewMatrix, clipBounds));

    GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fDrawContext->mustUseHWAA(*args.fPaint));
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(TesselatingPathBatch) {
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPath path = GrTest::TestPath(random);
    SkRect clipBounds = GrTest::TestRect(random);
    SkMatrix vmi;
    bool result = viewMatrix.invert(&vmi);
    if (!result) {
        SkFAIL("Cannot invert matrix\n");
    }
    vmi.mapRect(&clipBounds);
    GrStyle style;
    do {
        GrTest::TestStyle(random, &style);
    } while (style.strokeRec().isHairlineStyle());
    GrShape shape(path, style);
    return TessellatingPathBatch::Create(color, shape, viewMatrix, clipBounds);
}

#endif
