/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrTextureRenderTargetProxy_DEFINED
#define GrTextureRenderTargetProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"

#include <cstddef>
#include <string_view>

class GrBackendFormat;
class GrResourceProvider;
class GrSurface;
enum class GrDDLProvider : bool;
enum class GrInternalSurfaceFlags;
enum class GrMipmapStatus;
enum class SkBackingFit;
struct SkISize;

namespace skgpu {
enum class Budgeted : bool;
enum class Mipmapped : bool;
}  // namespace skgpu

#ifdef SK_BUILD_FOR_WIN
// Windows gives warnings about inheriting asTextureProxy/asRenderTargetProxy via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

// This class delays the acquisition of RenderTargets that are also textures until
// they are actually required
// Beware: the uniqueID of the TextureRenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget/Texture it represents!
class GrTextureRenderTargetProxy : public GrRenderTargetProxy, public GrTextureProxy {
private:
    // DDL TODO: rm the GrSurfaceProxy friending
    friend class GrSurfaceProxy; // for ctors
    friend class GrProxyProvider; // for ctors

    // Deferred version
    GrTextureRenderTargetProxy(const GrCaps&,
                               const GrBackendFormat&,
                               SkISize,
                               int sampleCnt,
                               skgpu::Mipmapped,
                               GrMipmapStatus,
                               SkBackingFit,
                               skgpu::Budgeted,
                               GrProtected,
                               GrInternalSurfaceFlags,
                               UseAllocator,
                               GrDDLProvider creatingProvider,
                               std::string_view label);

    // Lazy-callback version
    GrTextureRenderTargetProxy(const GrCaps&,
                               LazyInstantiateCallback&&,
                               const GrBackendFormat&,
                               SkISize,
                               int sampleCnt,
                               skgpu::Mipmapped,
                               GrMipmapStatus,
                               SkBackingFit,
                               skgpu::Budgeted,
                               GrProtected,
                               GrInternalSurfaceFlags,
                               UseAllocator,
                               GrDDLProvider creatingProvider,
                               std::string_view label);

    // Wrapped version
    GrTextureRenderTargetProxy(sk_sp<GrSurface>,
                               UseAllocator,
                               GrDDLProvider creatingProvider);

    void initSurfaceFlags(const GrCaps&);

    bool instantiate(GrResourceProvider*) override;
    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

    size_t onUninstantiatedGpuMemorySize() const override;
    LazySurfaceDesc callbackDesc() const override;
    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
