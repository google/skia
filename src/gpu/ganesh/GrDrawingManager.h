/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawingManager_DEFINED
#define GrDrawingManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/ganesh/GrBufferAllocPool.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrHashMapWithCache.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/PathRenderer.h"
#include "src/gpu/ganesh/PathRendererChain.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

// Enabling this will print out which path renderers are being chosen
#define GR_PATH_RENDERER_SPEW 0

class GrArenas;
class GrDeferredDisplayList;
class GrDirectContext;
class GrGpuBuffer;
class GrOnFlushCallbackObject;
class GrOpFlushState;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrRenderTask;
class GrResourceAllocator;
class GrSemaphore;
class GrSurfaceProxyView;
class GrTextureResolveRenderTask;
class SkData;
enum GrSurfaceOrigin : int;
enum class GrColorType;
enum class GrSemaphoresSubmitted : bool;
struct GrFlushInfo;
struct GrMipLevel;
struct SkIRect;

namespace SkSurfaces {
enum class BackendSurfaceAccess;
}
namespace skgpu {
class MutableTextureState;
namespace ganesh {
class AtlasPathRenderer;
class OpsTask;
class SoftwarePathRenderer;
}  // namespace ganesh
}  // namespace skgpu

class GrDrawingManager {
public:
    ~GrDrawingManager();

    void freeGpuResources();

    // OpsTasks created at flush time are stored and handled different from the others.
    sk_sp<skgpu::ganesh::OpsTask> newOpsTask(GrSurfaceProxyView, sk_sp<GrArenas> arenas);

    // Adds 'atlasTask' to the DAG and leaves it open.
    //
    // If 'previousAtlasTask' is provided, closes it and configures dependencies to guarantee
    // previousAtlasTask and all its users are completely out of service before atlasTask executes.
    void addAtlasTask(sk_sp<GrRenderTask> atlasTask, GrRenderTask* previousAtlasTask);

    // Create a render task that can resolve MSAA and/or regenerate mipmap levels on proxies. This
    // method will only add the new render task to the list. However, it adds the task before the
    // last task in the list. It is up to the caller to call addProxy() on the returned object.
    GrTextureResolveRenderTask* newTextureResolveRenderTaskBefore(const GrCaps&);

    // Creates a render task that can resolve MSAA and/or regenerate mimap levels on the passed in
    // proxy. The task is appended to the end of the current list of tasks.
    void newTextureResolveRenderTask(sk_sp<GrSurfaceProxy> proxy,
                                     GrSurfaceProxy::ResolveFlags,
                                     const GrCaps&);

    // Create a new render task that will cause the gpu to wait on semaphores before executing any
    // more RenderTasks that target proxy. It is possible for this wait to also block additional
    // work (even to other proxies) that has already been recorded or will be recorded later. The
    // only guarantee is that future work to the passed in proxy will wait on the semaphores to be
    // signaled.
    void newWaitRenderTask(const sk_sp<GrSurfaceProxy>& proxy,
                           std::unique_ptr<std::unique_ptr<GrSemaphore>[]>,
                           int numSemaphores);

    // Create a new render task which copies the pixels from the srcProxy into the dstBuffer. This
    // is used to support the asynchronous readback API. The srcRect is the region of the srcProxy
    // to be copied. The surfaceColorType says how we should interpret the data when reading back
    // from the source. DstColorType describes how the data should be stored in the dstBuffer.
    // DstOffset is the offset into the dstBuffer where we will start writing data.
    void newTransferFromRenderTask(const sk_sp<GrSurfaceProxy>& srcProxy, const SkIRect& srcRect,
                                   GrColorType surfaceColorType, GrColorType dstColorType,
                                   sk_sp<GrGpuBuffer> dstBuffer, size_t dstOffset);

    // Creates a new render task which copies a pixel rectangle from srcView into dstView. The src
    // pixels copied are specified by srcRect. They are copied to the dstRect in dstProxy. Some
    // backends and formats may require dstRect to have the same size as srcRect. Regardless,
    // srcRect must be contained by src's dimensions and dstRect must be contained by dst's
    // dimensions. Any clipping, aspect-ratio adjustment, etc. must be handled prior to this call.
    //
    // This method is not guaranteed to succeed depending on the type of surface, formats, etc, and
    // the backend-specific limitations. On success the task is returned so that the caller may mark
    // it skippable if the copy is later deemed unnecessary.
    sk_sp<GrRenderTask> newCopyRenderTask(sk_sp<GrSurfaceProxy> dst,
                                          SkIRect dstRect,
                                          const sk_sp<GrSurfaceProxy>& src,
                                          SkIRect srcRect,
                                          GrSamplerState::Filter filter,
                                          GrSurfaceOrigin);

    // Adds a render task that copies the range [srcOffset, srcOffset + size] from src to
    // [dstOffset, dstOffset + size] in dst. The src buffer must have type kXferCpuToGpu and the
    // dst must NOT have type kXferCpuToGpu. Neither buffer may be mapped when this executes.
    // Because this is used to insert transfers to vertex/index buffers between draws and we don't
    // track dependencies with buffers, this task is a hard boundary for task reordering.
    void newBufferTransferTask(sk_sp<GrGpuBuffer> src,
                               size_t srcOffset,
                               sk_sp<GrGpuBuffer> dst,
                               size_t dstOffset,
                               size_t size);

    // Adds a render task that copies the src SkData to [dstOffset, dstOffset + src->size()] in dst.
    // The dst must not have type kXferCpuToGpu and must not be mapped. Because this is used to
    // insert updata to vertex/index buffers between draws and we don't track dependencies with
    // buffers, this task is a hard boundary for task reordering.
    void newBufferUpdateTask(sk_sp<SkData> src, sk_sp<GrGpuBuffer> dst, size_t dstOffset);

    // Adds a task that writes the data from the passed GrMipLevels to dst. The lifetime of the
    // pixel data in the levels should be tied to the passed SkData or the caller must flush the
    // context before the data may become invalid. srcColorType is the color type of the
    // GrMipLevels. dstColorType is the color type being used with dst and must be compatible with
    // dst's format according to GrCaps::areColorTypeAndFormatCompatible().
    bool newWritePixelsTask(sk_sp<GrSurfaceProxy> dst,
                            SkIRect rect,
                            GrColorType srcColorType,
                            GrColorType dstColorType,
                            const GrMipLevel[],
                            int levelCount);

    GrRecordingContext* getContext() { return fContext; }

    using PathRenderer = skgpu::ganesh::PathRenderer;
    using PathRendererChain = skgpu::ganesh::PathRendererChain;

    PathRenderer* getPathRenderer(const PathRenderer::CanDrawPathArgs&,
                                  bool allowSW,
                                  PathRendererChain::DrawType,
                                  PathRenderer::StencilSupport* = nullptr);

    PathRenderer* getSoftwarePathRenderer();

    // Returns a direct pointer to the atlas path renderer, or null if it is not supported and
    // turned on.
    skgpu::ganesh::AtlasPathRenderer* getAtlasPathRenderer();

    // Returns a direct pointer to the tessellation path renderer, or null if it is not supported
    // and turned on.
    PathRenderer* getTessellationPathRenderer();

    static bool ProgramUnitTest(GrDirectContext*, int maxStages, int maxLevels);

    GrSemaphoresSubmitted flushSurfaces(SkSpan<GrSurfaceProxy*>,
                                        SkSurfaces::BackendSurfaceAccess,
                                        const GrFlushInfo&,
                                        const skgpu::MutableTextureState* newState);

    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

#if defined(GPU_TEST_UTILS)
    void testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject*);
    PathRendererChain::Options testingOnly_getOptionsForPathRendererChain() {
        return fOptionsForPathRendererChain;
    }
#endif

    GrRenderTask* getLastRenderTask(const GrSurfaceProxy*) const;
    skgpu::ganesh::OpsTask* getLastOpsTask(const GrSurfaceProxy*) const;
    void setLastRenderTask(const GrSurfaceProxy*, GrRenderTask*);

    void moveRenderTasksToDDL(GrDeferredDisplayList* ddl);
    void createDDLTask(sk_sp<const GrDeferredDisplayList>,
                       sk_sp<GrRenderTargetProxy> newDest);

    // This is public so it can be called by an SkImage factory (in SkImages namespace).
    // It is not meant to be directly called in other situations.
    bool flush(SkSpan<GrSurfaceProxy*> proxies,
               SkSurfaces::BackendSurfaceAccess access,
               const GrFlushInfo&,
               const skgpu::MutableTextureState* newState);

private:
    GrDrawingManager(GrRecordingContext*,
                     const PathRendererChain::Options&,
                     bool reduceOpsTaskSplitting);

    bool wasAbandoned() const;

    void closeActiveOpsTask();

    // return true if any GrRenderTasks were actually executed; false otherwise
    bool executeRenderTasks(GrOpFlushState*);

    void removeRenderTasks();

    void sortTasks();

    // Attempt to reorder tasks to reduce render passes, and check the memory budget of the
    // resulting intervals. Returns whether the reordering was successful & the memory budget
    // acceptable. If it returns true, fDAG has been updated to reflect the reordered tasks.
    bool reorderTasks(GrResourceAllocator*);

    void closeAllTasks();

    GrRenderTask* appendTask(sk_sp<GrRenderTask>);
    GrRenderTask* insertTaskBeforeLast(sk_sp<GrRenderTask>);

    bool submitToGpu();

    SkDEBUGCODE(void validate() const;)

    friend class GrDirectContext; // access to: flush & cleanup
    friend class GrOnFlushResourceProvider; // this is just a shallow wrapper around this class
    friend class GrRecordingContext;  // access to: ctor

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrRecordingContext*                        fContext;

    // This cache is used by both the vertex and index pools. It reuses memory across multiple
    // flushes.
    sk_sp<GrBufferAllocPool::CpuBufferCache>   fCpuBufferCache;

    skia_private::TArray<sk_sp<GrRenderTask>>  fDAG;
    std::vector<int>                           fReorderBlockerTaskIndices;
    skgpu::ganesh::OpsTask*                    fActiveOpsTask = nullptr;

    PathRendererChain::Options                 fOptionsForPathRendererChain;
    std::unique_ptr<PathRendererChain>         fPathRendererChain;
    sk_sp<skgpu::ganesh::SoftwarePathRenderer> fSoftwarePathRenderer;

    skgpu::TokenTracker                        fTokenTracker;
    bool                                       fFlushing = false;
    const bool                                 fReduceOpsTaskSplitting;

    skia_private::TArray<GrOnFlushCallbackObject*> fOnFlushCBObjects;

    struct SurfaceIDKeyTraits {
        static uint32_t GetInvalidKey() {
            return GrSurfaceProxy::UniqueID::InvalidID().asUInt();
        }
    };

    GrHashMapWithCache<uint32_t, GrRenderTask*, SurfaceIDKeyTraits, GrCheapHash> fLastRenderTasks;
};

#endif
