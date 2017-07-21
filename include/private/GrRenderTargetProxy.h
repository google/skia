/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxy_DEFINED
#define GrRenderTargetProxy_DEFINED

#include "GrSurfaceProxy.h"
#include "GrTypesPriv.h"

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
    bool instantiate(GrResourceProvider*) override;

    GrFSAAType fsaaType() const {
        if (!fSampleCnt) {
            SkASSERT(!(fRenderTargetFlags & GrRenderTargetFlags::kMixedSampled));
            return GrFSAAType::kNone;
        }
        return (fRenderTargetFlags & GrRenderTargetFlags::kMixedSampled)
                                                             ? GrFSAAType::kMixedSamples
                                                             : GrFSAAType::kUnifiedMSAA;
    }

    /**
     * Returns the number of samples/pixel in the stencil buffer (Zero if non-MSAA).
     */
    int numStencilSamples() const { return fSampleCnt; }

    /**
     * Returns the number of samples/pixel in the color buffer (Zero if non-MSAA or mixed sampled).
     */
    int numColorSamples() const {
        return GrFSAAType::kMixedSamples == this->fsaaType() ? 0 : fSampleCnt;
    }

    int worstCaseWidth() const;

    int worstCaseHeight() const;

    int maxWindowRectangles(const GrCaps& caps) const;

    GrRenderTargetFlags testingOnly_getFlags() const;

    // TODO: move this to a priv class!
    bool refsWrappedObjects() const;

protected:
    friend class GrSurfaceProxy;  // for ctors

    // Deferred version
    GrRenderTargetProxy(const GrCaps&, const GrSurfaceDesc&,
                        SkBackingFit, SkBudgeted, uint32_t flags);

    // Wrapped version
    GrRenderTargetProxy(sk_sp<GrSurface>);

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    size_t onUninstantiatedGpuMemorySize() const override;

    int                 fSampleCnt;
    // For wrapped render targets the actual GrRenderTarget is stored in the GrIORefProxy class.
    // For deferred proxies that pointer is filled in when we need to instantiate the
    // deferred resource.

    // These don't usually get computed until the render target is instantiated, but the render
    // target proxy may need to answer queries about it before then. And since in the deferred case
    // we know the newly created render target will be internal, we are able to precompute what the
    // flags will ultimately end up being. In the wrapped case we just copy the wrapped
    // rendertarget's info here.
    GrRenderTargetFlags fRenderTargetFlags;

    typedef GrSurfaceProxy INHERITED;
};

#endif
