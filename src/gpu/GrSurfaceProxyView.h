/*
 * Copyright 2019 Google LLC
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

    GrSurfaceProxyView(GrSurfaceProxyView&& view)
            : fProxy(std::move(view.fProxy)), fOrigin(view.fOrigin), fSwizzle(view.fSwizzle) {}

    GrSurfaceProxy* asSurfaceProxy() const { return fProxy.get(); }
    GrTextureProxy* asTextureProxy() const { return fProxy->asTextureProxy(); }
    GrRenderTargetProxy* asRenderTargetProxy() const { return fProxy->asRenderTargetProxy(); }

    GrSurfaceOrigin origin() const { return fOrigin; }
    const GrSwizzle& swizzle() const { return fSwizzle; }

private:
    sk_sp<GrSurfaceProxy> fProxy;
    GrSurfaceOrigin fOrigin;
    GrSwizzle fSwizzle;
};

#endif

