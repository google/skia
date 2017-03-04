/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "GrRenderTarget.h"
#include "GrSurfaceProxy.h"
#include "GrTypes.h"

class GrResourceProvider;

// This class delays the acquisition of RenderTargets until they are actually
// required
// Beware: the uniqueID of the RenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget it represents!
class GrRenderTargetProxy : virtual public GrSurfaceProxy {
public:
    GrRenderTargetProxy* asRenderTargetProxy() override { return this; }
    const GrRenderTargetProxy* asRenderTargetProxy() const override { return this; }

    // Actually instantiate the backing rendertarget, if necessary.
    GrRenderTarget* instantiate(GrResourceProvider* resourceProvider);

    bool isStencilBufferMultisampled() const { return fDesc.fSampleCnt > 0; }

    /**
     * For our purposes, "Mixed Sampled" means the stencil buffer is multisampled but the color
     * buffer is not.
     */
    bool isMixedSampled() const { return fRenderTargetFlags & GrRenderTarget::Flags::kMixedSampled; }

    /**
     * "Unified Sampled" means the stencil and color buffers are both multisampled.
     */
    bool isUnifiedMultisampled() const { return fDesc.fSampleCnt > 0 && !this->isMixedSampled(); }

    /**
     * Returns the number of samples/pixel in the stencil buffer (Zero if non-MSAA).
     */
    int numStencilSamples() const { return fDesc.fSampleCnt; }

    /**
     * Returns the number of samples/pixel in the color buffer (Zero if non-MSAA or mixed sampled).
     */
    int numColorSamples() const { return this->isMixedSampled() ? 0 : fDesc.fSampleCnt; }

    int maxWindowRectangles(const GrCaps& caps) const;

    GrRenderTarget::Flags testingOnly_getFlags() const;

    // TODO: move this to a priv class!
    bool refsWrappedObjects() const;

protected:
    friend class GrSurfaceProxy;  // for ctors

    // Deferred version
    GrRenderTargetProxy(const GrCaps&, const GrSurfaceDesc&,
                        SkBackingFit, SkBudgeted, uint32_t flags);

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrSurface>);

private:
    size_t onGpuMemorySize() const override;

    // For wrapped render targets the actual GrRenderTarget is stored in the GrIORefProxy class.
    // For deferred proxies that pointer is filled in when we need to instantiate the
    // deferred resource.

    // These don't usually get computed until the render target is instantiated, but the render
    // target proxy may need to answer queries about it before then. And since in the deferred case
    // we know the newly created render target will be internal, we are able to precompute what the
    // flags will ultimately end up being. In the wrapped case we just copy the wrapped
    // rendertarget's info here.
    GrRenderTarget::Flags   fRenderTargetFlags;

    typedef GrSurfaceProxy INHERITED;
};

#endif
