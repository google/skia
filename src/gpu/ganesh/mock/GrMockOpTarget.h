/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOpTarget_DEFINED
#define GrMockOpTarget_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawIndirectCommand.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrXferProcessor.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrAtlasManager;
class GrCaps;
class GrDeferredUploadTarget;
class GrGeometryProcessor;
class GrRenderTargetProxy;
class GrResourceProvider;
class GrSurfaceProxy;
class GrSurfaceProxyView;
class GrThreadSafeCache;

namespace skgpu { namespace ganesh { class SmallPathAtlasMgr; } }
namespace sktext { namespace gpu { class StrikeCache; } }


// This is a mock GrMeshDrawTarget implementation that just gives back pointers into
// pre-allocated CPU buffers, rather than allocating and mapping GPU buffers.
class GrMockOpTarget : public GrMeshDrawTarget {
public:
    GrMockOpTarget(sk_sp<GrDirectContext> mockContext) : fMockContext(std::move(mockContext)) {
        fStaticVertexBuffer = fMockContext->priv().getGpu()->createBuffer(
                sizeof(fStaticVertexData), GrGpuBufferType::kVertex, kDynamic_GrAccessPattern);
        fStaticIndirectBuffer = fMockContext->priv().getGpu()->createBuffer(
                sizeof(fStaticIndirectData), GrGpuBufferType::kDrawIndirect,
                kDynamic_GrAccessPattern);
    }
    const GrDirectContext* mockContext() const { return fMockContext.get(); }
    const GrCaps& caps() const override { return *fMockContext->priv().caps(); }
    GrThreadSafeCache* threadSafeCache() const override {
        return fMockContext->priv().threadSafeCache();
    }
    GrResourceProvider* resourceProvider() const override {
        return fMockContext->priv().resourceProvider();
    }
#ifndef SK_ENABLE_OPTIMIZE_SIZE
    skgpu::ganesh::SmallPathAtlasMgr* smallPathAtlasManager() const override { return nullptr; }
#endif
    void resetAllocator() { fAllocator.reset(); }
    SkArenaAlloc* allocator() override { return &fAllocator; }
    void putBackVertices(int vertices, size_t vertexStride) override { /* no-op */ }
    GrAppliedClip detachAppliedClip() override { return GrAppliedClip::Disabled(); }
    const GrDstProxyView& dstProxyView() const override { return fDstProxyView; }
    GrXferBarrierFlags renderPassBarriers() const override { return GrXferBarrierFlags::kNone; }
    GrLoadOp colorLoadOp() const override { return GrLoadOp::kLoad; }

    void* makeVertexSpace(size_t vertexSize, int vertexCount, sk_sp<const GrBuffer>* buffer,
                          int* startVertex) override {
        if (vertexSize * vertexCount > sizeof(fStaticVertexData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static vertex data; only have %zu.\n",
                     vertexSize * vertexCount, sizeof(fStaticVertexData));
        }
        *buffer = fStaticVertexBuffer;
        *startVertex = 0;
        return fStaticVertexData;
    }

    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 sk_sp<const GrBuffer>* buffer, int* startVertex,
                                 int* actualVertexCount) override {
        if (vertexSize * minVertexCount > sizeof(fStaticVertexData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static vertex data; only have %zu.\n",
                     vertexSize * minVertexCount, sizeof(fStaticVertexData));
        }
        *buffer = fStaticVertexBuffer;
        *startVertex = 0;
        *actualVertexCount = sizeof(fStaticVertexData) / vertexSize;
        return fStaticVertexData;
    }

    GrDrawIndirectWriter makeDrawIndirectSpace(int drawCount, sk_sp<const GrBuffer>* buffer,
                                               size_t* offsetInBytes) override {
        if (sizeof(GrDrawIndirectCommand) * drawCount > sizeof(fStaticIndirectData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static indirect data; only have %zu.\n",
                     sizeof(GrDrawIndirectCommand) * drawCount, sizeof(fStaticIndirectData));
        }
        *buffer = fStaticIndirectBuffer;
        *offsetInBytes = 0;
        return fStaticIndirectData;
    }

    void putBackIndirectDraws(int count) override { /* no-op */ }

    GrDrawIndexedIndirectWriter makeDrawIndexedIndirectSpace(int drawCount,
                                                             sk_sp<const GrBuffer>* buffer,
                                                             size_t* offsetInBytes) override {
        if (sizeof(GrDrawIndexedIndirectCommand) * drawCount > sizeof(fStaticIndirectData)) {
            SK_ABORT("FATAL: wanted %zu bytes of static indirect data; only have %zu.\n",
                     sizeof(GrDrawIndexedIndirectCommand) * drawCount, sizeof(fStaticIndirectData));
        }
        *buffer = fStaticIndirectBuffer;
        *offsetInBytes = 0;
        return fStaticIndirectData;
    }

    void putBackIndexedIndirectDraws(int count) override { /* no-op */ }

    // Call these methods to see what got written after the previous call to make*Space.
    const void* peekStaticVertexData() const { return fStaticVertexData; }
    const void* peekStaticIndirectData() const { return fStaticIndirectData; }

#define UNIMPL(...) __VA_ARGS__ override { SK_ABORT("unimplemented."); }
    UNIMPL(void recordDraw(const GrGeometryProcessor*, const GrSimpleMesh[], int,
                           const GrSurfaceProxy* const[], GrPrimitiveType))
    UNIMPL(uint16_t* makeIndexSpace(int, sk_sp<const GrBuffer>*, int*))
    UNIMPL(uint16_t* makeIndexSpaceAtLeast(int, int, sk_sp<const GrBuffer>*, int*, int*))
    UNIMPL(void putBackIndices(int))
    UNIMPL(GrRenderTargetProxy* rtProxy() const)
    UNIMPL(const GrSurfaceProxyView& writeView() const)
    UNIMPL(const GrAppliedClip* appliedClip() const)
    UNIMPL(bool usesMSAASurface() const)
    UNIMPL(sktext::gpu::StrikeCache* strikeCache() const)
    UNIMPL(GrAtlasManager* atlasManager() const)
    UNIMPL(skia_private::TArray<GrSurfaceProxy*, true>* sampledProxyArray())
    UNIMPL(GrDeferredUploadTarget* deferredUploadTarget())
#undef UNIMPL

private:
    sk_sp<GrDirectContext> fMockContext;
    char fStaticVertexData[6 * 1024 * 1024];
    sk_sp<GrGpuBuffer> fStaticVertexBuffer;
    char fStaticIndirectData[sizeof(GrDrawIndexedIndirectCommand) * 32];
    sk_sp<GrGpuBuffer> fStaticIndirectBuffer;
    SkSTArenaAllocWithReset<1024 * 1024> fAllocator;
    GrDstProxyView fDstProxyView;
};

#endif
