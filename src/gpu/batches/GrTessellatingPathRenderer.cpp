/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTessellatingPathRenderer.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrPathUtils.h"
#include "GrVertices.h"
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

bool cache_match(GrVertexBuffer* vertexBuffer, SkScalar tol, int* actualCount) {
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

}  // namespace

GrTessellatingPathRenderer::GrTessellatingPathRenderer() {
}

bool GrTessellatingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This path renderer can draw all fill styles, all stroke styles except hairlines, but does
    // not do antialiasing. It can do convex and concave paths, but we'll leave the convex ones to
    // simpler algorithms.
    return !IsStrokeHairlineOrEquivalent(*args.fStroke, *args.fViewMatrix, nullptr) &&
           !args.fAntiAlias && !args.fPath->isConvex();
}

class TessellatingPathBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static GrDrawBatch* Create(const GrColor& color,
                               const SkPath& path,
                               const GrStrokeInfo& stroke,
                               const SkMatrix& viewMatrix,
                               SkRect clipBounds) {
        return new TessellatingPathBatch(color, path, stroke, viewMatrix, clipBounds);
    }

    const char* name() const override { return "TessellatingPathBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        color->setKnownFourComponents(fColor);
        coverage->setUnknownSingleComponent();
        overrides->fUsePLSDstRead = false;
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

    int tessellate(GrUniqueKey* key,
                   GrResourceProvider* resourceProvider,
                   SkAutoTUnref<GrVertexBuffer>& vertexBuffer,
                   bool canMapVB) const {
        SkPath path;
        GrStrokeInfo stroke(fStroke);
        if (stroke.isDashed()) {
            if (!stroke.applyDashToPath(&path, &stroke, fPath)) {
                return 0;
            }
        } else {
            path = fPath;
        }
        if (!stroke.isFillStyle()) {
            stroke.setResScale(SkScalarAbs(fViewMatrix.getMaxScale()));
            if (!stroke.applyToPath(&path, path)) {
                return 0;
            }
            stroke.setFillStyle();
        }
        SkScalar screenSpaceTol = GrPathUtils::kDefaultTolerance;
        SkRect pathBounds = path.getBounds();
        SkScalar tol = GrPathUtils::scaleToleranceToSrc(screenSpaceTol, fViewMatrix, pathBounds);

        bool isLinear;
        int count = GrTessellator::PathToTriangles(path, tol, fClipBounds, resourceProvider, 
                                                   vertexBuffer, canMapVB, &isLinear);
        if (!fPath.isVolatile()) {
            TessInfo info;
            info.fTolerance = isLinear ? 0 : tol;
            info.fCount = count;
            SkAutoTUnref<SkData> data(SkData::NewWithCopy(&info, sizeof(info)));
            key->setCustomData(data.get());
            resourceProvider->assignUniqueKeyToResource(*key, vertexBuffer.get());
            SkPathPriv::AddGenIDChangeListener(fPath, new PathInvalidator(*key));
        }
        return count;
    }

    void onPrepareDraws(Target* target) const override {
        // construct a cache key from the path's genID and the view matrix
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        int clipBoundsSize32 =
            fPath.isInverseFillType() ? sizeof(fClipBounds) / sizeof(uint32_t) : 0;
        int strokeDataSize32 = fStroke.computeUniqueKeyFragmentData32Cnt();
        GrUniqueKey::Builder builder(&key, kDomain, 2 + clipBoundsSize32 + strokeDataSize32);
        builder[0] = fPath.getGenerationID();
        builder[1] = fPath.getFillType();
        // For inverse fills, the tessellation is dependent on clip bounds.
        if (fPath.isInverseFillType()) {
            memcpy(&builder[2], &fClipBounds, sizeof(fClipBounds));
        }
        fStroke.asUniqueKeyFragment(&builder[2 + clipBoundsSize32]);
        builder.finish();
        GrResourceProvider* rp = target->resourceProvider();
        SkAutoTUnref<GrVertexBuffer> vertexBuffer(rp->findAndRefTByUniqueKey<GrVertexBuffer>(key));
        int actualCount;
        SkScalar screenSpaceTol = GrPathUtils::kDefaultTolerance;
        SkScalar tol = GrPathUtils::scaleToleranceToSrc(
            screenSpaceTol, fViewMatrix, fPath.getBounds());
        if (!cache_match(vertexBuffer.get(), tol, &actualCount)) {
            bool canMapVB = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
            actualCount = this->tessellate(&key, rp, vertexBuffer, canMapVB);
        }

        if (actualCount == 0) {
            return;
        }

        SkAutoTUnref<const GrGeometryProcessor> gp;
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
            gp.reset(GrDefaultGeoProcFactory::Create(color, coverage, localCoords,
                                                     fViewMatrix));
        }

        target->initDraw(gp, this->pipeline());
        SkASSERT(gp->getVertexStride() == sizeof(SkPoint));

        GrPrimitiveType primitiveType = TESSELLATOR_WIREFRAME ? kLines_GrPrimitiveType
                                                              : kTriangles_GrPrimitiveType;
        GrVertices vertices;
        vertices.init(primitiveType, vertexBuffer.get(), 0, actualCount);
        target->draw(vertices);
    }

    bool onCombineIfPossible(GrBatch*, const GrCaps&) override { return false; }

    TessellatingPathBatch(const GrColor& color,
                          const SkPath& path,
                          const GrStrokeInfo& stroke,
                          const SkMatrix& viewMatrix,
                          const SkRect& clipBounds)
      : INHERITED(ClassID())
      , fColor(color)
      , fPath(path)
      , fStroke(stroke)
      , fViewMatrix(viewMatrix) {
        const SkRect& pathBounds = path.getBounds();
        fClipBounds = clipBounds;
        // Because the clip bounds are used to add a contour for inverse fills, they must also
        // include the path bounds.
        fClipBounds.join(pathBounds);
        if (path.isInverseFillType()) {
            fBounds = fClipBounds;
        } else {
            fBounds = path.getBounds();
        }
        if (!stroke.isFillStyle()) {
            SkScalar radius = SkScalarHalf(stroke.getWidth());
            if (stroke.getJoin() == SkPaint::kMiter_Join) {
                SkScalar scale = stroke.getMiter();
                if (scale > SK_Scalar1) {
                    radius = SkScalarMul(radius, scale);
                }
            }
            fBounds.outset(radius, radius);
        }
        viewMatrix.mapRect(&fBounds);
    }

    GrColor                 fColor;
    SkPath                  fPath;
    GrStrokeInfo            fStroke;
    SkMatrix                fViewMatrix;
    SkRect                  fClipBounds; // in source space
    GrXPOverridesForBatch   fPipelineInfo;

    typedef GrVertexBatch INHERITED;
};

bool GrTessellatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fTarget->getAuditTrail(),
                              "GrTessellatingPathRenderer::onDrawPath");
    SkASSERT(!args.fAntiAlias);
    const GrRenderTarget* rt = args.fPipelineBuilder->getRenderTarget();
    if (nullptr == rt) {
        return false;
    }

    SkIRect clipBoundsI;
    args.fPipelineBuilder->clip().getConservativeBounds(rt->width(), rt->height(), &clipBoundsI);
    SkRect clipBounds = SkRect::Make(clipBoundsI);
    SkMatrix vmi;
    if (!args.fViewMatrix->invert(&vmi)) {
        return false;
    }
    vmi.mapRect(&clipBounds);
    SkAutoTUnref<GrDrawBatch> batch(TessellatingPathBatch::Create(args.fColor, *args.fPath,
                                                                  *args.fStroke, *args.fViewMatrix,
                                                                  clipBounds));
    args.fTarget->drawBatch(*args.fPipelineBuilder, batch);

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
    GrStrokeInfo strokeInfo = GrTest::TestStrokeInfo(random);
    return TessellatingPathBatch::Create(color, path, strokeInfo, viewMatrix, clipBounds);
}

#endif
