/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfaceProxy.h"
#include "GrTypes.h"

class GrTextureProvider;

// This class delays the acquisition of RenderTargets until they are actually
// required
// Beware: the uniqueID of the RenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget it represents!
class GrRenderTargetProxy : public GrSurfaceProxy {
public:
    /**
     * The caller gets the creation ref.
     */
    static sk_sp<GrRenderTargetProxy> Make(const GrCaps&, const GrSurfaceDesc&,
                                           SkBackingFit, SkBudgeted);
    static sk_sp<GrRenderTargetProxy> Make(const GrCaps&, sk_sp<GrRenderTarget>);

    ~GrRenderTargetProxy() override;

    // TODO: add asTextureProxy variants
    GrRenderTargetProxy* asRenderTargetProxy() override { return this; }
    const GrRenderTargetProxy* asRenderTargetProxy() const override { return this; }

    // Actually instantiate the backing rendertarget, if necessary.
    GrRenderTarget* instantiate(GrTextureProvider* texProvider);

    bool isStencilBufferMultisampled() const { return fDesc.fSampleCnt > 0; }

    /**
     * For our purposes, "Mixed Sampled" means the stencil buffer is multisampled but the color
     * buffer is not.
     */
    bool isMixedSampled() const { return fFlags & GrRenderTargetPriv::Flags::kMixedSampled; }

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

    void setLastDrawTarget(GrDrawTarget* dt);
    GrDrawTarget* getLastDrawTarget() { return fLastDrawTarget; }

    GrRenderTargetPriv::Flags testingOnly_getFlags() const;

private:
    // Deferred version
    GrRenderTargetProxy(const GrCaps&, const GrSurfaceDesc&, SkBackingFit, SkBudgeted);

    // Wrapped version
    GrRenderTargetProxy(const GrCaps&, sk_sp<GrRenderTarget> rt);

    // For wrapped render targets we store it here.
    // For deferred proxies we will fill this in when we need to instantiate the deferred resource
    sk_sp<GrRenderTarget>       fTarget;

    // These don't usually get computed until the render target is instantiated, but the render
    // target proxy may need to answer queries about it before then. And since in the deferred case
    // we know the newly created render target will be internal, we are able to precompute what the
    // flags will ultimately end up being. In the wrapped case we just copy the wrapped
    // rendertarget's info here.
    GrRenderTargetPriv::Flags   fFlags;

    // The last drawTarget that wrote to or is currently going to write to this renderTarget
    // The drawTarget can be closed (e.g., no draw context is currently bound
    // to this renderTarget).
    // This back-pointer is required so that we can add a dependancy between
    // the drawTarget used to create the current contents of this renderTarget
    // and the drawTarget of a destination renderTarget to which this one is being drawn.
    GrDrawTarget* fLastDrawTarget;

    typedef GrSurfaceProxy INHERITED;
};

#endif
