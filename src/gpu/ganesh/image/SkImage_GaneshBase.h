/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GaneshBase_DEFINED
#define SkImage_GaneshBase_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/chromium/SkImageChromium.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "src/image/SkImage_Base.h"

#include <memory>
#include <tuple>
#include <cstddef>
#include <cstdint>

class GrBackendFormat;
class GrBackendTexture;
class GrCaps;
class GrContextThreadSafeProxy;
class GrDirectContext;
class GrFragmentProcessor;
class GrSurfaceProxyView;
class GrTextureProxy;
class SkBitmap;
class SkColorSpace;
class SkImage;
class SkMatrix;
class SkRecorder;
class SkSurface;
enum GrSurfaceOrigin : int;
enum SkAlphaType : int;
enum SkColorType : int;
enum class GrColorType;
enum class GrImageTexGenPolicy : int;
enum class GrSemaphoresSubmitted : bool;
enum class SkTileMode;
struct GrFlushInfo;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
struct SkRect;
namespace skgpu {
enum class Mipmapped : bool;
class RefCntedCallback;
}  // namespace skgpu
namespace skgpu::ganesh { class SurfaceDrawContext; }

class SkImage_GaneshBase : public SkImage_Base {
public:
    // From SkImage.h
    bool isValid(SkRecorder*) const final;

    // From SkImage_Base.h
    GrImageContext* context() const final { return fContext.get(); }
    GrDirectContext* directContext() const final { return GrAsDirectContext(this->context()); }

    bool getROPixels(GrDirectContext*, SkBitmap*, CachingHint) const final;

    sk_sp<SkSurface> onMakeSurface(SkRecorder*, const SkImageInfo&) const final;

    bool onReadPixels(GrDirectContext* dContext,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRB,
                      int srcX,
                      int srcY,
                      CachingHint) const override;

    virtual GrSemaphoresSubmitted flush(GrDirectContext*, const GrFlushInfo&) const = 0;

    static bool ValidateBackendTexture(const GrCaps*,
                                       const GrBackendTexture& tex,
                                       GrColorType grCT,
                                       SkColorType ct,
                                       SkAlphaType at,
                                       sk_sp<SkColorSpace> cs);
    static bool ValidateCompressedBackendTexture(const GrCaps*,
                                                 const GrBackendTexture& tex,
                                                 SkAlphaType);

    // Helper for making a lazy proxy for a promise image.
    // PromiseImageTextureFulfillProc must not be null.
    static sk_sp<GrTextureProxy> MakePromiseImageLazyProxy(
            GrContextThreadSafeProxy*,
            SkISize dimensions,
            const GrBackendFormat&,
            skgpu::Mipmapped,
            SkImages::PromiseImageTextureFulfillProc,
            sk_sp<skgpu::RefCntedCallback> releaseHelper);

    virtual std::tuple<GrSurfaceProxyView, GrColorType> asView(GrRecordingContext*,
                                                               skgpu::Mipmapped,
                                                               GrImageTexGenPolicy) const = 0;

    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            skgpu::ganesh::SurfaceDrawContext*,
            SkSamplingOptions,
            const SkTileMode[2],
            const SkMatrix&,
            const SkRect*,
            const SkRect*) const = 0;

    virtual GrSurfaceOrigin origin() const = 0;

protected:
    SkImage_GaneshBase(sk_sp<GrImageContext>, SkImageInfo, uint32_t uniqueID);
    sk_sp<SkImage> onMakeSubset(SkRecorder*, const SkIRect& subset, RequiredProperties) const final;
    sk_sp<SkImage> makeColorTypeAndColorSpace(SkRecorder*,
                                              SkColorType,
                                              sk_sp<SkColorSpace>,
                                              RequiredProperties) const final;
    virtual sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrDirectContext*,
                                                        SkColorType,
                                                        sk_sp<SkColorSpace>) const = 0;

    sk_sp<GrImageContext> fContext;
};

#endif
