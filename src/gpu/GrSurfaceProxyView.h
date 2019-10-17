/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxyView_DEFINED
#define GrSurfaceProxyView_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSwizzle.h"

class GrSurfaceProxyView {
public:
    GrSurfaceProxyView(sk_sp<GrSurfaceProxy> proxy, GrSurfaceOrigin origin, GrSwizzle swizzle)
            : fProxy(proxy), fOrigin(origin), fSwizzle(swizzle) {}

    GrSurfaceProxy* asSurfaceProxy() { return fProxy.get(); }
    const GrSurfaceProxy* asSurfaceProxy() const { return fProxy.get(); }
    sk_sp<GrSurfaceProxy> asSurfaceProxyRef() const { return fProxy; }

    GrTextureProxy* asTextureProxy() { return fProxy->asTextureProxy(); }
    const GrTextureProxy* asTextureProxy() const { return fProxy->asTextureProxy(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() {
        return sk_ref_sp(fProxy->asTextureProxy());
    }

    GrRenderTargetProxy* asRenderTargetProxy() { return fProxy->asRenderTargetProxy(); }
    const GrRenderTargetProxy* asRenderTargetProxy() const { return fProxy->asRenderTargetProxy(); }
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() {
        return sk_ref_sp(fProxy->asRenderTargetProxy());
    }

    GrSurfaceOrigin origin() const { return fOrigin; }
    const GrSwizzle& swizzle() const { return fSwizzle; }

private:
    sk_sp<GrSurfaceProxy> fProxy;
    GrSurfaceOrigin fOrigin;
    GrSwizzle fSwizzle;
};

#endif

