/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTarget_DEFINED
#define GrRenderTarget_DEFINED

#include "GrSurface.h"
#include "SkRect.h"

class GrStencilAttachment;
class GrRenderTargetPriv;

/**
 * GrRenderTarget represents a 2D buffer of pixels that can be rendered to.
 * A context's render target is set by setRenderTarget(). Render targets are
 * created by a createTexture with the kRenderTarget_SurfaceFlag flag.
 * Additionally, GrContext provides methods for creating GrRenderTargets
 * that wrap externally created render targets.
 */
class GrRenderTarget : virtual public GrSurface {
public:
    // GrSurface overrides
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrRenderTarget* asRenderTarget() const  override { return this; }

    // GrRenderTarget
    /**
     * On some hardware it is possible for a render target to have multisampling
     * only in certain buffers.
     * Enforce only two legal sample configs.
     * kUnified_SampleConfig signifies multisampling in both color and stencil
     * buffers and is available across all hardware.
     * kStencil_SampleConfig means multisampling is present in stencil buffer
     * only; this config requires hardware support of
     * NV_framebuffer_mixed_samples.
    */
    enum SampleConfig {
        kUnified_SampleConfig = 0,
        kStencil_SampleConfig = 1
    };

    /**
     * @return true if the surface is multisampled in all buffers,
     *         false otherwise
     */
    bool isUnifiedMultisampled() const {
        if (fSampleConfig != kUnified_SampleConfig) {
            return false;
        }
        return 0 != fDesc.fSampleCnt;
    }

    /**
     * @return true if the surface is multisampled in the stencil buffer,
     *         false otherwise
     */
    bool isStencilBufferMultisampled() const {
        return 0 != fDesc.fSampleCnt;
    }

    /**
     * @return the number of color samples-per-pixel, or zero if non-MSAA or
     *         multisampled in the stencil buffer only.
     */
    int numColorSamples() const {
        if (fSampleConfig == kUnified_SampleConfig) {
            return fDesc.fSampleCnt;
        }
        return 0;
    }

    /**
     * @return the number of stencil samples-per-pixel, or zero if non-MSAA.
     */
    int numStencilSamples() const {
        return fDesc.fSampleCnt;
    }

    /**
     * @return true if the surface is mixed sampled, false otherwise.
     */
    bool hasMixedSamples() const {
        SkASSERT(kStencil_SampleConfig != fSampleConfig ||
                 this->isStencilBufferMultisampled());
        return kStencil_SampleConfig == fSampleConfig;
    }

    /**
     * Call to indicate the multisample contents were modified such that the
     * render target needs to be resolved before it can be used as texture. Gr
     * tracks this for its own drawing and thus this only needs to be called
     * when the render target has been modified outside of Gr. This has no
     * effect on wrapped backend render targets.
     *
     * @param rect  a rect bounding the area needing resolve. NULL indicates
     *              the whole RT needs resolving.
     */
    void flagAsNeedingResolve(const SkIRect* rect = NULL);

    /**
     * Call to override the region that needs to be resolved.
     */
    void overrideResolveRect(const SkIRect rect);

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
    const SkIRect& getResolveRect() const { return fResolveRect; }

    /**
     * Provide a performance hint that the render target's contents are allowed
     * to become undefined.
     */
    void discard();

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
     *  Return the native ID or handle to the rendertarget, depending on the
     *  platform. e.g. on OpenGL, return the FBO ID.
     */
    virtual GrBackendObject getRenderTargetHandle() const = 0;

    // Provides access to functions that aren't part of the public API.
    GrRenderTargetPriv renderTargetPriv();
    const GrRenderTargetPriv renderTargetPriv() const;

protected:
    GrRenderTarget(GrGpu* gpu, LifeCycle lifeCycle, const GrSurfaceDesc& desc,
                   SampleConfig sampleConfig)
        : INHERITED(gpu, lifeCycle, desc)
        , fStencilAttachment(NULL)
        , fSampleConfig(sampleConfig) {
        fResolveRect.setLargestInverted();
    }

    // override of GrResource
    void onAbandon() override;
    void onRelease() override;

private:
    // Checked when this object is asked to attach a stencil buffer.
    virtual bool canAttemptStencilAttachment() const = 0;

    friend class GrRenderTargetPriv;

    GrStencilAttachment*  fStencilAttachment;
    SampleConfig          fSampleConfig;

    SkIRect               fResolveRect;

    typedef GrSurface INHERITED;
};


#endif
