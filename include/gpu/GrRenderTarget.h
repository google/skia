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
#include "GrSurface.h"

class GrStencilBuffer;
class GrTexture;

/**
 * GrRenderTarget represents a 2D buffer of pixels that can be rendered to.
 * A context's render target is set by setRenderTarget(). Render targets are
 * created by a createTexture with the kRenderTarget_TextureFlag flag.
 * Additionally, GrContext provides methods for creating GrRenderTargets
 * that wrap externally created render targets.
 */
class GrRenderTarget : public GrSurface {
public:
    SK_DECLARE_INST_COUNT(GrRenderTarget)

    // GrResource overrides
    virtual size_t sizeInBytes() const SK_OVERRIDE;

    // GrSurface overrides
    /**
     * @return the texture associated with the rendertarget, may be NULL.
     */
    virtual GrTexture* asTexture() SK_OVERRIDE { return fTexture; }
    virtual const GrTexture* asTexture() const SK_OVERRIDE { return fTexture; }

    /**
     * @return this render target.
     */
    virtual GrRenderTarget* asRenderTarget() SK_OVERRIDE { return this; }
    virtual const GrRenderTarget* asRenderTarget() const  SK_OVERRIDE {
        return this;
    }

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

    // GrRenderTarget
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
     * @return true if the surface is multisampled, false otherwise
     */
    bool isMultisampled() const { return 0 != fDesc.fSampleCnt; }

    /**
     * @return the number of samples-per-pixel or zero if non-MSAA.
     */
    int numSamples() const { return fDesc.fSampleCnt; }

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
                   const GrTextureDesc& desc)
        : INHERITED(gpu, desc)
        , fStencilBuffer(NULL)
        , fTexture(texture) {
        fResolveRect.setLargestInverted();
    }

    friend class GrTexture;
    // When a texture unrefs an owned rendertarget this func
    // removes the back pointer. This could be called from
    // texture's destructor but would have to be done in derived
    // classes. By the time of texture base destructor it has already
    // lost its pointer to the rt.
    void onTextureReleaseRenderTarget() {
        GrAssert(NULL != fTexture);
        fTexture = NULL;
    }

    // override of GrResource
    virtual void onAbandon() SK_OVERRIDE;
    virtual void onRelease() SK_OVERRIDE;

private:
    GrStencilBuffer*  fStencilBuffer;
    GrTexture*        fTexture; // not ref'ed

    GrIRect           fResolveRect;

    typedef GrSurface INHERITED;
};

#endif
