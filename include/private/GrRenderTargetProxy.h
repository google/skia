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
    static sk_sp<GrRenderTargetProxy> Make(sk_sp<GrRenderTarget> rt);

    ~GrRenderTargetProxy() override;

    // TODO: add asTextureProxy variants
    GrRenderTargetProxy* asRenderTargetProxy() override { return this; }
    const GrRenderTargetProxy* asRenderTargetProxy() const override { return this; }

    // Actually instantiate the backing rendertarget, if necessary.
    GrRenderTarget* instantiate(GrTextureProvider* texProvider);

    /**
     * @return true  if the surface is multisampled in all buffers,
     *         false otherwise
     */
    bool isUnifiedMultisampled() const {
        if (fSampleConfig != GrRenderTarget::kUnified_SampleConfig) {
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
        if (fSampleConfig == GrRenderTarget::kUnified_SampleConfig) {
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
        SkASSERT(GrRenderTarget::kStencil_SampleConfig != fSampleConfig ||
                 this->isStencilBufferMultisampled());
        return GrRenderTarget::kStencil_SampleConfig == fSampleConfig;
    }

    void setLastDrawTarget(GrDrawTarget* dt);
    GrDrawTarget* getLastDrawTarget() { return fLastDrawTarget; }

private:
    // TODO: we can probably munge the 'desc' in both the wrapped and deferred 
    // cases to make the sampleConfig/numSamples stuff more rational.
    GrRenderTargetProxy(const GrCaps& caps, const GrSurfaceDesc& desc,
                        SkBackingFit fit, SkBudgeted budgeted)
        : INHERITED(desc, fit, budgeted)
        , fTarget(nullptr)
        , fSampleConfig(GrRenderTarget::ComputeSampleConfig(caps, desc.fSampleCnt))
        , fLastDrawTarget(nullptr) {
    }

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrRenderTarget> rt);

    // For wrapped render targets we store it here.
    // For deferred proxies we will fill this in when we need to instantiate the deferred resource
    sk_sp<GrRenderTarget>        fTarget;

    // The sample config doesn't usually get computed until the render target is instantiated but
    // the render target proxy may need to answer queries about it before then. For this reason
    // we precompute it in the deferred case. In the wrapped case we just copy the wrapped
    // rendertarget's info here.
    GrRenderTarget::SampleConfig fSampleConfig;

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
