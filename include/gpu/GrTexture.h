
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrResource.h"

class GrRenderTarget;

class GrTexture : public GrResource {

public:
    /**
     * Retrieves the width of the texture.
     *
     * @return the width in texels
     */
    int width() const { return fWidth; }

    /**
     * Retrieves the height of the texture.
     *
     * @return the height in texels
     */
    int height() const { return fHeight; }

    /**
     * Convert from texels to normalized texture coords for POT textures
     * only.
     */
    GrFixed normalizeFixedX(GrFixed x) const { GrAssert(GrIsPow2(fWidth));
                                               return x >> fShiftFixedX; }
    GrFixed normalizeFixedY(GrFixed y) const { GrAssert(GrIsPow2(fHeight));
                                               return y >> fShiftFixedY; }

    /**
     * Retrieves the pixel config specified when the texture was created.
     */
    GrPixelConfig config() const { return fConfig; }

    /**
     *  Approximate number of bytes used by the texture
     */
    virtual size_t sizeInBytes() const {
        return (size_t) fWidth * fHeight * GrBytesPerPixel(fConfig);
    }

    /**
     * Read a rectangle of pixels from the texture.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero
     *                      means rows are tightly packed.
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer,
                    size_t rowBytes);

    /**
     * Writes a rectangle of pixels to the texture.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read pixels from
     * @param rowBytes      number of bytes between consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    void writePixels(int left, int top, int width, int height,
                     GrPixelConfig config, const void* buffer,
                     size_t rowBytes);

    /**
     * Retrieves the render target underlying this texture that can be passed to
     * GrGpu::setRenderTarget().
     *
     * @return    handle to render target or NULL if the texture is not a
     *            render target
     */
    GrRenderTarget* asRenderTarget() { return fRenderTarget; }
    const GrRenderTarget* asRenderTarget() const { return fRenderTarget; }

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

#if GR_DEBUG
    void validate() const {
        this->INHERITED::validate();
    }
#else
    void validate() const {}
#endif

protected:
    GrRenderTarget* fRenderTarget; // texture refs its rt representation
                                   // base class cons sets to NULL
                                   // subclass cons can create and set

    GrTexture(GrGpu* gpu,
              int width,
              int height,
              GrPixelConfig config)
    : INHERITED(gpu)
    , fRenderTarget(NULL)
    , fWidth(width)
    , fHeight(height)
    , fConfig(config) {
        // only make sense if alloc size is pow2
        fShiftFixedX = 31 - Gr_clz(fWidth);
        fShiftFixedY = 31 - Gr_clz(fHeight);
    }

    // GrResource overrides
    virtual void onRelease() {
        this->releaseRenderTarget();
    }

    virtual void onAbandon();

private:
    int fWidth;
    int fHeight;

    // these two shift a fixed-point value into normalized coordinates
    // for this texture if the texture is power of two sized.
    int      fShiftFixedX;
    int      fShiftFixedY;

    GrPixelConfig fConfig;

    typedef GrResource INHERITED;
};

#endif

