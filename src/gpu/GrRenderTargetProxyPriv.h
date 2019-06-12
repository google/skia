/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRenderTargetProxyPriv_DEFINED
#define GrRenderTargetProxyPriv_DEFINED

#include "include/private/GrRenderTargetProxy.h"
#include "src/gpu/GrCaps.h"

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

    bool hasIntermediateMSAARenderbuffer(const GrCaps& caps) const {
        // A RT has a separate MSAA renderbuffer if:
        // 1) It's multisampled
        // 2) We do not use implicit-resolve extensions.
        // 3) It is not mixed sampled.
        // 4) It's not FBO 0, which is special and always auto-resolves.
        return !caps.usesImplicitMSAAResolve() &&
               GrFSAAType::kUnifiedMSAA == fRenderTargetProxy->fsaaType() &&
               !fRenderTargetProxy->rtPriv().glRTFBOIDIs0();
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

