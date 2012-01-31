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


#ifndef GrRenderTarget_DEFINED
#define GrRenderTarget_DEFINED

#include "GrRect.h"
#include "GrResource.h"

class GrStencilBuffer;
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
     * @return the pixel config. Can be kUnknown_GrPixelConfig
     * if client asked us to render to a target that has a pixel
     * config that isn't equivalent with one of our configs.
     */
    GrPixelConfig config() const { return fConfig; }

    /**
     * @return the texture associated with the rendertarget, may be NULL.
     */
    GrTexture* asTexture() {return fTexture;}

    /**
     * If this RT is multisampled, this is the multisample buffer
     * @return the 3D API's handle to this object (e.g. FBO ID in OpenGL)
     */
    virtual intptr_t getRenderTargetHandle() const = 0;

    /**
     * If this RT is multisampled, this is the buffer it is resolved to.
     * Otherwise, same as getRenderTargetHandle().
     * (In GL a separate FBO ID is used for the msaa and resolved buffers)
     * @return the 3D API's handle to this object (e.g. FBO ID in OpenGL)
     */
    virtual intptr_t getRenderTargetResolvedHandle() const = 0;

    /**
     * @return true if the render target is multisampled, false otherwise
     */
    bool isMultisampled() const { return 0 != fSampleCnt; }

    /**
     * @return the number of samples-per-pixel or zero if non-MSAA.
     */
    int numSamples() const { return fSampleCnt; }

    /**
     * Call to indicate the multisample contents were modified such that the
     * render target needs to be resolved before it can be used as texture. Gr
     * tracks this for its own drawing and thus this only needs to be called
     * when the render target has been modified outside of Gr. Only meaningful
     * for Gr-created RT/Textures and Platform RT/Textures created with the
     * kGrCanResolve flag.
     * @param rect  a rect bounding the area needing resolve. NULL indicates
     *              the whole RT needs resolving.
     */
    void flagAsNeedingResolve(const GrIRect* rect = NULL);

    /**
     * Call to override the region that needs to be resolved.
     */
    void overrideResolveRect(const GrIRect rect);

    /**
     * Call to indicate that GrRenderTarget was externally resolved. This may
     * allow Gr to skip a redundant resolve step.
     */
    void flagAsResolved() { fResolveRect.setLargestInverted(); }

    /**
     * @return true if the GrRenderTarget requires MSAA resolving
     */
    bool needsResolve() const { return !fResolveRect.isEmpty(); }

    /**
     * Returns a rect bounding the region needing resolving.
     */
    const GrIRect& getResolveRect() const { return fResolveRect; }

    /**
     * If the render target is multisampled this will perform a multisample
     * resolve. Any pending draws to the target are first flushed. This only
     * applies to render targets that are associated with GrTextures. After the
     * function returns the GrTexture will contain the resolved pixels.
     */
    void resolve();

    // GrResource overrides
    virtual size_t sizeInBytes() const;

    /**
     * Reads a rectangle of pixels from the render target.
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
     *              because of an unsupported pixel config.
     */
    bool readPixels(int left, int top, int width, int height,
                    GrPixelConfig config, void* buffer, size_t rowBytes);

    /**
     * Copy the src pixels [buffer, rowbytes, pixelconfig] into the render
     * target at the specified rectangle.
     * @param left          left edge of the rectangle to write (inclusive)
     * @param top           top edge of the rectangle to write (inclusive)
     * @param width         width of rectangle to write in pixels.
     * @param height        height of rectangle to write in pixels.
     * @param config        the pixel config of the source buffer
     * @param buffer        memory to read the rectangle from.
     * @param rowBytes      number of bytes bewtween consecutive rows. Zero
     *                      means rows are tightly packed.
     */
    void writePixels(int left, int top, int width, int height,
                     GrPixelConfig config, const void* buffer, size_t rowBytes);

    // a MSAA RT may require explicit resolving , it may auto-resolve (e.g. FBO
    // 0 in GL), or be unresolvable because the client didn't give us the 
    // resolve destination.
    enum ResolveType {
        kCanResolve_ResolveType,
        kAutoResolves_ResolveType,
        kCantResolve_ResolveType,
    };
    virtual ResolveType getResolveType() const = 0;

    /**
     * GrStencilBuffer is not part of the public API.
     */
    GrStencilBuffer* getStencilBuffer() const { return fStencilBuffer; }
    void setStencilBuffer(GrStencilBuffer* stencilBuffer);

protected:
    GrRenderTarget(GrGpu* gpu,
                   GrTexture* texture,
                   int width,
                   int height,
                   GrPixelConfig config,
                   int sampleCnt)
        : INHERITED(gpu)
        , fStencilBuffer(NULL)
        , fTexture(texture)
        , fWidth(width)
        , fHeight(height)
        , fConfig(config)
        , fSampleCnt(sampleCnt) {
        fResolveRect.setLargestInverted();
    }

    friend class GrTexture;
    // When a texture unrefs an owned rendertarget this func
    // removes the back pointer. This could be done called from 
    // texture's destructor but would have to be done in derived
    // class. By the time of texture base destructor it has already
    // lost its pointer to the rt.
    void onTextureReleaseRenderTarget() {
        GrAssert(NULL != fTexture);
        fTexture = NULL;
    }

private:
    GrStencilBuffer*  fStencilBuffer;
    GrTexture*        fTexture; // not ref'ed
    int               fWidth;
    int               fHeight;
    GrPixelConfig     fConfig;
    int               fSampleCnt;
    GrIRect           fResolveRect;

    typedef GrResource INHERITED;
};

#endif
