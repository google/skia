/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrSurface_DEFINED
#define GrSurface_DEFINED

#include "GrTypes.h"
#include "GrGpuResource.h"
#include "SkImageInfo.h"
#include "SkRect.h"

class GrRenderTarget;
class GrSurfacePriv;
class GrTexture;

class SK_API GrSurface : public GrGpuResource {
public:
    /**
     * Retrieves the width of the surface.
     */
    int width() const { return fDesc.fWidth; }

    /**
     * Retrieves the height of the surface.
     */
    int height() const { return fDesc.fHeight; }

    /**
     * Helper that gets the width and height of the surface as a bounding rectangle.
     */
    void getBoundsRect(SkRect* rect) const { rect->setWH(SkIntToScalar(this->width()),
                                                         SkIntToScalar(this->height())); }

    GrSurfaceOrigin origin() const {
        SkASSERT(kTopLeft_GrSurfaceOrigin == fDesc.fOrigin || kBottomLeft_GrSurfaceOrigin == fDesc.fOrigin);
        return fDesc.fOrigin;
    }

    /**
     * Retrieves the pixel config specified when the surface was created.
     * For render targets this can be kUnknown_GrPixelConfig
     * if client asked us to render to a target that has a pixel
     * config that isn't equivalent with one of our configs.
     */
    GrPixelConfig config() const { return fDesc.fConfig; }

    /**
     * Return the descriptor describing the surface
     */
    const GrSurfaceDesc& desc() const { return fDesc; }

    /**
     * @return the texture associated with the surface, may be NULL.
     */
    virtual GrTexture* asTexture() { return NULL; }
    virtual const GrTexture* asTexture() const { return NULL; }

    /**
     * @return the render target underlying this surface, may be NULL.
     */
    virtual GrRenderTarget* asRenderTarget() { return NULL; }
    virtual const GrRenderTarget* asRenderTarget() const { return NULL; }

    /**
     * Reads a rectangle of pixels from the surface.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags See the GrContext::PixelOpsFlags enum.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an unsupported
     *              pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config,
                    void* buffer,
                    size_t rowBytes = 0,
                    uint32_t pixelOpsFlags = 0);

    /**
     * Copy the src pixels [buffer, rowbytes, pixelconfig] into the surface at the specified
     * rectangle.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read the rectangle from.
     * @param rowBytes      number of bytes between consecutive rows. Zero means rows are tightly
     *                      packed.
     * @param pixelOpsFlags See the GrContext::PixelOpsFlags enum.
     *
     * @return true if the read succeeded, false if not. The read can fail because of an
     *              unsupported pixel config.
     */
    bool writePixels(int left, int top, int width, int height,
                     GrPixelConfig config,
                     const void* buffer,
                     size_t rowBytes = 0,
                     uint32_t pixelOpsFlags = 0);

    /**
     * After this returns any pending writes to the surface will be issued to the backend 3D API.
     */
    void flushWrites();


    /**
     * After this returns any pending surface IO will be issued to the backend 3D API and
     * if the surface has MSAA it will be resolved.
     */
    void prepareForExternalIO();

    /** Access methods that are only to be used within Skia code. */
    inline GrSurfacePriv surfacePriv();
    inline const GrSurfacePriv surfacePriv() const;

    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    void setRelease(ReleaseProc proc, ReleaseCtx ctx) {
        fReleaseProc = proc;
        fReleaseCtx = ctx;
    }

    static size_t WorseCaseSize(const GrSurfaceDesc& desc);

protected:
    // Methods made available via GrSurfacePriv
    SkImageInfo info(SkAlphaType) const;
    bool savePixels(const char* filename);
    bool hasPendingRead() const;
    bool hasPendingWrite() const;
    bool hasPendingIO() const;

    // Provides access to methods that should be public within Skia code.
    friend class GrSurfacePriv;

    GrSurface(GrGpu* gpu, LifeCycle lifeCycle, const GrSurfaceDesc& desc)
        : INHERITED(gpu, lifeCycle)
        , fDesc(desc)
        , fReleaseProc(NULL)
        , fReleaseCtx(NULL)
    {}

    ~GrSurface() override {
        // check that invokeReleaseProc has been called (if needed)
        SkASSERT(NULL == fReleaseProc);
    }

    GrSurfaceDesc fDesc;

    void onRelease() override;
    void onAbandon() override;

private:
    void invokeReleaseProc() {
        if (fReleaseProc) {
            fReleaseProc(fReleaseCtx);
            fReleaseProc = NULL;
        }
    }

    ReleaseProc fReleaseProc;
    ReleaseCtx  fReleaseCtx;

    typedef GrGpuResource INHERITED;
};

#endif
