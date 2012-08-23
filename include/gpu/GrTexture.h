
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrSurface.h"
#include "GrCacheID.h"

class GrRenderTarget;
class GrResourceKey;
class GrTextureParams;

class GrTexture : public GrSurface {

public:
    SK_DECLARE_INST_COUNT(GrTexture)
    GR_DECLARE_RESOURCE_CACHE_TYPE()

    // from GrResource
    /**
     * Informational texture flags
     */
    enum FlagBits {
        kFirstBit = (kLastPublic_GrTextureFlagBit << 1),

        /**
         * This texture should be returned to the texture cache when
         * it is no longer reffed
         */
        kReturnToCache_FlagBit        = kFirstBit,
    };

    void setFlag(GrTextureFlags flags) {
        fDesc.fFlags = fDesc.fFlags | flags;
    }
    void resetFlag(GrTextureFlags flags) {
        fDesc.fFlags = fDesc.fFlags & ~flags;
    }
    bool isSetFlag(GrTextureFlags flags) const {
        return 0 != (fDesc.fFlags & flags);
    }

    /**
     *  Approximate number of bytes used by the texture
     */
    virtual size_t sizeInBytes() const SK_OVERRIDE {
        return (size_t) fDesc.fWidth *
                        fDesc.fHeight *
                        GrBytesPerPixel(fDesc.fConfig);
    }

    // GrSurface overrides
    virtual bool readPixels(int left, int top, int width, int height,
                            GrPixelConfig config,
                            void* buffer,
                            size_t rowBytes = 0,
                            uint32_t pixelOpsFlags = 0) SK_OVERRIDE;

    virtual void writePixels(int left, int top, int width, int height,
                             GrPixelConfig config,
                             const void* buffer,
                             size_t rowBytes = 0,
                             uint32_t pixelOpsFlags = 0) SK_OVERRIDE;

    /**
     * @return this texture
     */
    virtual GrTexture* asTexture() SK_OVERRIDE { return this; }
    virtual const GrTexture* asTexture() const SK_OVERRIDE { return this; }

    /**
     * Retrieves the render target underlying this texture that can be passed to
     * GrGpu::setRenderTarget().
     *
     * @return    handle to render target or NULL if the texture is not a
     *            render target
     */
    virtual GrRenderTarget* asRenderTarget() SK_OVERRIDE {
        return fRenderTarget;
    }
    virtual const GrRenderTarget* asRenderTarget() const SK_OVERRIDE {
        return fRenderTarget;
    }

    // GrTexture
    /**
     * Convert from texels to normalized texture coords for POT textures
     * only.
     */
    GrFixed normalizeFixedX(GrFixed x) const {
        GrAssert(GrIsPow2(fDesc.fWidth));
        return x >> fShiftFixedX;
    }
    GrFixed normalizeFixedY(GrFixed y) const {
        GrAssert(GrIsPow2(fDesc.fHeight));
        return y >> fShiftFixedY;
    }

    /**
     * Removes the reference on the associated GrRenderTarget held by this
     * texture. Afterwards asRenderTarget() will return NULL. The
     * GrRenderTarget survives the release if another ref is held on it.
     */
    void releaseRenderTarget();

    /**
     *  Return the native ID or handle to the texture, depending on the
     *  platform. e.g. on opengl, return the texture ID.
     */
    virtual intptr_t getTextureHandle() const = 0;

    /**
     *  Call this when the state of the native API texture object is
     *  altered directly, without being tracked by skia.
     */
    virtual void invalidateCachedState() = 0;

#if GR_DEBUG
    void validate() const {
        this->INHERITED::validate();

        this->validateDesc();
    }
#else
    void validate() const {}
#endif

    static GrResourceKey ComputeKey(const GrGpu* gpu,
                                    const GrTextureParams* sampler,
                                    const GrTextureDesc& desc,
                                    const GrCacheData& cacheData,
                                    bool scratch);

    static bool NeedsResizing(const GrResourceKey& key);
    static bool IsScratchTexture(const GrResourceKey& key);
    static bool NeedsFiltering(const GrResourceKey& key);

protected:
    GrRenderTarget* fRenderTarget; // texture refs its rt representation
                                   // base class cons sets to NULL
                                   // subclass cons can create and set

    GrTexture(GrGpu* gpu, const GrTextureDesc& desc)
    : INHERITED(gpu, desc)
    , fRenderTarget(NULL) {

        // only make sense if alloc size is pow2
        fShiftFixedX = 31 - Gr_clz(fDesc.fWidth);
        fShiftFixedY = 31 - Gr_clz(fDesc.fHeight);
    }

    // GrResource overrides
    virtual void onRelease() SK_OVERRIDE;
    virtual void onAbandon() SK_OVERRIDE;

    void validateDesc() const;

private:
    // these two shift a fixed-point value into normalized coordinates
    // for this texture if the texture is power of two sized.
    int                 fShiftFixedX;
    int                 fShiftFixedY;

    virtual void internal_dispose() const SK_OVERRIDE;

    typedef GrSurface INHERITED;
};

#endif

