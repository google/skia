/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrTriangulatingPathRenderer.h"

#include "include/private/SkIDChangeListener.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSimpleMesh.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.h"

#include <cstdio>

#ifndef GR_AA_TESSELLATOR_MAX_VERB_COUNT
#define GR_AA_TESSELLATOR_MAX_VERB_COUNT 10
#endif

/*
 * This path renderer linearizes and decomposes the path into triangles using GrTriangulator,
 * uploads the triangles to a vertex buffer, and renders them with a single draw call. It can do
 * screenspace antialiasing with a one-pixel coverage ramp.
 */
namespace {

struct TessInfo {
    SkScalar  fTolerance;
    int       fCount;
};

// When the SkPathRef genID changes, invalidate a corresponding GrResource described by key.
class UniqueKeyInvalidator : public SkIDChangeListener {
public:
    UniqueKeyInvalidator(const GrUniqueKey& key, uint32_t contextUniqueID)
            : fMsg(key, contextUniqueID) {}

private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void changed() override { SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(fMsg); }
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

class StaticVertexAllocator : public GrEagerVertexAllocator {
public:
    StaticVertexAllocator(GrResourceProvider* resourceProvider, bool canMapVB)
      : fResourceProvider(resourceProvider)
      , fCanMapVB(canMapVB)
      , fVertices(nullptr) {
    }
#ifdef SK_DEBUG
    ~StaticVertexAllocator() override {
        SkASSERT(!fLockStride);
    }
#endif
    void* lock(size_t stride, int eagerCount) override {
        SkASSERT(!fLockStride);
        SkASSERT(stride);
        size_t size = eagerCount * stride;
        fVertexBuffer = fResourceProvider->createBuffer(size, GrGpuBufferType::kVertex,
                                                        kStatic_GrAccessPattern);
        if (!fVertexBuffer.get()) {
            return nullptr;
        }
        if (fCanMapVB) {
            fVertices = fVertexBuffer->map();
        } else {
            fVertices = sk_malloc_throw(eagerCount * stride);
        }
        fLockStride = stride;
        return fVertices;
    }
    void unlock(int actualCount) override {
        SkASSERT(fLockStride);
        if (fCanMapVB) {
            fVertexBuffer->unmap();
        } else {
            fVertexBuffer->updateData(fVertices, actualCount * fLockStride);
            sk_free(fVertices);
        }
        fVertices = nullptr;
        fLockStride = 0;
    }
    sk_sp<GrGpuBuffer> detachVertexBuffer() { return std::move(fVertexBuffer); }

private:
    sk_sp<GrGpuBuffer> fVertexBuffer;
    GrResourceProvider* fResourceProvider;
    bool fCanMapVB;
    void* fVertices;
    size_t fLockStride = 0;
};

}  // namespace

GrTriangulatingPathRenderer::GrTriangulatingPathRenderer()
  : fMaxVerbCount(GR_AA_TESSELLATOR_MAX_VERB_COUNT) {
}

GrPathRenderer::CanDrawPath
GrTriangulatingPathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
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

class TriangulatingPathOp final : public GrMeshDrawOp {
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
        return Helper::FactoryHelper<TriangulatingPathOp>(context, std::move(paint), shape,
                                                          viewMatrix, devClipBounds, aaType,
                                                          stencilSettings);
    }

    const char* name() const override { return "TriangulatingPathOp"; }

    void visitProxies(const VisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
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

    TriangulatingPathOp(Helper::MakeArgs helperArgs,
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

    void draw(Target* target) {
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
            this->createMesh(target, std::move(cachedVertexBuffer), 0, actualCount);
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
        StaticVertexAllocator allocator(rp, canMapVB);
        int count = GrTriangulator::PathToTriangles(getPath(), tol, clipBounds, &allocator,
                                                    GrTriangulator::Mode::kNormal, &isLinear);
        if (count == 0) {
            return;
        }
        sk_sp<GrGpuBuffer> vb = allocator.detachVertexBuffer();
        TessInfo info;
        info.fTolerance = isLinear ? 0 : tol;
        info.fCount = count;
        fShape.addGenIDChangeListener(
                sk_make_sp<UniqueKeyInvalidator>(key, target->contextUniqueID()));
        key.setCustomData(SkData::MakeWithCopy(&info, sizeof(info)));
        rp->assignUniqueKeyToResource(key, vb.get());

        this->createMesh(target, std::move(vb), 0, count);
    }

    void drawAA(Target* target) {
        SkASSERT(fAntiAlias);
        SkPath path = getPath();
        if (path.isEmpty()) {
            return;
        }
        SkRect clipBounds = SkRect::Make(fDevClipBounds);
        path.transform(fViewMatrix);
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;
        bool isLinear;
        GrEagerDynamicVertexAllocator allocator(target, &vertexBuffer, &firstVertex);
        int count = GrTriangulator::PathToTriangles(path, tol, clipBounds, &allocator,
                                                    GrTriangulator::Mode::kEdgeAntialias, &isLinear);
        if (count == 0) {
            return;
        }
        this->createMesh(target, std::move(vertexBuffer), firstVertex, count);
    }

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView* outputView,
                             GrAppliedClip&& appliedClip,
                             const GrXferProcessor::DstProxyView& dstProxyView) override {
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
                gp = GrDefaultGeoProcFactory::MakeForDeviceSpace(arena, color, coverageType,
                                                                 localCoordsType, fViewMatrix);
            } else {
                gp = GrDefaultGeoProcFactory::Make(arena, color, coverageType, localCoordsType,
                                                   fViewMatrix);
            }
        }
        if (!gp) {
            return;
        }

#ifdef SK_DEBUG
        auto mode = (fAntiAlias) ? GrTriangulator::Mode::kEdgeAntialias
                                 : GrTriangulator::Mode::kNormal;
        SkASSERT(GrTriangulator::GetVertexStride(mode) == gp->vertexStride());
#endif

        GrPrimitiveType primitiveType = TRIANGULATOR_WIREFRAME ? GrPrimitiveType::kLines
                                                               : GrPrimitiveType::kTriangles;

        fProgramInfo =  fHelper.createProgramInfoWithStencil(caps, arena, outputView,
                                                             std::move(appliedClip), dstProxyView,
                                                             gp, primitiveType);
    }

    void onPrepareDraws(Target* target) override {
        if (fAntiAlias) {
            this->drawAA(target);
        } else {
            this->draw(target);
        }
    }

    void createMesh(Target* target, sk_sp<const GrBuffer> vb, int firstVertex, int count) {
        fMesh = target->allocMesh();
        fMesh->set(std::move(vb), count, firstVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo) {
            this->createProgramInfo(flushState);
        }

        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    Helper         fHelper;
    SkPMColor4f    fColor;
    GrShape        fShape;
    SkMatrix       fViewMatrix;
    SkIRect        fDevClipBounds;
    bool           fAntiAlias;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

bool GrTriangulatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrTriangulatingPathRenderer::onDrawPath");
    SkIRect clipBoundsI;
    args.fClip->getConservativeBounds(args.fRenderTargetContext->width(),
                                      args.fRenderTargetContext->height(),
                                      &clipBoundsI);
    std::unique_ptr<GrDrawOp> op = TriangulatingPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fShape, *args.fViewMatrix, clipBoundsI,
            args.fAAType, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(TriangulatingPathOp) {
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
    return TriangulatingPathOp::Make(context, std::move(paint), shape, viewMatrix, devClipBounds,
                                     aaType, GrGetRandomStencil(random, context));
}

#endif
