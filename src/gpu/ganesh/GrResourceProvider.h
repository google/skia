/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceProvider_DEFINED
#define GrResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrGpuResource.h"
#include "src/gpu/ganesh/GrResourceCache.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>

class GrAttachment;
class GrBackendFormat;
class GrBackendRenderTarget;
class GrBackendSemaphore;
class GrBackendTexture;
class GrGpu;
class GrRenderTarget;
class GrResourceProviderPriv;
class GrSemaphore;
class GrTexture;
class SkData;

enum class SkBackingFit;
struct GrVkDrawableInfo;
struct SkImageInfo;

namespace skgpu {
class ScratchKey;
class SingleOwner;
class UniqueKey;
enum class Budgeted : bool;
enum class Mipmapped : bool;
enum class Protected : bool;
enum class Renderable : bool;
}

/**
 * A factory for arbitrary resource types.
 */
class GrResourceProvider {
public:
    GrResourceProvider(GrGpu*, GrResourceCache*, skgpu::SingleOwner*);

    /**
     * Finds a resource in the cache, based on the specified key. Prior to calling this, the caller
     * must be sure that if a resource of exists in the cache with the given unique key then it is
     * of type T.
     */
    template <typename T = GrGpuResource>
    typename std::enable_if<std::is_base_of<GrGpuResource, T>::value, sk_sp<T>>::type
    findByUniqueKey(const skgpu::UniqueKey& key) {
        return sk_sp<T>(static_cast<T*>(this->findResourceByUniqueKey(key).release()));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     * Finds a texture that approximately matches the descriptor. Will be at least as large in width
     * and height as desc specifies. If renderable is kYes then the GrTexture will also be a
     * GrRenderTarget. The texture's format and sample count will always match the request.
     * The contents of the texture are undefined.
     */
    sk_sp<GrTexture> createApproxTexture(SkISize dimensions,
                                         const GrBackendFormat& format,
                                         GrTextureType textureType,
                                         skgpu::Renderable renderable,
                                         int renderTargetSampleCnt,
                                         skgpu::Protected isProtected,
                                         std::string_view label);

    /** Create an exact fit texture with no initial data to upload. */
    sk_sp<GrTexture> createTexture(SkISize dimensions,
                                   const GrBackendFormat& format,
                                   GrTextureType textureType,
                                   skgpu::Renderable renderable,
                                   int renderTargetSampleCnt,
                                   skgpu::Mipmapped mipmapped,
                                   skgpu::Budgeted budgeted,
                                   skgpu::Protected isProtected,
                                   std::string_view label);

    /**
     * Create an exact fit texture with initial data to upload. The color type must be valid
     * for the format and also describe the texel data. This will ensure any conversions that
     * need to get applied to the data before upload are applied.
     */
    sk_sp<GrTexture> createTexture(SkISize dimensions,
                                   const GrBackendFormat& format,
                                   GrTextureType textureType,
                                   GrColorType colorType,
                                   skgpu::Renderable renderable,
                                   int renderTargetSampleCnt,
                                   skgpu::Budgeted budgeted,
                                   skgpu::Mipmapped mipmapped,
                                   skgpu::Protected isProtected,
                                   const GrMipLevel texels[],
                                   std::string_view label);

    /**
     * Create a potentially loose fit texture with the provided data. The color type must be valid
     * for the format and also describe the texel data. This will ensure any conversions that
     * need to get applied to the data before upload are applied.
     */
    sk_sp<GrTexture> createTexture(SkISize dimensions,
                                   const GrBackendFormat&,
                                   GrTextureType textureType,
                                   GrColorType srcColorType,
                                   skgpu::Renderable,
                                   int renderTargetSampleCnt,
                                   skgpu::Budgeted,
                                   SkBackingFit,
                                   skgpu::Protected,
                                   const GrMipLevel& mipLevel,
                                   std::string_view label);

    /**
     * Search the cache for a scratch texture matching the provided arguments. Failing that
     * it returns null. If non-null, the resulting texture is always budgeted.
     */
    sk_sp<GrTexture> findAndRefScratchTexture(const skgpu::ScratchKey&, std::string_view label);
    sk_sp<GrTexture> findAndRefScratchTexture(SkISize dimensions,
                                              const GrBackendFormat&,
                                              GrTextureType textureType,
                                              skgpu::Renderable,
                                              int renderTargetSampleCnt,
                                              skgpu::Mipmapped,
                                              skgpu::Protected,
                                              std::string_view label);

    /**
     * Creates a compressed texture. The GrGpu must support the SkImageImage::Compression type.
     * It will not be renderable.
     */
    sk_sp<GrTexture> createCompressedTexture(SkISize dimensions,
                                             const GrBackendFormat&,
                                             skgpu::Budgeted,
                                             skgpu::Mipmapped,
                                             skgpu::Protected,
                                             SkData* data,
                                             std::string_view label);

    ///////////////////////////////////////////////////////////////////////////
    // Wrapped Backend Surfaces

    /**
     * Wraps an existing texture with a GrTexture object.
     *
     * GrIOType must either be kRead or kRW. kRead blocks any operations that would modify the
     * pixels (e.g. dst for a copy, regenerating MIP levels, write pixels).
     *
     * OpenGL: if the object is a texture Gr may change its GL texture params
     *         when it is drawn.
     *
     * @return GrTexture object or NULL on failure.
     */
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTexture& tex,
                                        GrWrapOwnership,
                                        GrWrapCacheable,
                                        GrIOType);

    sk_sp<GrTexture> wrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                  GrWrapOwnership,
                                                  GrWrapCacheable);

    /**
     * This makes the backend texture be renderable. If sampleCnt is > 1 and the underlying API
     * uses separate MSAA render buffers then a MSAA render buffer is created that resolves
     * to the texture.
     */
    sk_sp<GrTexture> wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                  int sampleCnt,
                                                  GrWrapOwnership,
                                                  GrWrapCacheable);

    /**
     * Wraps an existing render target with a GrRenderTarget object. It is
     * similar to wrapBackendTexture but can be used to draw into surfaces
     * that are not also textures (e.g. FBO 0 in OpenGL, or an MSAA buffer that
     * the client will resolve to a texture). Currently wrapped render targets
     * always use the kBorrow_GrWrapOwnership and GrWrapCacheable::kNo semantics.
     *
     * @return GrRenderTarget object or NULL on failure.
     */
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTarget&);

    sk_sp<GrRenderTarget> wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                              const GrVkDrawableInfo&);

    static const int kMinScratchTextureSize;

    /**
     * Either finds and refs a buffer with the given unique key, or creates a new new, fills its
     * contents with the InitializeBufferDataFn() callback, and assigns it the unique key.
     *
     * @param intendedType        hint to the graphics subsystem about how the buffer will be used.
     * @param size                minimum size of buffer to return.
     * @param key                 Key to be assigned to the buffer.
     * @param InitializeBufferFn  callback with which to initialize the buffer.
     *
     * @return The buffer if successful, otherwise nullptr.
     */
    using InitializeBufferFn = void(*)(skgpu::VertexWriter, size_t bufferSize);
    sk_sp<const GrGpuBuffer> findOrMakeStaticBuffer(GrGpuBufferType intendedType,
                                                    size_t size,
                                                    const skgpu::UniqueKey& key,
                                                    InitializeBufferFn);

    /**
     * Either finds and refs, or creates a static buffer with the given parameters and contents.
     *
     * @param intendedType    hint to the graphics subsystem about what the buffer will be used for.
     * @param size            minimum size of buffer to return.
     * @param data            optional data with which to initialize the buffer.
     * @param key             Key to be assigned to the buffer.
     *
     * @return The buffer if successful, otherwise nullptr.
     */
    sk_sp<const GrGpuBuffer> findOrMakeStaticBuffer(GrGpuBufferType intendedType,
                                                    size_t size,
                                                    const void* staticData,
                                                    const skgpu::UniqueKey& key);

    /**
     * Either finds and refs, or creates an index buffer with a repeating pattern for drawing
     * contiguous vertices of a repeated mesh. If the return is non-null, the caller owns a ref on
     * the returned GrBuffer.
     *
     * @param pattern     the pattern of indices to repeat
     * @param patternSize size in bytes of the pattern
     * @param reps        number of times to repeat the pattern
     * @param vertCount   number of vertices the pattern references
     * @param key         Key to be assigned to the index buffer.
     *
     * @return The index buffer if successful, otherwise nullptr.
     */
    sk_sp<const GrGpuBuffer> findOrCreatePatternedIndexBuffer(const uint16_t* pattern,
                                                              int patternSize,
                                                              int reps,
                                                              int vertCount,
                                                              const skgpu::UniqueKey& key) {
        if (auto buffer = this->findByUniqueKey<const GrGpuBuffer>(key)) {
            return buffer;
        }
        return this->createPatternedIndexBuffer(pattern, patternSize, reps, vertCount, &key);
    }

    /**
     * Returns an index buffer that can be used to render non-antialiased quads.
     * Each quad consumes 6 indices (0, 1, 2, 2, 1, 3) and 4 vertices.
     * Call MaxNumNonAAQuads to get the max allowed number of non-AA quads.
     * Draw with GrPrimitiveType::kTriangles
     * @ return the non-AA quad index buffer
     */
    sk_sp<const GrGpuBuffer> refNonAAQuadIndexBuffer() {
        if (!fNonAAQuadIndexBuffer) {
            fNonAAQuadIndexBuffer = this->createNonAAQuadIndexBuffer();
        }
        return fNonAAQuadIndexBuffer;
    }

    static int MaxNumNonAAQuads();
    static int NumVertsPerNonAAQuad();
    static int NumIndicesPerNonAAQuad();

    /**
     * Returns an index buffer that can be used to render antialiased quads.
     * Each quad consumes 30 indices and 8 vertices.
     * Call MaxNumAAQuads to get the max allowed number of AA quads.
     * Draw with GrPrimitiveType::kTriangles
     * @ return the AA quad index buffer
     */
    sk_sp<const GrGpuBuffer> refAAQuadIndexBuffer() {
        if (!fAAQuadIndexBuffer) {
            fAAQuadIndexBuffer = this->createAAQuadIndexBuffer();
        }
        return fAAQuadIndexBuffer;
    }

    static int MaxNumAAQuads();
    static int NumVertsPerAAQuad();
    static int NumIndicesPerAAQuad();

    enum class ZeroInit : bool { kNo = false, kYes = true };

    /**
     * Returns a buffer.
     *
     * @param size            minimum size of buffer to return.
     * @param GrGpuBufferType hint to the graphics subsystem about what the buffer will be used for.
     * @param GrAccessPattern hint to the graphics subsystem about how the data will be accessed.
     * @param ZeroInit        if kYes zero-initialize the buffer. Otherwise, contents are undefined.
     *
     * @return the buffer if successful, otherwise nullptr.
     */
    sk_sp<GrGpuBuffer> createBuffer(size_t size, GrGpuBufferType, GrAccessPattern, ZeroInit);

    /** Same as above but also fills the buffer from data. */
    sk_sp<GrGpuBuffer> createBuffer(const void* data,
                                    size_t size,
                                    GrGpuBufferType type,
                                    GrAccessPattern pattern);

    /**
     * If passed in render target already has a stencil buffer on the specified surface, return
     * true. Otherwise attempt to attach one and return true on success.
     */
    bool attachStencilAttachment(GrRenderTarget* rt, bool useMSAASurface);

    sk_sp<GrAttachment> makeMSAAAttachment(SkISize dimensions,
                                           const GrBackendFormat& format,
                                           int sampleCnt,
                                           skgpu::Protected isProtected,
                                           GrMemoryless isMemoryless);

    /**
     * Gets a GrAttachment that can be used for MSAA rendering. This attachment may be shared by
     * other users. Thus any renderpass that uses the attachment should not assume any specific
     * data at the start and should not try to save written data at the end. Ideally the render pass
     * should discard the data at the end.
     */
    sk_sp<GrAttachment> getDiscardableMSAAAttachment(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     int sampleCnt,
                                                     skgpu::Protected isProtected,
                                                     GrMemoryless memoryless);

    /**
     * Assigns a unique key to a resource. If the key is associated with another resource that
     * association is removed and replaced by this resource.
     */
    void assignUniqueKeyToResource(const skgpu::UniqueKey&, GrGpuResource*);

    std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true);

    std::unique_ptr<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                                      GrSemaphoreWrapType,
                                                      GrWrapOwnership = kBorrow_GrWrapOwnership);

    void abandon() {
        fCache = nullptr;
        fGpu = nullptr;
    }

    uint32_t contextUniqueID() const { return fCache->contextUniqueID(); }
    const GrCaps* caps() const { return fCaps.get(); }
    bool overBudget() const { return fCache->overBudget(); }

    static SkISize MakeApprox(SkISize);

    inline GrResourceProviderPriv priv();
    inline const GrResourceProviderPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    sk_sp<GrGpuResource> findResourceByUniqueKey(const skgpu::UniqueKey&);

    /*
     * Try to find an existing scratch texture that exactly matches 'desc'. If successful
     * update the budgeting accordingly.
     */
    sk_sp<GrTexture> getExactScratch(SkISize dimensions,
                                     const GrBackendFormat&,
                                     GrTextureType,
                                     skgpu::Renderable,
                                     int renderTargetSampleCnt,
                                     skgpu::Budgeted,
                                     skgpu::Mipmapped,
                                     skgpu::Protected,
                                     std::string_view label);

    // Attempts to find a resource in the cache that exactly matches the SkISize. Failing that
    // it returns null. If non-null, the resulting msaa attachment is always budgeted.
    sk_sp<GrAttachment> refScratchMSAAAttachment(SkISize dimensions,
                                                 const GrBackendFormat&,
                                                 int sampleCnt,
                                                 skgpu::Protected,
                                                 GrMemoryless memoryless,
                                                 std::string_view label);

    // Used to perform any conversions necessary to texel data before creating a texture with
    // existing data or uploading to a scratch texture.
    using TempLevels = skia_private::AutoSTArray<14, GrMipLevel>;
    using TempLevelDatas = skia_private::AutoSTArray<14, std::unique_ptr<char[]>>;
    GrColorType prepareLevels(const GrBackendFormat& format,
                              GrColorType,
                              SkISize baseSize,
                              const GrMipLevel texels[],
                              int mipLevelCount,
                              TempLevels*,
                              TempLevelDatas*) const;

    // GrResourceProvider may be asked to "create" a new texture with initial pixel data to populate
    // it. In implementation it may pull an existing texture from GrResourceCache and then write the
    // pixel data to the texture. It takes a width/height for the base level because we may be
    // using an approximate-sized scratch texture. On success the texture is returned and nullptr
    // on failure.
    sk_sp<GrTexture> writePixels(sk_sp<GrTexture> texture,
                                 GrColorType colorType,
                                 SkISize baseSize,
                                 const GrMipLevel texels[],
                                 int mipLevelCount) const;

    GrResourceCache* cache() { return fCache; }
    const GrResourceCache* cache() const { return fCache; }

    friend class GrResourceProviderPriv;

    // Method made available via GrResourceProviderPriv
    GrGpu* gpu() { return fGpu; }
    const GrGpu* gpu() const { return fGpu; }

    bool isAbandoned() const {
        SkASSERT(SkToBool(fGpu) == SkToBool(fCache));
        return !SkToBool(fCache);
    }

    sk_sp<const GrGpuBuffer> createPatternedIndexBuffer(const uint16_t* pattern,
                                                        int patternSize,
                                                        int reps,
                                                        int vertCount,
                                                        const skgpu::UniqueKey* key);

    sk_sp<const GrGpuBuffer> createNonAAQuadIndexBuffer();
    sk_sp<const GrGpuBuffer> createAAQuadIndexBuffer();

    GrResourceCache* fCache;
    GrGpu* fGpu;
    sk_sp<const GrCaps> fCaps;
    sk_sp<const GrGpuBuffer> fNonAAQuadIndexBuffer;
    sk_sp<const GrGpuBuffer> fAAQuadIndexBuffer;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable skgpu::SingleOwner* fSingleOwner;)
};

#endif
