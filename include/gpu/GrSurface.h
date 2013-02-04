/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrSurface_DEFINED
#define GrSurface_DEFINED

#include "GrTypes.h"
#include "GrResource.h"

class GrTexture;
class GrRenderTarget;

class GrSurface : public GrResource {
public:
    SK_DECLARE_INST_COUNT(GrSurface);

    /**
     * Retrieves the width of the surface.
     *
     * @return the width in texels
     */
    int width() const { return fDesc.fWidth; }

    /**
     * Retrieves the height of the surface.
     *
     * @return the height in texels
     */
    int height() const { return fDesc.fHeight; }

    GrSurfaceOrigin origin() const {
        GrAssert(kTopLeft_GrSurfaceOrigin == fDesc.fOrigin || kBottomLeft_GrSurfaceOrigin == fDesc.fOrigin);
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
    const GrTextureDesc& desc() const { return fDesc; }

    /**
     * @return the texture associated with the surface, may be NULL.
     */
    virtual GrTexture* asTexture() = 0;
    virtual const GrTexture* asTexture() const = 0;

    /**
     * @return the render target underlying this surface, may be NULL.
     */
    virtual GrRenderTarget* asRenderTarget() = 0;
    virtual const GrRenderTarget* asRenderTarget() const = 0;

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
    virtual bool readPixels(int left, int top, int width, int height,
                            GrPixelConfig config,
                            void* buffer,
                            size_t rowBytes = 0,
                            uint32_t pixelOpsFlags = 0) = 0;

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
     */
    virtual void writePixels(int left, int top, int width, int height,
                             GrPixelConfig config,
                             const void* buffer,
                             size_t rowBytes = 0,
                             uint32_t pixelOpsFlags = 0) = 0;

protected:
    GrSurface(GrGpu* gpu, bool isWrapped, const GrTextureDesc& desc)
    : INHERITED(gpu, isWrapped)
    , fDesc(desc) {
    }

    GrTextureDesc fDesc;

private:
    typedef GrResource INHERITED;
};

#endif // GrSurface_DEFINED
