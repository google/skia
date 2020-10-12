/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContext_DEFINED
#define GrDirectContext_DEFINED

#include "include/private/GrContext.h"

class GrAtlasManager;
class GrSmallPathAtlasMgr;

class SK_API GrDirectContext : public GrContext {
public:
#ifdef SK_GL
    /**
     * Creates a GrDirectContext for a backend context. If no GrGLInterface is provided then the
     * result of GrGLMakeNativeInterface() is used if it succeeds.
     */
    static sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeGL(sk_sp<const GrGLInterface>);
    static sk_sp<GrDirectContext> MakeGL(const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeGL();
#endif

#ifdef SK_VULKAN
    /**
     * The Vulkan context (VkQueue, VkDevice, VkInstance) must be kept alive until the returned
     * GrDirectContext is destroyed. This also means that any objects created with this
     * GrDirectContext (e.g. SkSurfaces, SkImages, etc.) must also be released as they may hold
     * refs on the GrDirectContext. Once all these objects and the GrDirectContext are released,
     * then it is safe to delete the vulkan objects.
     */
    static sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext&, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext&);
#endif

#ifdef SK_METAL
    /**
     * Makes a GrDirectContext which uses Metal as the backend. The device parameter is an
     * MTLDevice and queue is an MTLCommandQueue which should be used by the backend. These objects
     * must have a ref on them which can be transferred to Ganesh which will release the ref
     * when the GrDirectContext is destroyed.
     */
    static sk_sp<GrDirectContext> MakeMetal(void* device, void* queue, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeMetal(void* device, void* queue);
#endif

#ifdef SK_DIRECT3D
    /**
     * Makes a GrDirectContext which uses Direct3D as the backend. The Direct3D context
     * must be kept alive until the returned GrDirectContext is first destroyed or abandoned.
     */
    static sk_sp<GrDirectContext> MakeDirect3D(const GrD3DBackendContext&, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeDirect3D(const GrD3DBackendContext&);
#endif

#ifdef SK_DAWN
    static sk_sp<GrDirectContext> MakeDawn(const wgpu::Device&,
                                           const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeDawn(const wgpu::Device&);
#endif

    static sk_sp<GrDirectContext> MakeMock(const GrMockOptions*, const GrContextOptions&);
    static sk_sp<GrDirectContext> MakeMock(const GrMockOptions*);

    ~GrDirectContext() override;

    /**
     * The context normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     * The flag bits, state, is dependent on which backend is used by the
     * context, either GL or D3D (possible in future).
     */
    void resetContext(uint32_t state = kAll_GrBackendState);

    /**
     * If the backend is GrBackendApi::kOpenGL, then all texture unit/target combinations for which
     * the context has modified the bound texture will have texture id 0 bound. This does not
     * flush the context. Calling resetContext() does not change the set that will be bound
     * to texture id 0 on the next call to resetGLTextureBindings(). After this is called
     * all unit/target combinations are considered to have unmodified bindings until the context
     * subsequently modifies them (meaning if this is called twice in a row with no intervening
     * context usage then the second call is a no-op.)
     */
    void resetGLTextureBindings();

    /**
     * Abandons all GPU resources and assumes the underlying backend 3D API context is no longer
     * usable. Call this if you have lost the associated GPU context, and thus internal texture,
     * buffer, etc. references/IDs are now invalid. Calling this ensures that the destructors of the
     * GrContext and any of its created resource objects will not make backend 3D API calls. Content
     * rendered but not previously flushed may be lost. After this function is called all subsequent
     * calls on the GrContext will fail or be no-ops.
     *
     * The typical use case for this function is that the underlying 3D context was lost and further
     * API calls may crash.
     *
     * For Vulkan, even if the device becomes lost, the VkQueue, VkDevice, or VkInstance used to
     * create the context must be kept alive even after abandoning the context. Those objects must
     * live for the lifetime of the context object itself. The reason for this is so that
     * we can continue to delete any outstanding GrBackendTextures/RenderTargets which must be
     * cleaned up even in a device lost state.
     */
    void abandonContext() override;

    /**
     * Returns true if the context was abandoned or if the if the backend specific context has
     * gotten into an unrecoverarble, lost state (e.g. in Vulkan backend if we've gotten a
     * VK_ERROR_DEVICE_LOST). If the backend context is lost, this call will also abandon the
     * GrContext.
     */
    bool abandoned() override;

    // TODO: Remove this from public after migrating Chrome.
    sk_sp<GrContextThreadSafeProxy> threadSafeProxy();

    /**
     * Checks if the underlying 3D API reported an out-of-memory error. If this returns true it is
     * reset and will return false until another out-of-memory error is reported by the 3D API. If
     * the context is abandoned then this will report false.
     *
     * Currently this is implemented for:
     *
     * OpenGL [ES] - Note that client calls to glGetError() may swallow GL_OUT_OF_MEMORY errors and
     * therefore hide the error from Skia. Also, it is not advised to use this in combination with
     * enabling GrContextOptions::fSkipGLErrorChecks. That option may prevent the context from ever
     * checking the GL context for OOM.
     *
     * Vulkan - Reports true if VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY has
     * occurred.
     */
    bool oomed();

    /**
     * This is similar to abandonContext() however the underlying 3D context is not yet lost and
     * the context will cleanup all allocated resources before returning. After returning it will
     * assume that the underlying context may no longer be valid.
     *
     * The typical use case for this function is that the client is going to destroy the 3D context
     * but can't guarantee that context will be destroyed first (perhaps because it may be ref'ed
     * elsewhere by either the client or Skia objects).
     *
     * For Vulkan, even if the device becomes lost, the VkQueue, VkDevice, or VkInstance used to
     * create the context must be alive before calling releaseResourcesAndAbandonContext.
     */
    void releaseResourcesAndAbandonContext();

    ///////////////////////////////////////////////////////////////////////////
    // Resource Cache

    /** DEPRECATED
     *  Return the current GPU resource cache limits.
     *
     *  @param maxResources If non-null, will be set to -1.
     *  @param maxResourceBytes If non-null, returns maximum number of bytes of
     *                          video memory that can be held in the cache.
     */
    void getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const;

    /**
     *  Return the current GPU resource cache limit in bytes.
     */
    size_t getResourceCacheLimit() const;

    /**
     *  Gets the current GPU resource cache usage.
     *
     *  @param resourceCount If non-null, returns the number of resources that are held in the
     *                       cache.
     *  @param maxResourceBytes If non-null, returns the total number of bytes of video memory held
     *                          in the cache.
     */
    void getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const;

    /**
     *  Gets the number of bytes in the cache consumed by purgeable (e.g. unlocked) resources.
     */
    size_t getResourceCachePurgeableBytes() const;

    /** DEPRECATED
     *  Specify the GPU resource cache limits. If the current cache exceeds the maxResourceBytes
     *  limit, it will be purged (LRU) to keep the cache within the limit.
     *
     *  @param maxResources Unused.
     *  @param maxResourceBytes The maximum number of bytes of video memory
     *                          that can be held in the cache.
     */
    void setResourceCacheLimits(int maxResources, size_t maxResourceBytes);

    /**
     *  Specify the GPU resource cache limit. If the cache currently exceeds this limit,
     *  it will be purged (LRU) to keep the cache within the limit.
     *
     *  @param maxResourceBytes The maximum number of bytes of video memory
     *                          that can be held in the cache.
     */
    void setResourceCacheLimit(size_t maxResourceBytes);

    /**
     * Frees GPU created by the context. Can be called to reduce GPU memory
     * pressure.
     */
    void freeGpuResources();

    /**
     * Purge GPU resources that haven't been used in the past 'msNotUsed' milliseconds or are
     * otherwise marked for deletion, regardless of whether the context is under budget.
     */
    void performDeferredCleanup(std::chrono::milliseconds msNotUsed);

    // Temporary compatibility API for Android.
    void purgeResourcesNotUsedInMs(std::chrono::milliseconds msNotUsed) {
        this->performDeferredCleanup(msNotUsed);
    }

    /**
     * Purge unlocked resources from the cache until the the provided byte count has been reached
     * or we have purged all unlocked resources. The default policy is to purge in LRU order, but
     * can be overridden to prefer purging scratch resources (in LRU order) prior to purging other
     * resource types.
     *
     * @param maxBytesToPurge the desired number of bytes to be purged.
     * @param preferScratchResources If true scratch resources will be purged prior to other
     *                               resource types.
     */
    void purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources);

    /**
     * This entry point is intended for instances where an app has been backgrounded or
     * suspended.
     * If 'scratchResourcesOnly' is true all unlocked scratch resources will be purged but the
     * unlocked resources with persistent data will remain. If 'scratchResourcesOnly' is false
     * then all unlocked resources will be purged.
     * In either case, after the unlocked resources are purged a separate pass will be made to
     * ensure that resource usage is under budget (i.e., even if 'scratchResourcesOnly' is true
     * some resources with persistent data may be purged to be under budget).
     *
     * @param scratchResourcesOnly   If true only unlocked scratch resources will be purged prior
     *                               enforcing the budget requirements.
     */
    void purgeUnlockedResources(bool scratchResourcesOnly);

    /**
     * Gets the maximum supported texture size.
     */
    using GrRecordingContext::maxTextureSize;

    /**
     * Gets the maximum supported render target size.
     */
    using GrRecordingContext::maxRenderTargetSize;

    /**
     * Can a SkImage be created with the given color type.
     */
    using GrRecordingContext::colorTypeSupportedAsImage;

    /**
     * Can a SkSurface be created with the given color type. To check whether MSAA is supported
     * use maxSurfaceSampleCountForColorType().
     */
    using GrRecordingContext::colorTypeSupportedAsSurface;

    /**
     * Gets the maximum supported sample count for a color type. 1 is returned if only non-MSAA
     * rendering is supported for the color type. 0 is returned if rendering to this color type
     * is not supported at all.
     */
    using GrRecordingContext::maxSurfaceSampleCountForColorType;

protected:
    GrDirectContext(GrBackendApi backend, const GrContextOptions& options);

    bool init() override;

    GrAtlasManager* onGetAtlasManager() override { return fAtlasManager.get(); }
    GrSmallPathAtlasMgr* onGetSmallPathAtlasMgr() override;

    GrDirectContext* asDirectContext() override { return this; }

private:
    std::unique_ptr<GrAtlasManager> fAtlasManager;

    std::unique_ptr<GrSmallPathAtlasMgr> fSmallPathAtlasMgr;

    using INHERITED = GrContext;
};


#endif
