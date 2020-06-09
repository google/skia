/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContext_DEFINED
#define GrContext_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/GrRecordingContext.h"

// We shouldn't need this but currently Android is relying on this being include transitively.
#include "include/core/SkUnPreMultiply.h"

class GrAtlasManager;
class GrBackendSemaphore;
class GrCaps;
class GrClientMappedBufferManager;
class GrContextPriv;
class GrContextThreadSafeProxy;
struct GrD3DBackendContext;
class GrFragmentProcessor;
struct GrGLInterface;
class GrGpu;
struct GrMockOptions;
class GrPath;
class GrRenderTargetContext;
class GrResourceCache;
class GrResourceProvider;
class GrStrikeCache;
class GrSurfaceProxy;
class GrSwizzle;
class GrTextContext;
class GrTextureProxy;
struct GrVkBackendContext;

class SkImage;
class SkString;
class SkSurfaceCharacterization;
class SkSurfaceProps;
class SkTaskGroup;
class SkTraceMemoryDump;

class SK_API GrContext : public GrRecordingContext {
public:
#ifdef SK_GL
    /**
     * Creates a GrContext for a backend context. If no GrGLInterface is provided then the result of
     * GrGLMakeNativeInterface() is used if it succeeds.
     */
    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>, const GrContextOptions&);
    static sk_sp<GrContext> MakeGL(sk_sp<const GrGLInterface>);
    static sk_sp<GrContext> MakeGL(const GrContextOptions&);
    static sk_sp<GrContext> MakeGL();
#endif

    /**
     * The Vulkan context (VkQueue, VkDevice, VkInstance) must be kept alive until the returned
     * GrContext is first destroyed or abandoned.
     */
    static sk_sp<GrContext> MakeVulkan(const GrVkBackendContext&, const GrContextOptions&);
    static sk_sp<GrContext> MakeVulkan(const GrVkBackendContext&);

#ifdef SK_METAL
    /**
     * Makes a GrContext which uses Metal as the backend. The device parameter is an MTLDevice
     * and queue is an MTLCommandQueue which should be used by the backend. These objects must
     * have a ref on them which can be transferred to Ganesh which will release the ref when the
     * GrContext is destroyed.
     */
    static sk_sp<GrContext> MakeMetal(void* device, void* queue, const GrContextOptions& options);
    static sk_sp<GrContext> MakeMetal(void* device, void* queue);
#endif

#ifdef SK_DIRECT3D
    /**
     * Makes a GrContext which uses Direct3D as the backend. The Direct3D context
     * must be kept alive until the returned GrContext is first destroyed or abandoned.
     */
    static sk_sp<GrContext> MakeDirect3D(const GrD3DBackendContext&,
                                         const GrContextOptions& options);
    static sk_sp<GrContext> MakeDirect3D(const GrD3DBackendContext&);
#endif

#ifdef SK_DAWN
    static sk_sp<GrContext> MakeDawn(const wgpu::Device& device, const GrContextOptions& options);
    static sk_sp<GrContext> MakeDawn(const wgpu::Device& device);
#endif

    static sk_sp<GrContext> MakeMock(const GrMockOptions*, const GrContextOptions&);
    static sk_sp<GrContext> MakeMock(const GrMockOptions*);

    ~GrContext() override;

    // TODO: Remove this from public after migrating Chrome.
    sk_sp<GrContextThreadSafeProxy> threadSafeProxy();

    /**
     * The GrContext normally assumes that no outsider is setting state
     * within the underlying 3D API's context/device/whatever. This call informs
     * the context that the state was modified and it should resend. Shouldn't
     * be called frequently for good performance.
     * The flag bits, state, is dpendent on which backend is used by the
     * context, either GL or D3D (possible in future).
     */
    void resetContext(uint32_t state = kAll_GrBackendState);

    /**
     * If the backend is GrBackendApi::kOpenGL, then all texture unit/target combinations for which
     * the GrContext has modified the bound texture will have texture id 0 bound. This does not
     * flush the GrContext. Calling resetContext() does not change the set that will be bound
     * to texture id 0 on the next call to resetGLTextureBindings(). After this is called
     * all unit/target combinations are considered to have unmodified bindings until the GrContext
     * subsequently modifies them (meaning if this is called twice in a row with no intervening
     * GrContext usage then the second call is a no-op.)
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
     * create the GrContext must be alive before calling abandonContext.
     */
    void abandonContext() override;

    /**
     * Returns true if the context was abandoned or if the if the backend specific context has
     * gotten into an unrecoverarble, lost state (e.g. in Vulkan backend if we've gotten a
     * VK_ERROR_DEVICE_LOST). If the backend context is lost, this call will also abandon the
     * GrContext.
     */
    bool abandoned() override;

    /**
     * This is similar to abandonContext() however the underlying 3D context is not yet lost and
     * the GrContext will cleanup all allocated resources before returning. After returning it will
     * assume that the underlying context may no longer be valid.
     *
     * The typical use case for this function is that the client is going to destroy the 3D context
     * but can't guarantee that GrContext will be destroyed first (perhaps because it may be ref'ed
     * elsewhere by either the client or Skia objects).
     *
     * For Vulkan, even if the device becomes lost, the VkQueue, VkDevice, or VkInstance used to
     * create the GrContext must be alive before calling releaseResourcesAndAbandonContext.
     */
    virtual void releaseResourcesAndAbandonContext();

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
    virtual void freeGpuResources();

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
    int maxTextureSize() const;

    /**
     * Gets the maximum supported render target size.
     */
    int maxRenderTargetSize() const;

    /**
     * Can a SkImage be created with the given color type.
     */
    bool colorTypeSupportedAsImage(SkColorType) const;

    /**
     * Can a SkSurface be created with the given color type. To check whether MSAA is supported
     * use maxSurfaceSampleCountForColorType().
     */
    bool colorTypeSupportedAsSurface(SkColorType colorType) const {
        if (kR16G16_unorm_SkColorType == colorType ||
            kA16_unorm_SkColorType == colorType ||
            kA16_float_SkColorType == colorType ||
            kR16G16_float_SkColorType == colorType ||
            kR16G16B16A16_unorm_SkColorType == colorType ||
            kGray_8_SkColorType == colorType) {
            return false;
        }

        return this->maxSurfaceSampleCountForColorType(colorType) > 0;
    }

    /**
     * Gets the maximum supported sample count for a color type. 1 is returned if only non-MSAA
     * rendering is supported for the color type. 0 is returned if rendering to this color type
     * is not supported at all.
     */
    int maxSurfaceSampleCountForColorType(SkColorType) const;

    ///////////////////////////////////////////////////////////////////////////
    // Misc.

    /**
     * Inserts a list of GPU semaphores that the current GPU-backed API must wait on before
     * executing any more commands on the GPU. Skia will take ownership of the underlying semaphores
     * and delete them once they have been signaled and waited on. If this call returns false, then
     * the GPU back-end will not wait on any passed in semaphores, and the client will still own the
     * semaphores.
     */
    bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores);

    /**
     * Call to ensure all drawing to the context has been flushed and submitted to the underlying 3D
     * API. This is equivalent to calling GrContext::flush with a default GrFlushInfo followed by
     * GrContext::submit.
     */
    void flushAndSubmit() {
        this->flush(GrFlushInfo(), GrPrepareForExternalIORequests());
        this->submit();
    }

    /**
     * Call to ensure all drawing to the context has been flushed to underlying 3D API specific
     * objects. A call to GrContext::submit is always required to ensure work is actually sent to
     * the gpu. Some specific API details:
     *     GL: Commands are actually sent to the driver, but glFlush is never called. Thus some
     *         sync objects from the flush will not be valid until a submission occurs.
     *
     *     Vulkan/Metal/D3D/Dawn: Commands are recorded to the backend APIs corresponding command
     *         buffer or encoder objects. However, these objects are not sent to the gpu until a
     *         submission occurs.
     *
     * If the return is GrSemaphoresSubmitted::kYes, only initialized GrBackendSemaphores will be
     * submitted to the gpu during the next submit call (it is possible Skia failed to create a
     * subset of the semaphores). The client should not wait on these semaphores until after submit
     * has been called, and must keep them alive until then. If this call returns
     * GrSemaphoresSubmitted::kNo, the GPU backend will not submit any semaphores to be signaled on
     * the GPU. Thus the client should not have the GPU wait on any of the semaphores passed in with
     * the GrFlushInfo. Regardless of whether semaphores were submitted to the GPU or not, the
     * client is still responsible for deleting any initialized semaphores.
     * Regardleess of semaphore submission the context will still be flushed. It should be
     * emphasized that a return value of GrSemaphoresSubmitted::kNo does not mean the flush did not
     * happen. It simply means there were no semaphores submitted to the GPU. A caller should only
     * take this as a failure if they passed in semaphores to be submitted.
     */
    GrSemaphoresSubmitted flush(const GrFlushInfo& info) {
        return this->flush(info, GrPrepareForExternalIORequests());
    }

    /**
     * Call to ensure all drawing to the context has been flushed to underlying 3D API specific
     * objects. A call to GrContext::submit is always required to ensure work is actually sent to
     * the gpu. Some specific API details:
     *     GL: Commands are actually sent to the driver, but glFlush is never called. Thus some
     *         sync objects from the flush will not be valid until submit is called.
     *
     *     Vulkan/Metal/D3D/Dawn: Commands are recorded to the backend APIs corresponding command
     *         buffer or encoder objects. However, these objects are not sent to the gpu until
     *         submit is called.
     *
     * Note: The default values for GrFlushInfo will submit the work the gpu.
     *
     * If the return is GrSemaphoresSubmitted::kYes, only initialized GrBackendSemaphores will be
     * submitted to the gpu during the next submit call (it is possible Skia failed to create a
     * subset of the semaphores). The client should not wait on these semaphores until after submit
     * has been called, and must keep them alive until then. If this call returns
     * GrSemaphoresSubmitted::kNo, the GPU backend will not submit any semaphores to be signaled on
     * the GPU. Thus the client should not have the GPU wait on any of the semaphores passed in with
     * the GrFlushInfo. The client is always responsible for deleting any initialized semaphores.
     * Regardleess of semaphore submission the context will still be flushed. It should be
     * emphasized that a return value of GrSemaphoresSubmitted::kNo does not mean the flush did not
     * happen. It simply means there were no semaphores submitted to the GPU. A caller should only
     * take this as a failure if they passed in semaphores to be submitted.
     *
     * If the GrPrepareForExternalIORequests contains valid gpu backed SkSurfaces or SkImages, Skia
     * will put the underlying backend objects into a state that is ready for external uses. See
     * declaration of GrPreopareForExternalIORequests for more details. Note that the backend
     * objects will not be moved to this state until submit has been called. If subsequent flushes
     * are called between this and submit, those objects are no longer guaranteed to be in a state
     * that is ready for external use.
     */
    GrSemaphoresSubmitted flush(const GrFlushInfo&, const GrPrepareForExternalIORequests&);

    /**
     * Submit outstanding work to the gpu from all previously un-submitted flushes. The return
     * value of the submit will indicate whether or not the submission to the GPU was successful.
     *
     * If the call returns true, all previously passed in semaphores in flush calls will have been
     * submitted to the GPU and they can safely be waited on. The caller should wait on those
     * semaphores or perform some other global synchronization before deleting the semaphores.
     *
     * If it returns false, then those same semaphores will not have been submitted and we will not
     * try to submit them again. The caller is free to delete the semaphores at any time.
     *
     * If the syncCpu flag is true this function will return once the gpu has finished with all
     * submitted work.
     */
    bool submit(bool syncCpu = false);

    /**
     * Checks whether any asynchronous work is complete and if so calls related callbacks.
     */
    void checkAsyncWorkCompletion();

    // Provides access to functions that aren't part of the public API.
    GrContextPriv priv();
    const GrContextPriv priv() const;

    /** Enumerates all cached GPU resources and dumps their memory to traceMemoryDump. */
    // Chrome is using this!
    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const;

    bool supportsDistanceFieldText() const;

    void storeVkPipelineCacheData();

    // Returns the gpu memory size of the the texture that backs the passed in SkImage. Returns 0 if
    // the SkImage is not texture backed. For external format textures this will also return 0 as we
    // cannot determine the correct size.
    static size_t ComputeImageSize(sk_sp<SkImage> image, GrMipMapped, bool useNextPow2 = false);

    /**
     * Retrieve the default GrBackendFormat for a given SkColorType and renderability.
     * It is guaranteed that this backend format will be the one used by the following
     * SkColorType and SkSurfaceCharacterization-based createBackendTexture methods.
     *
     * The caller should check that the returned format is valid.
     */
    GrBackendFormat defaultBackendFormat(SkColorType ct, GrRenderable renderable) const {
        return INHERITED::defaultBackendFormat(ct, renderable);
    }

   /**
    * The explicitly allocated backend texture API allows clients to use Skia to create backend
    * objects outside of Skia proper (i.e., Skia's caching system will not know about them.)
    *
    * It is the client's responsibility to delete all these objects (using deleteBackendTexture)
    * before deleting the GrContext used to create them. If the backend is Vulkan, the textures must
    * be deleted before abandoning the GrContext as well. Additionally, clients should only delete
    * these objects on the thread for which that GrContext is active.
    *
    * The client is responsible for ensuring synchronization between different uses
    * of the backend object (i.e., wrapping it in a surface, rendering to it, deleting the
    * surface, rewrapping it in a image and drawing the image will require explicit
    * sychronization on the client's part).
    */

    /**
     * If possible, create an uninitialized backend texture. The client should ensure that the
     * returned backend texture is valid.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_UNDEFINED.
     */
    GrBackendTexture createBackendTexture(int width, int height,
                                          const GrBackendFormat&,
                                          GrMipMapped,
                                          GrRenderable,
                                          GrProtected = GrProtected::kNo);

    /**
     * If possible, create an uninitialized backend texture. The client should ensure that the
     * returned backend texture is valid.
     * If successful, the created backend texture will be compatible with the provided
     * SkColorType.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_UNDEFINED.
     */
    GrBackendTexture createBackendTexture(int width, int height,
                                          SkColorType,
                                          GrMipMapped,
                                          GrRenderable,
                                          GrProtected = GrProtected::kNo);


    /**
     * If possible, create an uninitialized backend texture that is compatible with the
     * provided characterization. The client should ensure that the returned backend texture
     * is valid.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_UNDEFINED.
     */
    GrBackendTexture createBackendTexture(const SkSurfaceCharacterization& characterization);

    /**
     * If possible, create a backend texture initialized to a particular color. The client should
     * ensure that the returned backend texture is valid. The client can pass in a finishedProc
     * to be notified when the data has been uploaded by the gpu and the texture can be deleted. The
     * client is required to call GrContext::submit to send the upload work to the gpu. The
     * finishedProc will always get called even if we failed to create the GrBackendTexture.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createBackendTexture(int width, int height,
                                          const GrBackendFormat&,
                                          const SkColor4f& color,
                                          GrMipMapped,
                                          GrRenderable,
                                          GrProtected = GrProtected::kNo,
                                          GrGpuFinishedProc finishedProc = nullptr,
                                          GrGpuFinishedContext finishedContext = nullptr);

    /**
     * If possible, create a backend texture initialized to a particular color. The client should
     * ensure that the returned backend texture is valid. The client can pass in a finishedProc
     * to be notified when the data has been uploaded by the gpu and the texture can be deleted. The
     * client is required to call GrContext::submit to send the upload work to the gpu. The
     * finishedProc will always get called even if we failed to create the GrBackendTexture.
     * If successful, the created backend texture will be compatible with the provided
     * SkColorType.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createBackendTexture(int width, int height,
                                          SkColorType,
                                          const SkColor4f& color,
                                          GrMipMapped,
                                          GrRenderable,
                                          GrProtected = GrProtected::kNo,
                                          GrGpuFinishedProc finishedProc = nullptr,
                                          GrGpuFinishedContext finishedContext = nullptr);

    /**
     * If possible, create a backend texture initialized to a particular color that is
     * compatible with the provided characterization. The client should ensure that the
     * returned backend texture is valid. The client can pass in a finishedProc to be notified when
     * the data has been uploaded by the gpu and the texture can be deleted. The client is required
     * to call GrContext::submit to send the upload work to the gpu. The finishedProc will always
     * get called even if we failed to create the GrBackendTexture.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL if texturaeble
     *      VK_IMAGE_LAYOUT_UNDEFINED                if not textureable
     */
    GrBackendTexture createBackendTexture(const SkSurfaceCharacterization& characterization,
                                          const SkColor4f& color,
                                          GrGpuFinishedProc finishedProc = nullptr,
                                          GrGpuFinishedContext finishedContext = nullptr);

    /**
     * If possible, create a backend texture initialized with the provided pixmap data. The client
     * should ensure that the returned backend texture is valid. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture.
     * If successful, the created backend texture will be compatible with the provided
     * pixmap(s). Compatible, in this case, means that the backend format will be the result
     * of calling defaultBackendFormat on the base pixmap's colortype. The src data can be deleted
     * when this call returns.
     * If numLevels is 1 a non-mipMapped texture will result. If a mipMapped texture is desired
     * the data for all the mipmap levels must be provided. In the mipmapped case all the
     * colortypes of the provided pixmaps must be the same. Additionally, all the miplevels
     * must be sized correctly (please see SkMipMap::ComputeLevelSize and ComputeLevelCount).
     * Note: the pixmap's alphatypes and colorspaces are ignored.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createBackendTexture(const SkPixmap srcData[], int numLevels,
                                          GrRenderable, GrProtected,
                                          GrGpuFinishedProc finishedProc = nullptr,
                                          GrGpuFinishedContext finishedContext = nullptr);

    // Helper version of above for a single level.
    GrBackendTexture createBackendTexture(const SkPixmap& srcData,
                                          GrRenderable renderable,
                                          GrProtected isProtected,
                                          GrGpuFinishedProc finishedProc = nullptr,
                                          GrGpuFinishedContext finishedContext = nullptr) {
        return this->createBackendTexture(&srcData, 1, renderable, isProtected, finishedProc,
                                          finishedContext);
    }

    /**
     * If possible, updates a backend texture to be filled to a particular color. The client should
     * check the return value to see if the update was successful. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to update the GrBackendTexture.
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateBackendTexture(const GrBackendTexture&,
                              const SkColor4f& color,
                              GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext);

    /**
     * If possible, updates a backend texture filled with the provided pixmap data. The client
     * should check the return value to see if the update was successful. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture.
     * The backend texture must be compatible with the provided pixmap(s). Compatible, in this case,
     * means that the backend format is compatible with the base pixmap's colortype. The src data
     * can be deleted when this call returns.
     * If the backend texture is mip mapped, the data for all the mipmap levels must be provided.
     * In the mipmapped case all the colortypes of the provided pixmaps must be the same.
     * Additionally, all the miplevels must be sized correctly (please see
     * SkMipMap::ComputeLevelSize and ComputeLevelCount).
     * Note: the pixmap's alphatypes and colorspaces are ignored.
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateBackendTexture(const GrBackendTexture&,
                              const SkPixmap srcData[],
                              int numLevels,
                              GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext);

    /**
     * Retrieve the GrBackendFormat for a given SkImage::CompressionType. This is
     * guaranteed to match the backend format used by the following
     * createCompressedsBackendTexture methods that take a CompressionType.
     * The caller should check that the returned format is valid.
     */
    GrBackendFormat compressedBackendFormat(SkImage::CompressionType compression) const {
        return INHERITED::compressedBackendFormat(compression);
    }

    /**
     *If possible, create a compressed backend texture initialized to a particular color. The
     * client should ensure that the returned backend texture is valid. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture.
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    const GrBackendFormat&,
                                                    const SkColor4f& color,
                                                    GrMipMapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    SkImage::CompressionType,
                                                    const SkColor4f& color,
                                                    GrMipMapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    /**
     * If possible, create a backend texture initialized with the provided raw data. The client
     * should ensure that the returned backend texture is valid. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture
     * If numLevels is 1 a non-mipMapped texture will result. If a mipMapped texture is desired
     * the data for all the mipmap levels must be provided. Additionally, all the miplevels
     * must be sized correctly (please see SkMipMap::ComputeLevelSize and ComputeLevelCount).
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    const GrBackendFormat&,
                                                    const void* data, size_t dataSize,
                                                    GrMipMapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    SkImage::CompressionType,
                                                    const void* data, size_t dataSize,
                                                    GrMipMapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    void deleteBackendTexture(GrBackendTexture);

    // This interface allows clients to pre-compile shaders and populate the runtime program cache.
    // The key and data blobs should be the ones passed to the PersistentCache, in SkSL format.
    //
    // Steps to use this API:
    //
    // 1) Create a GrContext as normal, but set fPersistentCache on GrContextOptions to something
    //    that will save the cached shader blobs. Set fShaderCacheStrategy to kSkSL. This will
    //    ensure that the blobs are SkSL, and are suitable for pre-compilation.
    // 2) Run your application, and save all of the key/data pairs that are fed to the cache.
    //
    // 3) Switch over to shipping your application. Include the key/data pairs from above.
    // 4) At startup (or any convenient time), call precompileShader for each key/data pair.
    //    This will compile the SkSL to create a GL program, and populate the runtime cache.
    //
    // This is only guaranteed to work if the context/device used in step #2 are created in the
    // same way as the one used in step #4, and the same GrContextOptions are specified.
    // Using cached shader blobs on a different device or driver are undefined.
    bool precompileShader(const SkData& key, const SkData& data);

#ifdef SK_ENABLE_DUMP_GPU
    /** Returns a string with detailed information about the context & GPU, in JSON format. */
    SkString dump() const;
#endif

protected:
    GrContext(sk_sp<GrContextThreadSafeProxy>);

    bool init() override;

    GrContext* asDirectContext() override { return this; }

    virtual GrAtlasManager* onGetAtlasManager() = 0;

private:
    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup>            fTaskGroup;
    std::unique_ptr<GrStrikeCache>          fStrikeCache;
    sk_sp<GrGpu>                            fGpu;
    GrResourceCache*                        fResourceCache;
    GrResourceProvider*                     fResourceProvider;

    bool                                    fDidTestPMConversions;
    // true if the PM/UPM conversion succeeded; false otherwise
    bool                                    fPMUPMConversionsRoundTrip;

    GrContextOptions::PersistentCache*      fPersistentCache;
    GrContextOptions::ShaderErrorHandler*   fShaderErrorHandler;

    std::unique_ptr<GrClientMappedBufferManager> fMappedBufferManager;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    typedef GrRecordingContext INHERITED;
};

#endif
