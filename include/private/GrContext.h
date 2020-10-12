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
#include "include/gpu/GrRecordingContext.h"

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
class GrSmallPathAtlasMgr;
class GrStrikeCache;
class GrSurfaceProxy;
class GrSwizzle;
class GrTextureProxy;
struct GrVkBackendContext;

class SkImage;
class SkString;
class SkSurfaceCharacterization;
class SkSurfaceProps;
class SkTaskGroup;
class SkTraceMemoryDump;

/**
 * This deprecated class is being merged into GrDirectContext and removed.
 * Do not add new subclasses, new API, or attempt to instantiate one.
 * If new API requires direct GPU access, add it to GrDirectContext.
 * Otherwise, add it to GrRecordingContext.
 */
class SK_API GrContext : public GrRecordingContext {
public:
    ~GrContext() override;


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
                                                    GrMipmapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    SkImage::CompressionType,
                                                    const SkColor4f& color,
                                                    GrMipmapped,
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
     * must be sized correctly (please see SkMipmap::ComputeLevelSize and ComputeLevelCount).
     * For the Vulkan backend the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    const GrBackendFormat&,
                                                    const void* data, size_t dataSize,
                                                    GrMipmapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    GrBackendTexture createCompressedBackendTexture(int width, int height,
                                                    SkImage::CompressionType,
                                                    const void* data, size_t dataSize,
                                                    GrMipmapped,
                                                    GrProtected = GrProtected::kNo,
                                                    GrGpuFinishedProc finishedProc = nullptr,
                                                    GrGpuFinishedContext finishedContext = nullptr);

    /**
     * If possible, updates a backend texture filled with the provided color. If the texture is
     * mipmapped, all levels of the mip chain will be updated to have the supplied color. The client
     * should check the return value to see if the update was successful. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture.
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateCompressedBackendTexture(const GrBackendTexture&,
                                        const SkColor4f& color,
                                        GrGpuFinishedProc finishedProc,
                                        GrGpuFinishedContext finishedContext);

    /**
     * If possible, updates a backend texture filled with the provided raw data. The client
     * should check the return value to see if the update was successful. The client can pass in a
     * finishedProc to be notified when the data has been uploaded by the gpu and the texture can be
     * deleted. The client is required to call GrContext::submit to send the upload work to the gpu.
     * The finishedProc will always get called even if we failed to create the GrBackendTexture.
     * If a mipMapped texture is passed in, the data for all the mipmap levels must be provided.
     * Additionally, all the miplevels must be sized correctly (please see
     * SkMipMap::ComputeLevelSize and ComputeLevelCount).
     * For the Vulkan backend after a successful update the layout of the created VkImage will be:
     *      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
     */
    bool updateCompressedBackendTexture(const GrBackendTexture&,
                                        const void* data,
                                        size_t dataSize,
                                        GrGpuFinishedProc finishedProc,
                                        GrGpuFinishedContext finishedContext);

    /**
     * Updates the state of the GrBackendTexture/RenderTarget to have the passed in
     * GrBackendSurfaceMutableState. All objects that wrap the backend surface (i.e. SkSurfaces and
     * SkImages) will also be aware of this state change. This call does not submit the state change
     * to the gpu, but requires the client to call GrContext::submit to send it to the GPU. The work
     * for this call is ordered linearly with all other calls that require GrContext::submit to be
     * called (e.g updateBackendTexture and flush). If finishedProc is not null then it will be
     * called with finishedContext after the state transition is known to have occurred on the GPU.
     *
     * See GrBackendSurfaceMutableState to see what state can be set via this call.
     *
     * If the backend API is Vulkan, the caller can set the GrBackendSurfaceMutableState's
     * VkImageLayout to VK_IMAGE_LAYOUT_UNDEFINED or queueFamilyIndex to VK_QUEUE_FAMILY_IGNORED to
     * tell Skia to not change those respective states.
     *
     * If previousState is not null and this returns true, then Skia will have filled in
     * previousState to have the values of the state before this call.
     */
    bool setBackendTextureState(const GrBackendTexture&,
                                const GrBackendSurfaceMutableState&,
                                GrBackendSurfaceMutableState* previousState = nullptr,
                                GrGpuFinishedProc finishedProc = nullptr,
                                GrGpuFinishedContext finishedContext = nullptr);
    bool setBackendRenderTargetState(const GrBackendRenderTarget&,
                                     const GrBackendSurfaceMutableState&,
                                     GrBackendSurfaceMutableState* previousState = nullptr,
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

    // Provides access to functions that aren't part of the public API.
    GrContextPriv priv();
    const GrContextPriv priv() const;  // NOLINT(readability-const-return-type)
protected:
    GrContext(sk_sp<GrContextThreadSafeProxy>);

    virtual GrAtlasManager* onGetAtlasManager() = 0;
    virtual GrSmallPathAtlasMgr* onGetSmallPathAtlasMgr() = 0;

private:
    friend class GrDirectContext; // for access to fGpu

    // fTaskGroup must appear before anything that uses it (e.g. fGpu), so that it is destroyed
    // after all of its users. Clients of fTaskGroup will generally want to ensure that they call
    // wait() on it as they are being destroyed, to avoid the possibility of pending tasks being
    // invoked after objects they depend upon have already been destroyed.
    std::unique_ptr<SkTaskGroup>            fTaskGroup;
    std::unique_ptr<GrStrikeCache>          fStrikeCache;
    sk_sp<GrGpu>                            fGpu;
    std::unique_ptr<GrResourceCache>        fResourceCache;
    std::unique_ptr<GrResourceProvider>     fResourceProvider;

    bool                                    fDidTestPMConversions;
    // true if the PM/UPM conversion succeeded; false otherwise
    bool                                    fPMUPMConversionsRoundTrip;

    GrContextOptions::PersistentCache*      fPersistentCache;
    GrContextOptions::ShaderErrorHandler*   fShaderErrorHandler;

    std::unique_ptr<GrClientMappedBufferManager> fMappedBufferManager;

    // TODO: have the GrClipStackClip use renderTargetContexts and rm this friending
    friend class GrContextPriv;

    using INHERITED = GrRecordingContext;
};

#endif
