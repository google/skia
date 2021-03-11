/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrTriangulatingPathRenderer.h"

#include "include/private/SkIDChangeListener.h"
#include "src/core/SkGeometry.h"
#include "src/gpu/GrAATriangulator.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDefaultGeoProcFactory.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSimpleMesh.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/GrTriangulator.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/geometry/GrStyledShape.h"
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

// The TessInfo struct contains ancillary data not specifically required for the triangle
// data (which is stored in a GrThreadSafeCache::VertexData object).
// The 'fNumVertices' field is a temporary exception. It is still needed to support the
// AA triangulated path case - which doesn't use the GrThreadSafeCache nor the VertexData object).
// When there is an associated VertexData, its numVertices should always match the TessInfo's
// value.
struct TessInfo {
    int       fNumVertices;
    bool      fIsLinear;
    SkScalar  fTolerance;
};

static sk_sp<SkData> create_data(int numVertices, bool isLinear, SkScalar tol) {
    TessInfo info { numVertices, isLinear, tol };
    return SkData::MakeWithCopy(&info, sizeof(info));
}

bool cache_match(const SkData* data, SkScalar tol) {
    SkASSERT(data);

    const TessInfo* info = static_cast<const TessInfo*>(data->data());

    return info->fIsLinear || info->fTolerance < 3.0f * tol;
}

// Should 'challenger' replace 'incumbent' in the cache if there is a collision?
bool is_newer_better(SkData* incumbent, SkData* challenger) {
    const TessInfo* i = static_cast<const TessInfo*>(incumbent->data());
    const TessInfo* c = static_cast<const TessInfo*>(challenger->data());

    if (i->fIsLinear || i->fTolerance <= c->fTolerance) {
        return false;  // prefer the incumbent
    }

    return true;
}

// When the SkPathRef genID changes, invalidate a corresponding GrResource described by key.
class UniqueKeyInvalidator : public SkIDChangeListener {
public:
    UniqueKeyInvalidator(const GrUniqueKey& key, uint32_t contextUniqueID)
            : fMsg(key, contextUniqueID, /* inThreadSafeCache */ true) {}

private:
    GrUniqueKeyInvalidatedMessage fMsg;

    void changed() override { SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(fMsg); }
};

class StaticVertexAllocator : public GrEagerVertexAllocator {
public:
    StaticVertexAllocator(GrResourceProvider* resourceProvider, bool canMapVB)
            : fResourceProvider(resourceProvider)
            , fCanMapVB(canMapVB) {
    }

#ifdef SK_DEBUG
    ~StaticVertexAllocator() override {
        SkASSERT(!fLockStride && !fVertices && !fVertexBuffer && !fVertexData);
    }
#endif

    void* lock(size_t stride, int eagerCount) override {
        SkASSERT(!fLockStride && !fVertices && !fVertexBuffer && !fVertexData);
        SkASSERT(stride && eagerCount);

        size_t size = eagerCount * stride;
        fVertexBuffer = fResourceProvider->createBuffer(size, GrGpuBufferType::kVertex,
                                                        kStatic_GrAccessPattern);
        if (!fVertexBuffer) {
            return nullptr;
        }
        if (fCanMapVB) {
            fVertices = fVertexBuffer->map();
        }
        if (!fVertices) {
            fVertices = sk_malloc_throw(eagerCount * stride);
            fCanMapVB = false;
        }
        fLockStride = stride;
        return fVertices;
    }

    void unlock(int actualCount) override {
        SkASSERT(fLockStride && fVertices && fVertexBuffer && !fVertexData);

        if (fCanMapVB) {
            fVertexBuffer->unmap();
        } else {
            fVertexBuffer->updateData(fVertices, actualCount * fLockStride);
            sk_free(fVertices);
        }

        fVertexData = GrThreadSafeCache::MakeVertexData(std::move(fVertexBuffer),
                                                        actualCount, fLockStride);

        fVertices = nullptr;
        fLockStride = 0;
    }

    sk_sp<GrThreadSafeCache::VertexData> detachVertexData() {
        SkASSERT(!fLockStride && !fVertices && !fVertexBuffer && fVertexData);

        return std::move(fVertexData);
    }

private:
    sk_sp<GrThreadSafeCache::VertexData> fVertexData;
    sk_sp<GrGpuBuffer> fVertexBuffer;
    GrResourceProvider* fResourceProvider;
    bool fCanMapVB;
    void* fVertices = nullptr;
    size_t fLockStride = 0;
};

class CpuVertexAllocator : public GrEagerVertexAllocator {
public:
    CpuVertexAllocator() = default;

#ifdef SK_DEBUG
    ~CpuVertexAllocator() override {
        SkASSERT(!fLockStride && !fVertices && !fVertexData);
    }
#endif

    void* lock(size_t stride, int eagerCount) override {
        SkASSERT(!fLockStride && !fVertices && !fVertexData);
        SkASSERT(stride && eagerCount);

        fVertices = sk_malloc_throw(eagerCount * stride);
        fLockStride = stride;

        return fVertices;
    }

    void unlock(int actualCount) override {
        SkASSERT(fLockStride && fVertices && !fVertexData);

        fVertices = sk_realloc_throw(fVertices, actualCount * fLockStride);

        fVertexData = GrThreadSafeCache::MakeVertexData(fVertices, actualCount, fLockStride);

        fVertices = nullptr;
        fLockStride = 0;
    }

    sk_sp<GrThreadSafeCache::VertexData> detachVertexData() {
        SkASSERT(!fLockStride && !fVertices && fVertexData);

        return std::move(fVertexData);
    }

private:
    sk_sp<GrThreadSafeCache::VertexData> fVertexData;

    void*  fVertices = nullptr;
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

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const GrStyledShape& shape,
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

    TriangulatingPathOp(GrProcessorSet* processorSet,
                        const SkPMColor4f& color,
                        const GrStyledShape& shape,
                        const SkMatrix& viewMatrix,
                        const SkIRect& devClipBounds,
                        GrAAType aaType,
                        const GrUserStencilSettings* stencilSettings)
            : INHERITED(ClassID())
            , fHelper(processorSet, aaType, stencilSettings)
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
        this->setBounds(devBounds, HasAABloat(fAntiAlias), IsHairline::kNo);
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

    static void CreateKey(GrUniqueKey* key,
                          const GrStyledShape& shape,
                          const SkIRect& devClipBounds) {
        static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

        bool inverseFill = shape.inverseFilled();

        static constexpr int kClipBoundsCnt = sizeof(devClipBounds) / sizeof(uint32_t);
        int shapeKeyDataCnt = shape.unstyledKeySize();
        SkASSERT(shapeKeyDataCnt >= 0);
        GrUniqueKey::Builder builder(key, kDomain, shapeKeyDataCnt + kClipBoundsCnt, "Path");
        shape.writeUnstyledKey(&builder[0]);
        // For inverse fills, the tessellation is dependent on clip bounds.
        if (inverseFill) {
            memcpy(&builder[shapeKeyDataCnt], &devClipBounds, sizeof(devClipBounds));
        } else {
            memset(&builder[shapeKeyDataCnt], 0, sizeof(devClipBounds));
        }

        builder.finish();
    }

    // Triangulate the provided 'shape' in the shape's coordinate space. 'tol' should already
    // have been mapped back from device space.
    static int Triangulate(GrEagerVertexAllocator* allocator,
                           const SkMatrix& viewMatrix,
                           const GrStyledShape& shape,
                           const SkIRect& devClipBounds,
                           SkScalar tol,
                           bool* isLinear) {
        SkRect clipBounds = SkRect::Make(devClipBounds);

        SkMatrix vmi;
        if (!viewMatrix.invert(&vmi)) {
            return 0;
        }
        vmi.mapRect(&clipBounds);

        SkASSERT(!shape.style().applies());
        SkPath path;
        shape.asPath(&path);

        return GrTriangulator::PathToTriangles(path, tol, clipBounds, allocator, isLinear);
    }

    void createNonAAMesh(Target* target) {
        SkASSERT(!fAntiAlias);
        GrResourceProvider* rp = target->resourceProvider();
        auto threadSafeCache = target->threadSafeCache();

        GrUniqueKey key;
        CreateKey(&key, fShape, fDevClipBounds);

        SkScalar tol = GrPathUtils::scaleToleranceToSrc(GrPathUtils::kDefaultTolerance,
                                                        fViewMatrix, fShape.bounds());

        if (!fVertexData) {
            auto [cachedVerts, data] = threadSafeCache->findVertsWithData(key);
            if (cachedVerts && cache_match(data.get(), tol)) {
                fVertexData = std::move(cachedVerts);
            }
        }

        if (fVertexData) {
            if (!fVertexData->gpuBuffer()) {
                sk_sp<GrGpuBuffer> buffer = rp->createBuffer(fVertexData->size(),
                                                             GrGpuBufferType::kVertex,
                                                             kStatic_GrAccessPattern,
                                                             fVertexData->vertices());
                if (!buffer) {
                    return;
                }

                // Since we have a direct context and a ref on 'fVertexData' we need not worry
                // about any threading issues in this call.
                fVertexData->setGpuBuffer(std::move(buffer));
            }

            fMesh = CreateMesh(target, fVertexData->refGpuBuffer(), 0, fVertexData->numVertices());
            return;
        }

        bool canMapVB = GrCaps::kNone_MapFlags != target->caps().mapBufferFlags();
        StaticVertexAllocator allocator(rp, canMapVB);

        bool isLinear;
        int vertexCount = Triangulate(&allocator, fViewMatrix, fShape, fDevClipBounds, tol,
                                      &isLinear);
        if (vertexCount == 0) {
            return;
        }

        fVertexData = allocator.detachVertexData();

        key.setCustomData(create_data(vertexCount, isLinear, tol));

        auto [tmpV, tmpD] = threadSafeCache->addVertsWithData(key, fVertexData, is_newer_better);
        if (tmpV != fVertexData) {
            SkASSERT(!tmpV->gpuBuffer());
            // In this case, although the different triangulation found in the cache is better,
            // we will continue on with the current triangulation since it is already on the gpu.
        } else {
            // This isn't perfect. The current triangulation is in the cache but it may have
            // replaced a pre-existing one. A duplicated listener is unlikely and not that
            // expensive so we just roll with it.
            fShape.addGenIDChangeListener(
                sk_make_sp<UniqueKeyInvalidator>(key, target->contextUniqueID()));
        }

        fMesh = CreateMesh(target, fVertexData->refGpuBuffer(), 0, fVertexData->numVertices());
    }

    void createAAMesh(Target* target) {
        SkASSERT(!fVertexData);
        SkASSERT(fAntiAlias);
        SkPath path = this->getPath();
        if (path.isEmpty()) {
            return;
        }
        SkRect clipBounds = SkRect::Make(fDevClipBounds);
        path.transform(fViewMatrix);
        SkScalar tol = GrPathUtils::kDefaultTolerance;
        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;
        GrEagerDynamicVertexAllocator allocator(target, &vertexBuffer, &firstVertex);
        int vertexCount = GrAATriangulator::PathToAATriangles(path, tol, clipBounds, &allocator);
        if (vertexCount == 0) {
            return;
        }
        fMesh = CreateMesh(target, std::move(vertexBuffer), firstVertex, vertexCount);
    }

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             GrAppliedClip&& appliedClip,
                             const GrXferProcessor::DstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
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
        auto vertexStride = sizeof(SkPoint);
        if (fAntiAlias) {
            vertexStride += sizeof(float);
        }
        SkASSERT(vertexStride == gp->vertexStride());
#endif

        GrPrimitiveType primitiveType = TRIANGULATOR_WIREFRAME ? GrPrimitiveType::kLines
                                                               : GrPrimitiveType::kTriangles;

        fProgramInfo =  fHelper.createProgramInfoWithStencil(caps, arena, writeView,
                                                             std::move(appliedClip), dstProxyView,
                                                             gp, primitiveType,
                                                             renderPassXferBarriers, colorLoadOp);
    }

    void onPrePrepareDraws(GrRecordingContext* rContext,
                           const GrSurfaceProxyView& writeView,
                           GrAppliedClip* clip,
                           const GrXferProcessor::DstProxyView& dstProxyView,
                           GrXferBarrierFlags renderPassXferBarriers,
                           GrLoadOp colorLoadOp) override {
        TRACE_EVENT0("skia.gpu", TRACE_FUNC);

        INHERITED::onPrePrepareDraws(rContext, writeView, clip, dstProxyView,
                                     renderPassXferBarriers, colorLoadOp);

        if (fAntiAlias) {
            // TODO: pull the triangulation work forward to the recording thread for the AA case
            // too.
            return;
        }

        auto threadSafeViewCache = rContext->priv().threadSafeCache();

        GrUniqueKey key;
        CreateKey(&key, fShape, fDevClipBounds);

        SkScalar tol = GrPathUtils::scaleToleranceToSrc(GrPathUtils::kDefaultTolerance,
                                                        fViewMatrix, fShape.bounds());

        auto [cachedVerts, data] = threadSafeViewCache->findVertsWithData(key);
        if (cachedVerts && cache_match(data.get(), tol)) {
            fVertexData = std::move(cachedVerts);
            return;
        }

        CpuVertexAllocator allocator;

        bool isLinear;
        int vertexCount = Triangulate(&allocator, fViewMatrix, fShape, fDevClipBounds, tol,
                                      &isLinear);
        if (vertexCount == 0) {
            return;
        }

        fVertexData = allocator.detachVertexData();

        key.setCustomData(create_data(vertexCount, isLinear, tol));

        // If some other thread created and cached its own triangulation, the 'is_newer_better'
        // predicate will replace the version in the cache if 'fVertexData' is a more accurate
        // triangulation. This will leave some other recording threads using a poorer triangulation
        // but will result in a version with greater applicability being in the cache.
        auto [tmpV, tmpD] = threadSafeViewCache->addVertsWithData(key, fVertexData,
                                                                  is_newer_better);
        if (tmpV != fVertexData) {
            // Someone beat us to creating the triangulation (and it is better than ours) so
            // just go ahead and use it.
            SkASSERT(cache_match(tmpD.get(), tol));
            fVertexData = std::move(tmpV);
        } else {
            // This isn't perfect. The current triangulation is in the cache but it may have
            // replaced a pre-existing one. A duplicated listener is unlikely and not that
            // expensive so we just roll with it.
            fShape.addGenIDChangeListener(
                    sk_make_sp<UniqueKeyInvalidator>(key, rContext->priv().contextID()));
        }
    }

    void onPrepareDraws(Target* target) override {
        if (fAntiAlias) {
            this->createAAMesh(target);
        } else {
            this->createNonAAMesh(target);
        }
    }

    static GrSimpleMesh* CreateMesh(Target* target, sk_sp<const GrBuffer> vb,
                                    int firstVertex, int count) {
        auto mesh = target->allocMesh();
        mesh->set(std::move(vb), count, firstVertex);
        return mesh;
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

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        return SkStringPrintf("Color 0x%08x, aa: %d\n%s",
                              fColor.toBytes_RGBA(), fAntiAlias, fHelper.dumpInfo().c_str());
    }
#endif

    Helper         fHelper;
    SkPMColor4f    fColor;
    GrStyledShape  fShape;
    SkMatrix       fViewMatrix;
    SkIRect        fDevClipBounds;
    bool           fAntiAlias;

    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    sk_sp<GrThreadSafeCache::VertexData> fVertexData;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

bool GrTriangulatingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrTriangulatingPathRenderer::onDrawPath");

    GrOp::Owner op = TriangulatingPathOp::Make(
            args.fContext, std::move(args.fPaint), *args.fShape, *args.fViewMatrix,
            *args.fClipConservativeBounds, args.fAAType, args.fUserStencilSettings);
    args.fRenderTargetContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(TriangulatingPathOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    const SkPath& path = GrTest::TestPath(random);
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
    GrStyledShape shape(path, style);
    return TriangulatingPathOp::Make(context, std::move(paint), shape, viewMatrix, devClipBounds,
                                     aaType, GrGetRandomStencil(random, context));
}

#endif
