/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResourceProvider_DEFINED
#define GrResourceProvider_DEFINED

#include "GrBuffer.h"
#include "GrGpu.h"
#include "GrPathRange.h"

class GrPath;
class GrRenderTarget;
class GrSingleOwner;
class GrStencilAttachment;
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

    template <typename T> T* findAndRefTByUniqueKey(const GrUniqueKey& key) {
        return static_cast<T*>(this->findAndRefResourceByUniqueKey(key));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Textures

    /**
     * Creates a new texture in the resource cache and returns it. The caller owns a
     * ref on the returned texture which must be balanced by a call to unref.
     *
     * @param desc          Description of the texture properties.
     * @param budgeted      Does the texture count against the resource cache budget?
     * @param texels        A contiguous array of mipmap levels
     * @param mipLevelCount The amount of elements in the texels array
     */
    sk_sp<GrTextureProxy> createMipMappedTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                 const GrMipLevel* texels, int mipLevelCount,
                                                 uint32_t flags = 0,
                                                 SkDestinationSurfaceColorMode mipColorMode =
                                                        SkDestinationSurfaceColorMode::kLegacy);

    /** Assigns a unique key to the texture. The texture will be findable via this key using
    findTextureByUniqueKey(). If an existing texture has this key, it's key will be removed. */
    void assignUniqueKeyToProxy(const GrUniqueKey& key, GrTextureProxy*);

    /** Finds a texture by unique key. If the texture is found it is ref'ed and returned. */
    sk_sp<GrTextureProxy> findProxyByUniqueKey(const GrUniqueKey& key);

    /**
     * Finds a texture that approximately matches the descriptor. Will be at least as large in width
     * and height as desc specifies. If desc specifies that the texture should be a render target
     * then result will be a render target. Format and sample count will always match the request.
     * The contents of the texture are undefined. The caller owns a ref on the returned texture and
     * must balance with a call to unref.
     */
    GrTexture* createApproxTexture(const GrSurfaceDesc&, uint32_t flags);

    /** Create an exact fit texture with no initial data to upload.
     */
    sk_sp<GrTexture> createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                   uint32_t flags = 0);

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
    sk_sp<GrTexture> wrapBackendTexture(const GrBackendTextureDesc& desc,
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
    sk_sp<GrRenderTarget> wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc);

    static const int kMinScratchTextureSize;

    /**
     * Either finds and refs, or creates an index buffer for instanced drawing with a specific
     * pattern if the index buffer is not found. If the return is non-null, the caller owns
     * a ref on the returned GrBuffer.
     *
     * @param pattern     the pattern of indices to repeat
     * @param patternSize size in bytes of the pattern
     * @param reps        number of times to repeat the pattern
     * @param vertCount   number of vertices the pattern references
     * @param key         Key to be assigned to the index buffer.
     *
     * @return The index buffer if successful, otherwise nullptr.
     */
    const GrBuffer* findOrCreateInstancedIndexBuffer(const uint16_t* pattern,
                                                     int patternSize,
                                                     int reps,
                                                     int vertCount,
                                                     const GrUniqueKey& key) {
        if (GrBuffer* buffer = this->findAndRefTByUniqueKey<GrBuffer>(key)) {
            return buffer;
        }
        return this->createInstancedIndexBuffer(pattern, patternSize, reps, vertCount, key);
    }

    /**
     * Returns an index buffer that can be used to render quads.
     * Six indices per quad: 0, 1, 2, 0, 2, 3, etc.
     * The max number of quads is the buffer's index capacity divided by 6.
     * Draw with kTriangles_GrPrimitiveType
     * @ return the quad index buffer
     */
    const GrBuffer* refQuadIndexBuffer() {
        if (GrBuffer* buffer =
            this->findAndRefTByUniqueKey<GrBuffer>(fQuadIndexBufferKey)) {
            return buffer;
        }
        return this->createQuadIndexBuffer();
    }

    /**
     * Factories for GrPath and GrPathRange objects. It's an error to call these if path rendering
     * is not supported.
     */
    GrPath* createPath(const SkPath&, const GrStyle&);
    GrPathRange* createPathRange(GrPathRange::PathGenerator*, const GrStyle&);
    GrPathRange* createGlyphs(const SkTypeface*, const SkScalerContextEffects&,
                              const SkDescriptor*, const GrStyle&);

    /** These flags govern which scratch resources we are allowed to return */
    enum Flags {
        kExact_Flag           = 0x1,

        /** If the caller intends to do direct reads/writes to/from the CPU then this flag must be
         *  set when accessing resources during a GrOpList flush. This includes the execution of
         *  GrOp objects. The reason is that these memory operations are done immediately and
         *  will occur out of order WRT the operations being flushed.
         *  Make this automatic: https://bug.skia.org/4156
         */
        kNoPendingIO_Flag     = 0x2,

        kNoCreate_Flag        = 0x4,

        /** Normally the caps may indicate a preference for client-side buffers. Set this flag when
         *  creating a buffer to guarantee it resides in GPU memory.
         */
        kRequireGpuMemory_Flag = 0x8,
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
     * If passed in render target already has a stencil buffer, return it. Otherwise attempt to
     * attach one.
     */
    GrStencilAttachment* attachStencilAttachment(GrRenderTarget* rt);

     /**
      * Wraps an existing texture with a GrRenderTarget object. This is useful when the provided
      * texture has a format that cannot be textured from by Skia, but we want to raster to it.
      *
      * The texture is wrapped as borrowed. The texture object will not be freed once the
      * render target is destroyed.
      *
      * @return GrRenderTarget object or NULL on failure.
      */
     sk_sp<GrRenderTarget> wrapBackendTextureAsRenderTarget(const GrBackendTextureDesc& desc);

    /**
     * Assigns a unique key to a resource. If the key is associated with another resource that
     * association is removed and replaced by this resource.
     */
    void assignUniqueKeyToResource(const GrUniqueKey&, GrGpuResource*);

    /**
     * Finds a resource in the cache, based on the specified key. This is intended for use in
     * conjunction with addResourceToCache(). The return value will be NULL if not found. The
     * caller must balance with a call to unref().
     */
    GrGpuResource* findAndRefResourceByUniqueKey(const GrUniqueKey&);

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore();

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

    // 'proxy' is about to be used as a texture src or drawn to. This query can be used to
    // determine if it is going to need a texture domain or a full clear.
    static bool IsFunctionallyExact(GrSurfaceProxy* proxy);

    const GrCaps* caps() const { return fCaps.get(); }

private:
    GrTexture* findAndRefTextureByUniqueKey(const GrUniqueKey& key);
    void assignUniqueKeyToTexture(const GrUniqueKey& key, GrTexture* texture) {
        SkASSERT(key.isValid());
        this->assignUniqueKeyToResource(key, texture);
    }

    GrTexture* refScratchTexture(const GrSurfaceDesc&, uint32_t scratchTextureFlags);

    GrResourceCache* cache() { return fCache; }
    const GrResourceCache* cache() const { return fCache; }

    GrGpu* gpu() { return fGpu; }
    const GrGpu* gpu() const { return fGpu; }

    bool isAbandoned() const {
        SkASSERT(SkToBool(fGpu) == SkToBool(fCache));
        return !SkToBool(fCache);
    }

    const GrBuffer* createInstancedIndexBuffer(const uint16_t* pattern,
                                               int patternSize,
                                               int reps,
                                               int vertCount,
                                               const GrUniqueKey& key);

    const GrBuffer* createQuadIndexBuffer();

    GrResourceCache*    fCache;
    GrGpu*              fGpu;
    sk_sp<const GrCaps> fCaps;
    GrUniqueKey         fQuadIndexBufferKey;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

#endif
