/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "include/private/SkSpinlock.h"
#include "src/core/SkImagePriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/image/SkImage_GpuBase.h"

class GrDirectContext;
class GrRecordingContext;
class GrTexture;

class SkBitmap;

class SkImage_Gpu final : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrImageContext>, uint32_t uniqueID, GrSurfaceProxyView, SkColorType,
                SkAlphaType, sk_sp<SkColorSpace>);
    SkImage_Gpu(sk_sp<GrImageContext> context,
                uint32_t uniqueID,
                GrSurfaceProxyView view,
                SkColorInfo info)
            : SkImage_Gpu(std::move(context),
                          uniqueID,
                          std::move(view),
                          info.colorType(),
                          info.alphaType(),
                          info.refColorSpace()) {}

    static sk_sp<SkImage> MakeWithVolatileSrc(sk_sp<GrRecordingContext> rContext,
                                              GrSurfaceProxyView volatileSrc,
                                              SkColorInfo colorInfo);

    ~SkImage_Gpu() override;

    // If this is image is a cached SkSurface snapshot then this method is called by the SkSurface
    // before a write to check if the surface must make a copy to avoid modifying the image's
    // contents.
    bool surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const;

    bool onHasMipmaps() const override;

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) override;

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    bool onIsTextureBacked() const override { return true; }

    size_t onTextureSize() const override;

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                     const SkIRect& srcRect,
                                     RescaleGamma,
                                     RescaleMode,
                                     ReadPixelsCallback,
                                     ReadPixelsContext) override;

    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                           sk_sp<SkColorSpace>,
                                           const SkIRect& srcRect,
                                           const SkISize& dstSize,
                                           RescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) override;

    void generatingSurfaceIsDeleted() override;

private:
    SkImage_Gpu(sk_sp<GrDirectContext>,
                GrSurfaceProxyView volatileSrc,
                sk_sp<GrSurfaceProxy> stableCopy,
                sk_sp<GrRenderTask> copyTask,
                int volatileSrcTargetCount,
                SkColorInfo);

    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         GrMipmapped,
                                                         GrImageTexGenPolicy) const override;

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
        GrMipmapped mipmapped() const SK_EXCLUDES(fLock);
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
    GrSwizzle fSwizzle;
    GrSurfaceOrigin fOrigin;

    using INHERITED = SkImage_GpuBase;
};

#endif
