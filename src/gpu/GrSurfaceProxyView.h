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
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/GrTextureProxy.h"

class GrSurfaceProxyView {
public:
    GrSurfaceProxyView() = default;

    GrSurfaceProxyView(sk_sp<GrSurfaceProxy> proxy, GrSurfaceOrigin origin, GrSwizzle swizzle)
            : fProxy(proxy), fOrigin(origin), fSwizzle(swizzle) {}

    // This entry point is used when we don't care about the origin or the swizzle.
    explicit GrSurfaceProxyView(sk_sp<GrSurfaceProxy> proxy)
            : fProxy(proxy), fOrigin(kTopLeft_GrSurfaceOrigin) {}

    GrSurfaceProxyView(GrSurfaceProxyView&& view) = default;
    GrSurfaceProxyView(const GrSurfaceProxyView&) = default;

    operator bool() const { return SkToBool(fProxy.get()); }

    GrSurfaceProxyView& operator=(const GrSurfaceProxyView&) = default;
    GrSurfaceProxyView& operator=(GrSurfaceProxyView&& view) = default;

    bool operator==(const GrSurfaceProxyView& view) const {
        return fProxy->uniqueID() == view.fProxy->uniqueID() &&
               fOrigin == view.fOrigin &&
               fSwizzle == view.fSwizzle;
    }
    bool operator!=(const GrSurfaceProxyView& other) const { return !(*this == other); }

    int width() const { return this->proxy()->width(); }
    int height() const { return this->proxy()->height(); }
    SkISize dimensions() const { return this->proxy()->dimensions(); }

    GrSurfaceProxy* proxy() const { return fProxy.get(); }
    sk_sp<GrSurfaceProxy> refProxy() const { return fProxy; }

    GrTextureProxy* asTextureProxy() const {
        if (!fProxy) {
            return nullptr;
        }
        return fProxy->asTextureProxy();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef() const {
        return sk_ref_sp<GrTextureProxy>(this->asTextureProxy());
    }

    GrRenderTargetProxy* asRenderTargetProxy() const {
        if (!fProxy) {
            return nullptr;
        }
        return fProxy->asRenderTargetProxy();
    }

    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() const {
        return sk_ref_sp<GrRenderTargetProxy>(this->asRenderTargetProxy());
    }

    GrSurfaceOrigin origin() const { return fOrigin; }
    GrSwizzle swizzle() const { return fSwizzle; }

    void reset() {
        *this = {};
    }

    // This does not reset the origin or swizzle, so the View can still be used to access those
    // properties associated with the detached proxy.
    sk_sp<GrSurfaceProxy> detachProxy() {
        return std::move(fProxy);
    }

private:
    sk_sp<GrSurfaceProxy> fProxy;
    GrSurfaceOrigin fOrigin = kTopLeft_GrSurfaceOrigin;
    GrSwizzle fSwizzle;
};

#endif

