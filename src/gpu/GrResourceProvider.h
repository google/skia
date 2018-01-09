/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceProvider_DEFINED
#define GrResourceProvider_DEFINED

#include "GrBuffer.h"
#include "GrPathRange.h"
#include "GrResourceCache.h"
#include "SkImageInfo.h"
#include "SkScalerContext.h"

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
    GrResourceProvider(GrGpu* gpu, GrResourceCache* cache, GrSingleOwner* owner);

    /**
     * Finds a resource in the cache, based on the specified key. Prior to calling this, the caller
     * must be sure that if a resource of exists in the cache with the given unique key then it is
     * of type T.
     */
    template <typename T>
    sk_sp<T> findByUniqueKey(const GrUniqueKey& key) {
        return sk_sp<T>(static_cast<T*>(this->findResourceByUniqueKey(key).release()));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     * Finds a texture that approximately matches the descriptor. Will be at least as large in width
     * and height as desc specifies. If desc specifies that the texture should be a render target
     * then result will be a render target. Format and sample count will always match the request.
     * The contents of the texture are undefined.
     */
    sk_sp<GrTexture> createApproxTexture(const GrSurfaceDesc&, uint32_t flags);

    /** Create an exact fit texture with no initial data to upload.
     */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, SkBudgeted, uint32_t flags = 0);

    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, SkBudgeted,
                                   const GrMipLevel texels[], int mipLevelCount,
                                   SkDestinationSurfaceColorMode mipColorMode);

    // Create a potentially loose fit texture with the provided data
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc&, SkBudgeted, const GrMipLevel&);

    ///////////////////////////////////////////////////////////////////////////
    // Wrapped Backend Surfaces

    /**
     * Wraps an existing texture with a GrTexture object.
     *
     * OpenGL: if the object is a texture Gr may change its GL texture params
     *         when it is drawn.
     *
     * @return GrTexture object or NULL on failure.
     */
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTexture& tex,
                                        GrWrapOwnership = kBorrow_GrWrapOwnership);

    /**
     * This makes the backend texture be renderable. If sampleCnt is > 0 and the underlying API
     * uses separate MSAA render buffers then a MSAA render buffer is created that resolves
     * to the texture.
     */
    sk_sp<GrTexture> wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                  int sampleCnt,
                                                  GrWrapOwnership = kBorrow_GrWrapOwnership);

    /**
     * Wraps an existing render target with a GrRenderTarget object. It is
     * similar to wrapBackendTexture but can be used to draw into surfaces
     * that are not also textures (e.g. FBO 0 in OpenGL, or an MSAA buffer that
     * the client will resolve to a texture). Currently wrapped render targets
     * always use the kBorrow_GrWrapOwnership semantics.
     *
     * @return GrRenderTarget object or NULL on failure.
     */
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTarget&);

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
    sk_sp<const GrBuffer> findOrMakeStaticBuffer(GrBufferType intendedType, size_t size,
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
    sk_sp<const GrBuffer> findOrCreatePatternedIndexBuffer(const uint16_t* pattern,
                                                           int patternSize,
                                                           int reps,
                                                           int vertCount,
                                                           const GrUniqueKey& key) {
        if (auto buffer = this->findByUniqueKey<GrBuffer>(key)) {
            return buffer;
        }
        return this->createPatternedIndexBuffer(pattern, patternSize, reps, vertCount, key);
    }

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 2, 1, 3, etc.
     * The max number of quads is the buffer's index capacity divided by 6.
     * Draw with GrPrimitiveType::kTriangles
     * @ return the quad index buffer
     */
    sk_sp<const GrBuffer> refQuadIndexBuffer() {
        if (auto buffer = this->findByUniqueKey<const GrBuffer>(fQuadIndexBufferKey)) {
            return buffer;
        }
        return this->createQuadIndexBuffer();
    }

    static int QuadCountOfQuadBuffer();

    /**
     * Factories for GrPath and GrPathRange objects. It's an error to call these if path rendering
     * is not supported.
     */
    sk_sp<GrPath> createPath(const SkPath&, const GrStyle&);
    sk_sp<GrPathRange> createPathRange(GrPathRange::PathGenerator*, const GrStyle&);
    sk_sp<GrPathRange> createGlyphs(const SkTypeface*, const SkScalerContextEffects&,
                                    const SkDescriptor*, const GrStyle&);

    /** These flags govern which scratch resources we are allowed to return */
    enum Flags {
        /** If the caller intends to do direct reads/writes to/from the CPU then this flag must be
         *  set when accessing resources during a GrOpList flush. This includes the execution of
         *  GrOp objects. The reason is that these memory operations are done immediately and
         *  will occur out of order WRT the operations being flushed.
         *  Make this automatic: https://bug.skia.org/4156
         */
        kNoPendingIO_Flag     = 0x1,

        /** Normally the caps may indicate a preference for client-side buffers. Set this flag when
         *  creating a buffer to guarantee it resides in GPU memory.
         */
        kRequireGpuMemory_Flag = 0x2,
    };

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
    GrBuffer* createBuffer(size_t size, GrBufferType intendedType, GrAccessPattern, uint32_t flags,
                           const void* data = nullptr);


    /**
     * If passed in render target already has a stencil buffer, return true. Otherwise attempt to
     * attach one and return true on success.
     */
    bool attachStencilAttachment(GrRenderTarget* rt);

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
                                                            int sampleCnt);

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

    // Takes the GrSemaphore and sets the ownership of the semaphore to the GrGpu object used by
    // this class. This call is only used when passing a GrSemaphore from one context to another.
    void takeOwnershipOfSemaphore(sk_sp<GrSemaphore>);
    // Takes the GrSemaphore and resets the ownership of the semaphore so that it is not owned by
    // any GrGpu. A follow up call to takeOwnershipofSemaphore must be made so that the underlying
    // semaphore can be deleted. This call is only used when passing a GrSemaphore from one context
    // to another.
    void releaseOwnershipOfSemaphore(sk_sp<GrSemaphore>);

    void abandon() {
        fCache = nullptr;
        fGpu = nullptr;
    }

    const GrCaps* caps() const { return fCaps.get(); }
    bool overBudget() const { return fCache->overBudget(); }

    inline GrResourceProviderPriv priv();
    inline const GrResourceProviderPriv priv() const;

private:
    sk_sp<GrGpuResource> findResourceByUniqueKey(const GrUniqueKey&);

    // Attempts to find a resource in the cache that exactly matches the GrSurfaceDesc. Failing that
    // it returns null. If non-null, the resulting texture is always budgeted.
    sk_sp<GrTexture> refScratchTexture(const GrSurfaceDesc&, uint32_t scratchTextureFlags);

    /*
     * Try to find an existing scratch texture that exactly matches 'desc'. If successful
     * update the budgeting accordingly.
     */
    sk_sp<GrTexture> getExactScratch(const GrSurfaceDesc&, SkBudgeted, uint32_t flags);

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

    sk_sp<const GrBuffer> createPatternedIndexBuffer(const uint16_t* pattern,
                                                     int patternSize,
                                                     int reps,
                                                     int vertCount,
                                                     const GrUniqueKey& key);

    sk_sp<const GrBuffer> createQuadIndexBuffer();

    GrResourceCache*    fCache;
    GrGpu*              fGpu;
    sk_sp<const GrCaps> fCaps;
    GrUniqueKey         fQuadIndexBufferKey;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

#endif
