/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxyPriv_DEFINED
#define GrRenderTargetProxyPriv_DEFINED

#include "GrRenderTargetProxy.h"

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

    // Once the size of a fully-lazy render target proxy is decided, and before it gets
    // instantiated, the client can use this optional method to specify the proxy's size. (A proxy's
    // size can be less than the GPU surface that backs it. e.g., SkBackingFit::kApprox.) Otherwise,
    // the proxy's size will be set to match the underlying render target upon instantiation.
    void setContentSize(const SkISize& contentSize) {
        SkASSERT(GrSurfaceProxy::LazyState::kFully == fRenderTargetProxy->lazyInstantiationState());
        SkASSERT(contentSize.width() > 0);
        SkASSERT(contentSize.height() > 0);
        fRenderTargetProxy->fWidth = contentSize.width();
        fRenderTargetProxy->fHeight = contentSize.height();
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

