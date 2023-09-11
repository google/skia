/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Ganesh_DEFINED
#define SkImage_Ganesh_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/base/SkSpinlock.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/image/SkImage_Base.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

class GrBackendFormat;
class GrBackendTexture;
class GrDirectContext;
class GrFragmentProcessor;
class GrImageContext;
class GrRecordingContext;
class GrRenderTask;
class GrSurfaceProxy;
class SkColorInfo;
class SkColorSpace;
class SkImage;
class SkMatrix;
enum class GrColorType;
enum class GrImageTexGenPolicy : int;
enum class GrSemaphoresSubmitted : bool;
enum class SkTileMode;
enum GrSurfaceOrigin : int;
enum SkColorType : int;
enum SkYUVColorSpace : int;
struct GrFlushInfo;
struct SkIRect;
struct SkISize;
struct SkImageInfo;
struct SkRect;

namespace skgpu {
enum class Mipmapped : bool;
enum class Protected : bool;
}

class SkImage_Ganesh final : public SkImage_GaneshBase {
public:
    SkImage_Ganesh(sk_sp<GrImageContext> context,
                   uint32_t uniqueID,
                   GrSurfaceProxyView view,
                   SkColorInfo info);

    static sk_sp<SkImage> MakeWithVolatileSrc(sk_sp<GrRecordingContext> rContext,
                                              GrSurfaceProxyView volatileSrc,
                                              SkColorInfo colorInfo);

    ~SkImage_Ganesh() override;

    // From SkImage.h
    size_t textureSize() const override;

    // From SkImage_Base.h
    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGanesh; }

    bool onHasMipmaps() const override;
    bool onIsProtected() const override;

    using SkImage_GaneshBase::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                     SkIRect srcRect,
                                     RescaleGamma,
                                     RescaleMode,
                                     ReadPixelsCallback,
                                     ReadPixelsContext) const override;

    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                           bool readAlpha,
                                           sk_sp<SkColorSpace>,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) const override;

    void generatingSurfaceIsDeleted() override;

    // From SkImage_GaneshBase.h
    GrSemaphoresSubmitted flush(GrDirectContext*, const GrFlushInfo&) const override;

    std::tuple<GrSurfaceProxyView, GrColorType> asView(GrRecordingContext*,
                                                       skgpu::Mipmapped,
                                                       GrImageTexGenPolicy) const override;

    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             SkSamplingOptions,
                                                             const SkTileMode[2],
                                                             const SkMatrix&,
                                                             const SkRect*,
                                                             const SkRect*) const override;

    // If this is image is a cached SkSurface snapshot then this method is called by the SkSurface
    // before a write to check if the surface must make a copy to avoid modifying the image's
    // contents.
    bool surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const;

    bool getExistingBackendTexture(GrBackendTexture* outTexture,
                                   bool flushPendingGrContextIO,
                                   GrSurfaceOrigin* origin) const;

    GrSurfaceOrigin origin() const override { return fOrigin; }

private:
    SkImage_Ganesh(sk_sp<GrDirectContext>,
                   GrSurfaceProxyView volatileSrc,
                   sk_sp<GrSurfaceProxy> stableCopy,
                   sk_sp<GrRenderTask> copyTask,
                   int volatileSrcTargetCount,
                   SkColorInfo);

    GrSurfaceProxyView makeView(GrRecordingContext*) const;

    // Thread-safe wrapper around the proxies backing this image. Handles dynamically switching
    // from a "volatile" proxy that may be overwritten (by an SkSurface that this image was snapped
    // from) to a "stable" proxy that is a copy of the volatile proxy. It allows the image to cancel
    // the copy if the stable proxy is never required because the contents of the volatile proxy
    // were never mutated by the SkSurface during the image lifetime.
    class ProxyChooser {
    public:
        ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy,
                     sk_sp<GrSurfaceProxy> volatileProxy,
                     sk_sp<GrRenderTask> copyTask,
                     int volatileProxyTargetCount);

        ProxyChooser(sk_sp<GrSurfaceProxy> stableProxy);

        ~ProxyChooser();

        // Checks if there is a volatile proxy that is safe to use. If so returns it, otherwise
        // returns the stable proxy (and drops the volatile one if it exists).
        sk_sp<GrSurfaceProxy> chooseProxy(GrRecordingContext* context) SK_EXCLUDES(fLock);
        // Call when it is known copy is necessary.
        sk_sp<GrSurfaceProxy> switchToStableProxy() SK_EXCLUDES(fLock);
        // Call when it is known for sure copy won't be necessary.
        sk_sp<GrSurfaceProxy> makeVolatileProxyStable() SK_EXCLUDES(fLock);

        bool surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const SK_EXCLUDES(fLock);

        // Queries that should be independent of which proxy is in use.
        size_t gpuMemorySize() const SK_EXCLUDES(fLock);
        skgpu::Mipmapped mipmapped() const SK_EXCLUDES(fLock);
        skgpu::Protected isProtected() const SK_EXCLUDES(fLock);
#ifdef SK_DEBUG
        const GrBackendFormat& backendFormat() SK_EXCLUDES(fLock);
#endif

    private:
        mutable SkSpinlock fLock;
        sk_sp<GrSurfaceProxy> fStableProxy SK_GUARDED_BY(fLock);
        sk_sp<GrSurfaceProxy> fVolatileProxy SK_GUARDED_BY(fLock);
        sk_sp<GrRenderTask> fVolatileToStableCopyTask;
        // The number of GrRenderTasks targeting the volatile proxy at creation time. If the
        // proxy's target count increases it indicates additional writes and we must switch
        // to using the stable copy.
        const int fVolatileProxyTargetCount = 0;
    };

    mutable ProxyChooser fChooser;
    skgpu::Swizzle fSwizzle;
    GrSurfaceOrigin fOrigin;

    using INHERITED = SkImage_GaneshBase;
};

#endif
