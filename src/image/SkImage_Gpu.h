/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/private/SkSpinlock.h"
#include "include/private/base/SkThreadAnnotations.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuBase.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

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
enum GrSurfaceOrigin : int;
enum SkColorType : int;
enum SkYUVColorSpace : int;
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
}

class SkImage_Gpu final : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrImageContext> context,
                uint32_t uniqueID,
                GrSurfaceProxyView view,
                SkColorInfo info);

    static sk_sp<SkImage> MakeWithVolatileSrc(sk_sp<GrRecordingContext> rContext,
                                              GrSurfaceProxyView volatileSrc,
                                              SkColorInfo colorInfo);

    ~SkImage_Gpu() override;

    // If this is image is a cached SkSurface snapshot then this method is called by the SkSurface
    // before a write to check if the surface must make a copy to avoid modifying the image's
    // contents.
    bool surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const;

    bool onHasMipmaps() const override;

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) const override;

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGanesh; }

    size_t onTextureSize() const override;

    using SkImage_GpuBase::onMakeColorTypeAndColorSpace;
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
                                           sk_sp<SkColorSpace>,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) const override;

    void generatingSurfaceIsDeleted() override;

private:
    SkImage_Gpu(sk_sp<GrDirectContext>,
                GrSurfaceProxyView volatileSrc,
                sk_sp<GrSurfaceProxy> stableCopy,
                sk_sp<GrRenderTask> copyTask,
                int volatileSrcTargetCount,
                SkColorInfo);

    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         skgpu::Mipmapped,
                                                         GrImageTexGenPolicy) const override;

    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(GrRecordingContext*,
                                                               SkSamplingOptions,
                                                               const SkTileMode[2],
                                                               const SkMatrix&,
                                                               const SkRect*,
                                                               const SkRect*) const override;

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
#ifdef SK_DEBUG
        GrBackendFormat backendFormat() SK_EXCLUDES(fLock);
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

    using INHERITED = SkImage_GpuBase;
};

#endif
