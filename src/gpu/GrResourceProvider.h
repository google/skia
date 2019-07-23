/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceProvider_DEFINED
#define GrResourceProvider_DEFINED

#include "include/gpu/GrContextOptions.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrResourceCache.h"

class GrBackendRenderTarget;
class GrBackendSemaphore;
class GrBackendTexture;
class GrGpu;
class GrPath;
class GrRenderTarget;
class GrResourceProviderPriv;
class GrSemaphore;
class GrSingleOwner;
class GrStencilAttachment;
class GrTexture;
struct GrVkDrawableInfo;

class GrStyle;
class SkDescriptor;
class SkPath;
class SkTypeface;

/**
 * A factory for arbitrary resource types. This class is intended for use within the Gr code base.
 *
 * Some members force callers to make a flags (pendingIO) decision. This can be relaxed once
 * https://bug.skia.org/4156 is fixed.
 */
class GrResourceProvider {
public:
    /** These flags govern which scratch resources we are allowed to return */
    enum class Flags {
        kNone            = 0x0,

        /** If the caller intends to do direct reads/writes to/from the CPU then this flag must be
         *  set when accessing resources during a GrOpList flush. This includes the execution of
         *  GrOp objects. The reason is that these memory operations are done immediately and
         *  will occur out of order WRT the operations being flushed.
         *  Make this automatic: https://bug.skia.org/4156
         */
        kNoPendingIO     = 0x1,
    };

    GrResourceProvider(GrGpu*, GrResourceCache*, GrSingleOwner*);

    /**
     * Finds a resource in the cache, based on the specified key. Prior to calling this, the caller
     * must be sure that if a resource of exists in the cache with the given unique key then it is
     * of type T.
     */
    template <typename T = GrGpuResource>
    typename std::enable_if<std::is_base_of<GrGpuResource, T>::value, sk_sp<T>>::type
    findByUniqueKey(const GrUniqueKey& key) {
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
    sk_sp<GrTexture> createApproxTexture(const GrSurfaceDesc&, GrRenderable,
                                         int renderTargetSampleCnt, GrProtected, Flags);

    /** Create an exact fit texture with no initial data to upload. */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, GrRenderable, int renderTargetSampleCnt,
                                   SkBudgeted, GrProtected, Flags = Flags::kNone);

    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, GrRenderable, int renderTargetSampleCnt,
                                   SkBudgeted, GrProtected, const GrMipLevel texels[],
                                   int mipLevelCount);

    /** Create a potentially loose fit texture with the provided data */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, GrRenderable, int renderTargetSampleCnt,
                                   SkBudgeted, SkBackingFit, GrProtected, const GrMipLevel&, Flags);

    /**
     * Creates a compressed texture. The GrGpu must support the SkImageImage::Compression type.
     * This does not currently support MIP maps. It will not be renderable.
     */
    sk_sp<GrTexture> createCompressedTexture(int width, int height, SkImage::CompressionType,
                                             SkBudgeted, SkData* data);

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
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTexture& tex, GrColorType, GrWrapOwnership,
                                        GrWrapCacheable, GrIOType);

    /**
     * This makes the backend texture be renderable. If sampleCnt is > 1 and the underlying API
     * uses separate MSAA render buffers then a MSAA render buffer is created that resolves
     * to the texture.
     */
    sk_sp<GrTexture> wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                  int sampleCnt,
                                                  GrColorType,
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
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                  GrColorType colorType);

    sk_sp<GrRenderTarget> wrapVulkanSecondaryCBAsRenderTarget(const SkImageInfo&,
                                                              const GrVkDrawableInfo&);

    static const uint32_t kMinScratchTextureSize;

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
    sk_sp<const GrGpuBuffer> findOrMakeStaticBuffer(GrGpuBufferType intendedType, size_t size,
                                                    const void* data, const GrUniqueKey& key);

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
                                                              const GrUniqueKey& key) {
        if (auto buffer = this->findByUniqueKey<const GrGpuBuffer>(key)) {
            return buffer;
        }
        return this->createPatternedIndexBuffer(pattern, patternSize, reps, vertCount, &key);
    }

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 2, 1, 3, etc.
     * The max number of quads is the buffer's index capacity divided by 6.
     * Draw with GrPrimitiveType::kTriangles
     * @ return the quad index buffer
     */
    sk_sp<const GrGpuBuffer> refQuadIndexBuffer() {
        if (!fQuadIndexBuffer) {
            fQuadIndexBuffer = this->createQuadIndexBuffer();
        }
        return fQuadIndexBuffer;
    }

    static int QuadCountOfQuadBuffer();

    /**
     * Factories for GrPath objects. It's an error to call these if path rendering
     * is not supported.
     */
    sk_sp<GrPath> createPath(const SkPath&, const GrStyle&);

    /**
     * Returns a buffer.
     *
     * @param size            minimum size of buffer to return.
     * @param intendedType    hint to the graphics subsystem about what the buffer will be used for.
     * @param GrAccessPattern hint to the graphics subsystem about how the data will be accessed.
     * @param flags           see Flags enum.
     * @param data            optional data with which to initialize the buffer.
     *
     * @return the buffer if successful, otherwise nullptr.
     */
    sk_sp<GrGpuBuffer> createBuffer(size_t size, GrGpuBufferType intendedType, GrAccessPattern,
                                    const void* data = nullptr);

    /**
     * If passed in render target already has a stencil buffer with at least "numSamples" samples,
     * return true. Otherwise attempt to attach one and return true on success.
     */
    bool attachStencilAttachment(GrRenderTarget* rt, int numStencilSamples);

     /**
      * Wraps an existing texture with a GrRenderTarget object. This is useful when the provided
      * texture has a format that cannot be textured from by Skia, but we want to raster to it.
      *
      * The texture is wrapped as borrowed. The texture object will not be freed once the
      * render target is destroyed.
      *
      * @return GrRenderTarget object or NULL on failure.
      */
     sk_sp<GrRenderTarget> wrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                            int sampleCnt,
                                                            GrColorType);

    /**
     * Assigns a unique key to a resource. If the key is associated with another resource that
     * association is removed and replaced by this resource.
     */
    void assignUniqueKeyToResource(const GrUniqueKey&, GrGpuResource*);

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true);

    enum class SemaphoreWrapType {
        kWillSignal,
        kWillWait,
    };

    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore&,
                                            SemaphoreWrapType wrapType,
                                            GrWrapOwnership = kBorrow_GrWrapOwnership);

    void abandon() {
        fCache = nullptr;
        fGpu = nullptr;
    }

    uint32_t contextUniqueID() const { return fCache->contextUniqueID(); }
    const GrCaps* caps() const { return fCaps.get(); }
    bool overBudget() const { return fCache->overBudget(); }

    static uint32_t MakeApprox(uint32_t value);

    inline GrResourceProviderPriv priv();
    inline const GrResourceProviderPriv priv() const;

private:
    sk_sp<GrGpuResource> findResourceByUniqueKey(const GrUniqueKey&);

    // Attempts to find a resource in the cache that exactly matches the GrSurfaceDesc. Failing that
    // it returns null. If non-null, the resulting texture is always budgeted.
    sk_sp<GrTexture> refScratchTexture(const GrSurfaceDesc&, GrRenderable,
                                       int renderTargetSampleCnt, GrProtected, Flags);

    /*
     * Try to find an existing scratch texture that exactly matches 'desc'. If successful
     * update the budgeting accordingly.
     */
    sk_sp<GrTexture> getExactScratch(const GrSurfaceDesc&, GrRenderable, int renderTargetSampleCnt,
                                     SkBudgeted, GrProtected, Flags);

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
                                                        const GrUniqueKey* key);

    sk_sp<const GrGpuBuffer> createQuadIndexBuffer();

    GrResourceCache* fCache;
    GrGpu* fGpu;
    sk_sp<const GrCaps> fCaps;
    sk_sp<const GrGpuBuffer> fQuadIndexBuffer;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

GR_MAKE_BITFIELD_CLASS_OPS(GrResourceProvider::Flags);

#endif
