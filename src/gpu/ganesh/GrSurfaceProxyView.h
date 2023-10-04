/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfaceProxyView_DEFINED
#define GrSurfaceProxyView_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/base/SkTypeTraits.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"

#include <string_view>
#include <type_traits>
#include <utility>

class GrRecordingContext;
class GrRenderTargetProxy;
class GrTextureProxy;
enum class SkBackingFit;
struct SkIRect;
namespace skgpu {
enum class Budgeted : bool;
enum class Mipmapped : bool;
}

class GrSurfaceProxyView {
public:
    GrSurfaceProxyView() = default;

    GrSurfaceProxyView(sk_sp<GrSurfaceProxy> proxy, GrSurfaceOrigin origin, skgpu::Swizzle swizzle)
            : fProxy(std::move(proxy)), fOrigin(origin), fSwizzle(swizzle) {}

    // This entry point is used when we don't care about the origin or the swizzle.
    explicit GrSurfaceProxyView(sk_sp<GrSurfaceProxy> proxy)
            : fProxy(std::move(proxy)), fOrigin(kTopLeft_GrSurfaceOrigin) {}

    GrSurfaceProxyView(GrSurfaceProxyView&& view) = default;
    GrSurfaceProxyView(const GrSurfaceProxyView&) = default;

    explicit operator bool() const { return SkToBool(fProxy.get()); }

    GrSurfaceProxyView& operator=(const GrSurfaceProxyView&) = default;
    GrSurfaceProxyView& operator=(GrSurfaceProxyView&& view) = default;

    bool operator==(const GrSurfaceProxyView& view) const;
    bool operator!=(const GrSurfaceProxyView& other) const { return !(*this == other); }

    int width() const { return this->proxy()->width(); }
    int height() const { return this->proxy()->height(); }
    SkISize dimensions() const { return this->proxy()->dimensions(); }

    skgpu::Mipmapped mipmapped() const;

    GrSurfaceProxy* proxy() const { return fProxy.get(); }
    sk_sp<GrSurfaceProxy> refProxy() const { return fProxy; }

    GrTextureProxy* asTextureProxy() const;
    sk_sp<GrTextureProxy> asTextureProxyRef() const;

    GrRenderTargetProxy* asRenderTargetProxy() const;
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() const;

    GrSurfaceOrigin origin() const { return fOrigin; }
    skgpu::Swizzle swizzle() const { return fSwizzle; }

    void concatSwizzle(skgpu::Swizzle swizzle);

    GrSurfaceProxyView makeSwizzle(skgpu::Swizzle swizzle) const&;

    GrSurfaceProxyView makeSwizzle(skgpu::Swizzle swizzle) &&;

    void reset();

    // Helper that copies a rect of a src view's proxy and then creates a view for the copy with
    // the same origin and swizzle as the src view.
    static GrSurfaceProxyView Copy(GrRecordingContext* context,
                                   GrSurfaceProxyView src,
                                   skgpu::Mipmapped mipmapped,
                                   SkIRect srcRect,
                                   SkBackingFit fit,
                                   skgpu::Budgeted budgeted,
                                   std::string_view label);

    static GrSurfaceProxyView Copy(GrRecordingContext* rContext,
                                   GrSurfaceProxyView src,
                                   skgpu::Mipmapped mipmapped,
                                   SkBackingFit fit,
                                   skgpu::Budgeted budgeted,
                                   std::string_view label);

    // This does not reset the origin or swizzle, so the View can still be used to access those
    // properties associated with the detached proxy.
    sk_sp<GrSurfaceProxy> detachProxy() { return std::move(fProxy); }

    using sk_is_trivially_relocatable = std::true_type;

private:
    sk_sp<GrSurfaceProxy> fProxy;
    GrSurfaceOrigin fOrigin = kTopLeft_GrSurfaceOrigin;
    skgpu::Swizzle fSwizzle;

    static_assert(::sk_is_trivially_relocatable<decltype(fProxy)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fOrigin)>::value);
    static_assert(::sk_is_trivially_relocatable<decltype(fSwizzle)>::value);
};

#endif
