/*
    Copyright 2010 Google Inc.

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

class GrTexture;

/**
 * GrRenderTarget represents a 2D buffer of pixels that can be rendered to.
 * A context's render target is set by setRenderTarget(). Render targets are
 * created by a createTexture with the kRenderTarget_TextureFlag flag. 
 * Additionally, the rendering destination set in the underlying 3D API at the
 * time of GrContext's creation can be retrieved by calling 
 * currentRenderTarget() after creation before any calles to setRenderTarget().
 */
class GrRenderTarget : public GrRefCnt {
public:
    /**
     * @return the width of the rendertarget
     */
    virtual uint32_t width() const = 0;
    /**
     * @return the height of the rendertarget
     */
    virtual uint32_t height() const = 0;
    
    /**
     * @return the texture associated with the rendertarget, may be NULL.
     */
    GrTexture* asTexture() {return fTexture;}

protected:
    GrRenderTarget(GrTexture* texture) : fTexture(texture) {}
    GrTexture* fTexture;
};

class GrTexture : public GrRefCnt {
public:
    enum PixelConfig {
        kUnknown_PixelConfig,
        kAlpha_8_PixelConfig,
        kIndex_8_PixelConfig,
        kRGB_565_PixelConfig,
        kRGBA_4444_PixelConfig, //!< premultiplied
        kRGBA_8888_PixelConfig, //!< premultiplied
        kRGBX_8888_PixelConfig, //!< treat the alpha channel as opaque
    };
    static size_t BytesPerPixel(PixelConfig);
    static bool PixelConfigIsOpaque(PixelConfig);

protected:
    GrTexture(uint32_t contentWidth,
              uint32_t contentHeight,
              uint32_t allocWidth,
              uint32_t allocHeight,
              PixelConfig config) : 
                fAllocWidth(allocWidth), 
                fAllocHeight(allocHeight),
                fContentWidth(contentWidth), 
                fContentHeight(contentHeight),
                fConfig(config) {
                    // only make sense if alloc size is pow2
                    fShiftFixedX = 31 - Gr_clz(allocWidth);
                    fShiftFixedY = 31 - Gr_clz(allocHeight);
                }
public:
    virtual ~GrTexture();
    
    /**
     * Retrieves the width of the content area of the texture. Reflects the
     * width passed to GrGpu::createTexture().
     * 
     * @return the width in texels
     */
    uint32_t contentWidth() const { return fContentWidth; }
    /**
     * Retrieves the height of the content area of the texture. Reflects the
     * height passed to GrGpu::createTexture().
     * 
     * @return the height in texels
     */
    uint32_t contentHeight() const { return fContentHeight; }

    /**
     * Retrieves the texture width actually allocated in texels.
     *
     * @return the width in texels
     */
    uint32_t allocWidth() const { return fAllocWidth; }
    /**
     * Retrieves the texture height actually allocated in texels.
     *
     * @return the height in texels
     */
    uint32_t allocHeight() const { return fAllocHeight; }

    /**
     * Convert from texels to normalized texture coords for POT textures
     * only.
     */
    GrFixed normalizeFixedX(GrFixed x) const { GrAssert(GrIsPow2(fAllocWidth));
                                               return x >> fShiftFixedX; }
    GrFixed normalizeFixedY(GrFixed y) const { GrAssert(GrIsPow2(fAllocHeight));
                                               return y >> fShiftFixedY; }

    /** 
     * Retrieves the pixel config specified when the texture was created.
     */
    PixelConfig config() const { return fConfig; }

    /**
     *  The number of bytes used by the texture
     */
    size_t sizeInBytes() const {
        return fAllocWidth * fAllocHeight * BytesPerPixel(fConfig);
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
     * Indicates that GPU context in which this texture was created is destroyed
     * and that Ganesh should not attempt to free the texture with the 
     * underlying API.
     */
    virtual void abandon() = 0;

    /**
     * Queries whether the texture was created as a render target.
     *
     * Use asRenderTarget() to use the texture as a render target if this 
     * returns true.
     *
     * @return true if the texture was created as a render target.
     */
    virtual bool isRenderTarget() const = 0;

    /**
     * Retrieves the render target underlying this texture that can be passed to
     * GrGpu::setRenderTarget().
     *
     * If isRenderTarget() is false then the returned handle is undefined.
     *
     * @return    handle to render target or undefined if the texture is not a
     *            render target
     */
    virtual GrRenderTarget* asRenderTarget() = 0;

    /**
     * Removes the "rendertargetness" from a texture. This may or may not
     * actually do anything with the underlying 3D API.
     */
    virtual void removeRenderTarget() = 0;

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
    uint32_t fAllocWidth;
    uint32_t fAllocHeight;
    uint32_t fContentWidth;
    uint32_t fContentHeight;
    // these two shift a fixed-point value into normalized coordinates
    // for this texture if the texture is power of two sized.
    int      fShiftFixedX;
    int      fShiftFixedY;
    PixelConfig fConfig;

    typedef GrRefCnt INHERITED;
};

#endif

