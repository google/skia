/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrRefCnt.h"
#include "GrClip.h"
#include "GrResource.h"

class GrTexture;

/**
 * GrRenderTarget represents a 2D buffer of pixels that can be rendered to.
 * A context's render target is set by setRenderTarget(). Render targets are
 * created by a createTexture with the kRenderTarget_TextureFlag flag.
 * Additionally, GrContext provides methods for creating GrRenderTargets
 * that wrap externally created render targets.
 */
class GrRenderTarget : public GrResource {
public:
    /**
     * @return the width of the rendertarget
     */
    int width() const { return fWidth; }
    /**
     * @return the height of the rendertarget
     */
    int height() const { return fHeight; }

    /**
     * @return the number of stencil bits in the rendertarget
     */
    int stencilBits() const { return fStencilBits; }

    /**
     * @return the texture associated with the rendertarget, may be NULL.
     */
    GrTexture* asTexture() {return fTexture;}

    /**
     * Reads a rectangle of pixels from the render target.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer);

protected:
    GrRenderTarget(GrGpu* gpu,
                   GrTexture* texture,
                   int width,
                   int height,
                   int stencilBits)
        : INHERITED(gpu)
        , fTexture(texture)
        , fWidth(width)
        , fHeight(height)
        , fStencilBits(stencilBits)
    {}


    GrTexture* fTexture;
    int        fWidth;
    int        fHeight;
    int        fStencilBits;

private:
    // GrGpu keeps a cached clip in the render target to avoid redundantly
    // rendering the clip into the same stencil buffer.
    friend class GrGpu;
    GrClip     fLastStencilClip;

    typedef GrResource INHERITED;
};

class GrTexture : public GrResource {
protected:
    GrTexture(GrGpu* gpu,
              int width,
              int height,
              GrPixelConfig config)
    : INHERITED(gpu)
    , fWidth(width)
    , fHeight(height)
    , fConfig(config) {
        // only make sense if alloc size is pow2
        fShiftFixedX = 31 - Gr_clz(fWidth);
        fShiftFixedY = 31 - Gr_clz(fHeight);
    }

public:
    virtual ~GrTexture();

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
    size_t sizeInBytes() const {
        return fWidth * fHeight * GrBytesPerPixel(fConfig);
    }

    /**
     * Updates a subrectangle of texels in the texture.
     *
     * @param x       left edge of rectangle to update
     * @param y       top edge of rectangle to update
     * @param width   width of rectangle to update
     * @param height  height of rectangle to update
     * @param srcData width*height texels of data in same format that was used
     *                at texture creation.
     */
    virtual void uploadTextureData(uint32_t x,
                                   uint32_t y,
                                   uint32_t width,
                                   uint32_t height,
                                   const void* srcData) = 0;

    /**
     * Reads a rectangle of pixels from the texture.
     * @param left          left edge of the rectangle to read (inclusive)
     * @param top           top edge of the rectangle to read (inclusive)
     * @param width         width of rectangle to read in pixels.
     * @param height        height of rectangle to read in pixels.
     * @param config        the pixel config of the destination buffer
     * @param buffer        memory to read the rectangle into.
     *
     * @return true if the read succeeded, false if not. The read can fail
     *              because of a unsupported pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer);

    /**
     * Retrieves the render target underlying this texture that can be passed to
     * GrGpu::setRenderTarget().
     *
     * @return    handle to render target or undefined if the texture is not a
     *            render target
     */
    virtual GrRenderTarget* asRenderTarget() = 0;

    /**
     * Removes the reference on the associated GrRenderTarget held by this
     * texture. Afterwards asRenderTarget() will return NULL. The
     * GrRenderTarget survives the release if another ref is held on it.
     */
    virtual void releaseRenderTarget() = 0;

    /**
     *  Return the native ID or handle to the texture, depending on the
     *  platform. e.g. on opengl, return the texture ID.
     */
    virtual intptr_t getTextureHandle() = 0;

#if GR_DEBUG
    void validate() const {
        this->INHERITED::validate();
    }
#else
    void validate() const {}
#endif

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

