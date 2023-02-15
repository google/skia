/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuBase_DEFINED
#define SkImage_GpuBase_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>

class GrBackendFormat;
class GrBackendTexture;
class GrCaps;
class GrContextThreadSafeProxy;
class GrDirectContext;
class GrRecordingContext;
class GrTextureProxy;
class SkBitmap;
class SkColorSpace;
class SkImage;
enum SkAlphaType : int;
enum SkColorType : int;
enum class GrColorType;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
namespace skgpu {
enum class Mipmapped : bool;
class RefCntedCallback;
}

class SkImage_GpuBase : public SkImage_Base {
public:
    GrImageContext* context() const final { return fContext.get(); }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset, GrDirectContext*) const final;

    bool onReadPixels(GrDirectContext *dContext,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRB,
                      int srcX,
                      int srcY,
                      CachingHint) const override;

    bool onIsValid(GrRecordingContext*) const final;

    static bool ValidateBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                       GrColorType grCT, SkColorType ct, SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool ValidateCompressedBackendTexture(const GrCaps*, const GrBackendTexture& tex,
                                                 SkAlphaType);

    // Helper for making a lazy proxy for a promise image.
    // PromiseImageTextureFulfillProc must not be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(
            GrContextThreadSafeProxy*,
            SkISize dimensions,
            GrBackendFormat,
            skgpu::Mipmapped,
            PromiseImageTextureFulfillProc,
            sk_sp<skgpu::RefCntedCallback> releaseHelper);

protected:
    SkImage_GpuBase(sk_sp<GrImageContext>, SkImageInfo, uint32_t uniqueID);

    sk_sp<GrImageContext> fContext;

#ifdef SK_GRAPHITE_ENABLED
    sk_sp<SkImage> onMakeTextureImage(skgpu::graphite::Recorder*,
                                      RequiredImageProperties) const final;
    sk_sp<SkImage> onMakeSubset(const SkIRect& subset,
                                skgpu::graphite::Recorder*,
                                RequiredImageProperties) const final;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                sk_sp<SkColorSpace>,
                                                skgpu::graphite::Recorder*,
                                                RequiredImageProperties) const final;
#endif
};

#endif
