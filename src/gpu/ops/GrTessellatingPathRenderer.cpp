/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrTessellatingPathRenderer.h"
#include <stdio.h>
#include "src/core/SkGeometry.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTessellator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

#ifndef GR_AA_TESSELLATOR_MAX_VERB_COUNT
#define GR_AA_TESSELLATOR_MAX_VERB_COUNT 10
#endif

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
    PathInvalidator(const GrUniqueKey& key, uint32_t contextUniqueID)
            : fMsg(key, contextUniqueID) {}

private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void onChange() override {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg);
    }
};

bool cache_match(GrGpuBuffer* vertexBuffer, SkScalar tol, int* actualCount) {
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
        fVertexBuffer = fResourceProvider->createBuffer(size, GrGpuBufferType::kVertex,
                                                        kStatic_GrAccessPattern);
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
    sk_sp<GrGpuBuffer> detachVertexBuffer() { return std::move(fVertexBuffer); }

private:
    sk_sp<GrGpuBuffer> fVertexBuffer;
    GrResourceProvider* fResourceProvider;
    bool fCanMapVB;
    void* fVertices;
};

class DynamicVertexAllocator : public GrTessellator::VertexAllocator {
public:
    DynamicVertexAllocator(size_t stride, GrMeshDrawOp::Target* target)
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
    sk_sp<const GrBuffer> detachVertexBuffer() const { return std::move(fVertexBuffer); }
    int firstVertex() const { return fFirstVertex; }

private:
    GrMeshDrawOp::Target* fTarget;
    sk_sp<const GrBuffer> fVertexBuffer;
    int fVertexCount;
    int fFirstVertex;
    void* fVertices;
};

}  // namespace

GrTessellatingPathRenderer::GrTessellatingPathRenderer()
  : fMaxVerbCount(GR_AA_TESSELLATOR_MAX_VERB_COUNT) {
}

GrPathRenderer::CanDrawPath
GrTessellatingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    // This path renderer can draw fill styles, and can do screenspace antialiasing via a
    // one-pixel coverage ramp. It can do convex and concave paths, but we'll leave the convex
    // ones to simpler algorithms. We pass on paths that have styles, though they may come back
    // around after applying the styling information to the geometry to create a filled path.
    if (!args.fShape->style().isSimpleFill() || args.fShape->knownToBeConvex()) {
        return CanDrawPath::kNo;
    }
    switch (args.fAAType) {
        case GrAAType::kNone:
        case GrAAType::kMSAA:
            // Prefer MSAA, if any antialiasing. In the non-analytic-AA case, We skip paths that
            // don't have a key since the real advantage of this path renderer comes from caching
            // the tessellated geometry.
            if (!args.fShape->hasUnstyledKey()) {
                return CanDrawPath::kNo;
            }
            break;
        case GrAAType::kCoverage:
            // Use analytic AA if we don't have MSAA. In this case, we do not cache, so we accept
            // paths without keys.
            SkPath path;
            args.fShape->asPath(&path);
            if (path.countVerbs() > fMaxVerbCount) {
                return CanDrawPath::kNo;
            }
            break;
    }
    return CanDrawPath::kYes;
}

namespace {

class TessellatingPathOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const GrShape& shape,
                                          const SkMatrix& viewMatrix,
                                          SkIRect devClipBounds,
                                          GrAAType aaType,
                                          const GrUserStencilSettings* stencilSettings) {
        return Helper::FactoryHelper<TessellatingPathOp>(context, std::move(paint), shape,
                                                         viewMatrix, devClipBounds,
                                                         aaType, stencilSettings);
    }

    const char* name() const override { return "TessellatingPathOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Color 0x%08x, aa: %d\n", fColor.toBytes_RGBA(), fAntiAlias);
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    TessellatingPathOp(Helper::MakeArgs helperArgs,
                       const SkPMColor4f& color,
                       const GrShape& shape,
                       const SkMatrix& viewMatrix,
                       const SkIRect& devClipBounds,
                       GrAAType aaType,
                       const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(helperArgs, aaType, stencilSettings)
            , fColor(color)
            , fShape(shape)
            , fViewMatrix(viewMatrix)
            , fDevClipBounds(devClipBounds)
            , fAntiAlias(GrAAType::kCoverage == aaType) {
        SkRect devBounds;
        viewMatrix.mapRect(&devBounds, shape.bounds());
        if (shape.inverseFilled()) {
            // Because the clip bounds are used to add a contour for inverse fills, they must also
            // include the path bounds.
            devBounds.join(SkRect::Make(fDevClipBounds));
        }
        this->setBounds(devBounds, HasAABloat::kNo, IsHairline::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(
            const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
            GrClampType clampType) override {
        GrProcessorAnalysisCoverage coverage = fAntiAlias
                                                       ? GrProcessorAnalysisCoverage::kSingleChannel
                                                       : GrProcessorAnalysisCoverage::kNone;
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(
                caps, clip, hasMixedSampledCoverage, clampType, coverage, &fColor, nullptr);
    }

private:
    SkPath getPath() const {
        SkASSERT(!fShape.style().applies());
        SkPath path;
        fShape.asPath(&path);
        return path;
    }

    void draw(Target* target, const GrGeometryProcessor* gp, size_t vertexStride) {
        SkASSERT(!fAntiAlias);
        GrResourceProvider* rp = target->resourceProvider();
        bool inverseFill = fShape.inverseFilled();
        // construct a cache key from the path's genID and the view matrix
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
        GrUniqueKey key;
        static constexpr int kClipBoundsCnt = sizeof(fDevClipBounds) / sizeof(uint32_t);
        int shapeKeyDataCnt = fShape.unstyledKeySize();
        SkASSERT(shapeKeyDataCnt >= 0);
        GrUniqueKey::Builder builder(&key, kDomain, shapeKeyDataCnt + kClipBoundsCnt, "Path");
        fShape.writeUnstyledKey(&builder[0]);
        // For inverse fills, the tessellation is dependent on clip bounds.
        if (inverseFill) {
            memcpy(&builder[shapeKeyDataCnt], &fDevClipBounds, sizeof(fDevClipBounds));
        } else {
            memset(&builder[shapeKeyDataCnt], 0, sizeof(fDevClipBounds));
        }
        builder.finish();
        sk_sp<GrGpuBuffer> cachedVertexBuffer(rp->findByUniqueKey<GrGpuBuffer>(key));
        int actualCount;
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        tol = GrPathUtils::scaleToleranceToSrc(tol, fViewMatrix, fShape.bounds());
        if (cache_match(cachedVertexBuffer.get(), tol, &actualCount)) {
            this->drawVertices(target, gp, std::move(cachedVertexBuffer), 0, actualCount);
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
        StaticVertexAllocator allocator(vertexStride, rp, canMapVB);
        int count = GrTessellator::PathToTriangles(getPath(), tol, clipBounds, &allocator, false,
                                                   &isLinear);
        if (count == 0) {
            return;
        }
        sk_sp<GrGpuBuffer> vb = allocator.detachVertexBuffer();
        TessInfo info;
        info.fTolerance = isLinear ? 0 : tol;
        info.fCount = count;
        fShape.addGenIDChangeListener(sk_make_sp<PathInvalidator>(key, target->contextUniqueID()));
        key.setCustomData(SkData::MakeWithCopy(&info, sizeof(info)));
        rp->assignUniqueKeyToResource(key, vb.get());

        this->drawVertices(target, gp, std::move(vb), 0, count);
    }

    void drawAA(Target* target, const GrGeometryProcessor* gp, size_t vertexStride) {
        SkASSERT(fAntiAlias);
        SkPath path = getPath();
        if (path.isEmpty()) {
            return;
        }
        SkRect clipBounds = SkRect::Make(fDevClipBounds);
        path.transform(fViewMatrix);
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        bool isLinear;
        DynamicVertexAllocator allocator(vertexStride, target);
        int count = GrTessellator::PathToTriangles(path, tol, clipBounds, &allocator, true,
                                                   &isLinear);
        if (count == 0) {
            return;
        }
        this->drawVertices(target, gp, allocator.detachVertexBuffer(),
                           allocator.firstVertex(), count);
    }

    void onPrepareDraws(Target* target) override {
        GrGeometryProcessor* gp;
        {
            using namespace GrDefaultGeoProcFactory;

            Color color(fColor);
            LocalCoords::Type localCoordsType = fHelper.usesLocalCoords()
                                                        ? LocalCoords::kUsePosition_Type
                                                        : LocalCoords::kUnused_Type;
            Coverage::Type coverageType;
            if (fAntiAlias) {
                if (fHelper.compatibleWithCoverageAsAlpha()) {
                    coverageType = Coverage::kAttributeTweakAlpha_Type;
                } else {
                    coverageType = Coverage::kAttribute_Type;
                }
            } else {
                coverageType = Coverage::kSolid_Type;
            }
            if (fAntiAlias) {
                gp = GrDefaultGeoProcFactory::MakeForDeviceSpace(target->allocator(),
                                                                 target->caps().shaderCaps(),
                                                                 color, coverageType,
                                                                 localCoordsType, fViewMatrix);
            } else {
                gp = GrDefaultGeoProcFactory::Make(target->allocator(), target->caps().shaderCaps(),
                                                   color, coverageType, localCoordsType,
                                                   fViewMatrix);
            }
        }
        if (!gp) {
            return;
        }
        size_t vertexStride = gp->vertexStride();
        if (fAntiAlias) {
            this->drawAA(target, gp, vertexStride);
        } else {
            this->draw(target, gp, vertexStride);
        }
    }

    void drawVertices(Target* target, const GrGeometryProcessor* gp, sk_sp<const GrBuffer> vb,
                      int firstVertex, int count) {
        GrPrimitiveType primitiveType = TESSELLATOR_WIREFRAME ? GrPrimitiveType::kLines
                                                              : GrPrimitiveType::kTriangles;

        GrMesh* mesh = target->allocMesh(primitiveType);
        mesh->setNonIndexedNonInstanced(count);
        mesh->setVertexData(std::move(vb), firstVertex);
        target->recordDraw(gp, mesh, 1, primitiveType);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                                 fHelper.detachProcessorSet(),
                                                                 fHelper.pipelineFlags(),
                                                                 fHelper.stencilSettings());

        flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
    }

    Helper                  fHelper;
    SkPMColor4f             fColor;
    GrShape                 fShape;
    SkMatrix                fViewMatrix;
    SkIRect                 fDevClipBounds;
    bool                    fAntiAlias;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrTessellatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrTessellatingPathRenderer::onDrawPath");
    SkIRect clipBoundsI;
    args.fClip->getConservativeBounds(args.fRenderTargetContext->width(),
                                      args.fRenderTargetContext->height(),
                                      &clipBoundsI);
    std::unique_ptr<GrDrawOp> op = TessellatingPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fShape, *args.fViewMatrix, clipBoundsI,
            args.fAAType, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(TesselatingPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPath path = GrTest::TestPath(random);
    SkIRect devClipBounds = SkIRect::MakeLTRB(
        random->nextU(), random->nextU(), random->nextU(), random->nextU());
    devClipBounds.sort();
    static constexpr GrAAType kAATypes[] = {GrAAType::kNone, GrAAType::kMSAA, GrAAType::kCoverage};
    GrAAType aaType;
    do {
        aaType = kAATypes[random->nextULessThan(SK_ARRAY_COUNT(kAATypes))];
    } while(GrAAType::kMSAA == aaType && numSamples <= 1);
    GrStyle style;
    do {
        GrTest::TestStyle(random, &style);
    } while (!style.isSimpleFill());
    GrShape shape(path, style);
    return TessellatingPathOp::Make(context, std::move(paint), shape, viewMatrix, devClipBounds,
                                    aaType, GrGetRandomStencil(random, context));
}

#endif
