/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxyPriv_DEFINED
#define GrRenderTargetProxyPriv_DEFINED

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrRenderTargetProxy.h"

/**
 * This class hides the more specialized capabilities of GrRenderTargetProxy.
 */
class GrRenderTargetProxyPriv {
public:
    void setGLRTFBOIDIs0() {
        fRenderTargetProxy->setGLRTFBOIDIs0();
    }

    bool glRTFBOIDIs0() const {
        return fRenderTargetProxy->glRTFBOIDIs0();
    }

    /*
     * Indicate that a draw to this proxy requires stencil, and how many stencil samples it needs.
     * The number of stencil samples on this proxy will be equal to the largest sample count passed
     * to this method.
     */
    void setNeedsStencil(int numStencilSamples) {
        SkASSERT(numStencilSamples >= fRenderTargetProxy->fSampleCnt);
        fRenderTargetProxy->fNumStencilSamples = SkTMax(
                numStencilSamples, fRenderTargetProxy->fNumStencilSamples);
    }

    /**
     * Returns the number of stencil samples required by this proxy.
     * NOTE: Once instantiated, the actual render target may have more samples, but it is guaranteed
     * to have at least this many. (After a multisample stencil buffer has been attached to a render
     * target, we never "downgrade" it to one with fewer samples.)
     */
    int numStencilSamples() const { return fRenderTargetProxy->fNumStencilSamples; }

    bool canUseMixedSamples(const GrCaps& caps) const {
        // TODO: Is this a wrapped external RT? Is GrBackendObjectOwnership available?
        return caps.mixedSamplesSupport() &&
               !fRenderTargetProxy->glRTFBOIDIs0() &&
               caps.internalMultisampleCount(fRenderTargetProxy->config()) > 0;
    }

private:
    explicit GrRenderTargetProxyPriv(GrRenderTargetProxy* renderTargetProxy)
            : fRenderTargetProxy(renderTargetProxy) {}
    GrRenderTargetProxyPriv(const GrRenderTargetProxyPriv&) {} // unimpl
    GrRenderTargetProxyPriv& operator=(const GrRenderTargetProxyPriv&); // unimpl

    // No taking addresses of this type.
    const GrRenderTargetProxyPriv* operator&() const;
    GrRenderTargetProxyPriv* operator&();

    GrRenderTargetProxy* fRenderTargetProxy;

    friend class GrRenderTargetProxy;  // to construct/copy this type.
};

inline GrRenderTargetProxyPriv GrRenderTargetProxy::rtPriv() {
    return GrRenderTargetProxyPriv(this);
}

inline const GrRenderTargetProxyPriv GrRenderTargetProxy::rtPriv() const {
    return GrRenderTargetProxyPriv(const_cast<GrRenderTargetProxy*>(this));
}

#endif

