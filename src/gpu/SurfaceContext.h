/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceContext_DEFINED
#define SurfaceContext_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSurface.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrPixmap.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyView.h"

class GrDrawingManager;
class GrRecordingContext;
class GrRenderTargetProxy;
class GrSingleOwner;
class GrSurface;
class GrSurfaceProxy;
class GrTextureProxy;
struct SkIPoint;
struct SkIRect;

namespace skgpu {

class SurfaceFillContext;

/**
 * A helper object to orchestrate commands for a particular surface
 *
 *            SurfaceContext
 *                   |
 *          SurfaceFillContext
 *           /               \
 *     v1::SFC               v2::SFC
 *        |                     |
 *        |                     |
 *        |                     |
 *     v1::SDC               v2::SDC
 *
 */
class SurfaceContext {
public:
    // Makes either a SurfaceContext, SurfaceFillContext, or a SurfaceDrawContext, depending on
    // GrRenderable and the GrImageInfo.
    static std::unique_ptr<SurfaceContext> Make(GrRecordingContext*,
                                                const GrImageInfo&,
                                                const GrBackendFormat&,
                                                SkBackingFit = SkBackingFit::kExact,
                                                GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
                                                GrRenderable = GrRenderable::kNo,
                                                int renderTargetSampleCnt = 1,
                                                GrMipmapped = GrMipmapped::kNo,
                                                GrProtected = GrProtected::kNo,
                                                SkBudgeted = SkBudgeted::kYes);

    // Same as the above but chooses the texture format using the default format for the color type.
    static std::unique_ptr<SurfaceContext> Make(GrRecordingContext*,
                                                const GrImageInfo&,
                                                SkBackingFit = SkBackingFit::kExact,
                                                GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
                                                GrRenderable = GrRenderable::kNo,
                                                int renderTargetSampleCnt = 1,
                                                GrMipmapped = GrMipmapped::kNo,
                                                GrProtected = GrProtected::kNo,
                                                SkBudgeted = SkBudgeted::kYes);

    // If it is known that the GrSurfaceProxy is not renderable, you can directly call the ctor
    // here to make a SurfaceContext on the stack.
    SurfaceContext(GrRecordingContext*, GrSurfaceProxyView readView, const GrColorInfo&);

    virtual ~SurfaceContext() = default;

    GrRecordingContext* recordingContext() const { return fContext; }

    const GrColorInfo& colorInfo() const { return fColorInfo; }
    GrImageInfo imageInfo() const { return {fColorInfo, fReadView.proxy()->dimensions()}; }

    GrSurfaceOrigin origin() const { return fReadView.origin(); }
    GrSwizzle readSwizzle() const { return fReadView.swizzle(); }
    // TODO: See if it makes sense for this to return a const& instead and require the callers to
    // make a copy (which refs the proxy) if needed.
    GrSurfaceProxyView readSurfaceView() { return fReadView; }

    SkISize dimensions() const { return fReadView.dimensions(); }
    int width() const { return fReadView.proxy()->width(); }
    int height() const { return fReadView.proxy()->height(); }

    GrMipmapped mipmapped() const { return fReadView.mipmapped(); }

    const GrCaps* caps() const;

    /**
     * Reads a rectangle of pixels from the surface context.
     * @param dContext      The direct context to use
     * @param dst           destination pixels for the read
     * @param srcPt         offset w/in the surface context from which to read
     *                      is a GrDirectContext and fail otherwise.
     */
    bool readPixels(GrDirectContext* dContext, GrPixmap dst, SkIPoint srcPt);

    using ReadPixelsCallback = SkImage::ReadPixelsCallback;
    using ReadPixelsContext  = SkImage::ReadPixelsContext;
    using RescaleGamma       = SkImage::RescaleGamma;
    using RescaleMode        = SkImage::RescaleMode;

    // GPU implementation for SkImage:: and SkSurface::asyncRescaleAndReadPixels.
    void asyncRescaleAndReadPixels(GrDirectContext*,
                                   const SkImageInfo& info,
                                   const SkIRect& srcRect,
                                   RescaleGamma rescaleGamma,
                                   RescaleMode,
                                   ReadPixelsCallback callback,
                                   ReadPixelsContext callbackContext);

    // GPU implementation for SkImage:: and SkSurface::asyncRescaleAndReadPixelsYUV420.
    void asyncRescaleAndReadPixelsYUV420(GrDirectContext*,
                                         SkYUVColorSpace yuvColorSpace,
                                         sk_sp<SkColorSpace> dstColorSpace,
                                         const SkIRect& srcRect,
                                         SkISize dstSize,
                                         RescaleGamma rescaleGamma,
                                         RescaleMode,
                                         ReadPixelsCallback callback,
                                         ReadPixelsContext context);

    /**
     * Writes a rectangle of pixels from src into the surfaceDrawContext at the specified position.
     * @param dContext         The direct context to use
     * @param src              source for the write
     * @param dstPt            offset w/in the surface context at which to write
     */
    bool writePixels(GrDirectContext* dContext,
                     GrCPixmap src,
                     SkIPoint dstPt);

    /**
     * Fully populates either the base level or all MIP levels of the GrSurface with pixel data.
     * @param dContext         The direct context to use
     * @param src              Array of pixmaps
     * @param numLevels        Number of pixmaps in src. To succeed this must be 1 or the total
     *                         number of MIP levels.
     */
    bool writePixels(GrDirectContext* dContext,
                     const GrCPixmap src[],
                     int numLevels);

    GrSurfaceProxy* asSurfaceProxy() { return fReadView.proxy(); }
    const GrSurfaceProxy* asSurfaceProxy() const { return fReadView.proxy(); }
    sk_sp<GrSurfaceProxy> asSurfaceProxyRef() { return fReadView.refProxy(); }

    GrTextureProxy* asTextureProxy() { return fReadView.asTextureProxy(); }
    const GrTextureProxy* asTextureProxy() const { return fReadView.asTextureProxy(); }
    sk_sp<GrTextureProxy> asTextureProxyRef() { return fReadView.asTextureProxyRef(); }

    GrRenderTargetProxy* asRenderTargetProxy() { return fReadView.asRenderTargetProxy(); }
    const GrRenderTargetProxy* asRenderTargetProxy() const {
        return fReadView.asRenderTargetProxy();
    }
    sk_sp<GrRenderTargetProxy> asRenderTargetProxyRef() {
        return fReadView.asRenderTargetProxyRef();
    }

    virtual SurfaceFillContext* asFillContext() { return nullptr; }

    /**
     * Rescales the contents of srcRect. The gamma in which the rescaling occurs is controlled by
     * RescaleGamma. It is always in the original gamut. The result is converted to the color type
     * and color space of info after rescaling. Note: this currently requires that the info have a
     * different size than srcRect. Though, it could be relaxed to allow non-scaling color
     * conversions.
     */
    std::unique_ptr<skgpu::SurfaceFillContext> rescale(const GrImageInfo& info,
                                                       GrSurfaceOrigin,
                                                       SkIRect srcRect,
                                                       SkImage::RescaleGamma,
                                                       SkImage::RescaleMode);

    /**
     * Like the above but allows the caller ot specify a destination fill context and
     * rect within that context. The dst rect must be contained by the dst or this will fail.
     */
    bool rescaleInto(SurfaceFillContext* dst,
                     SkIRect dstRect,
                     SkIRect srcRect,
                     SkImage::RescaleGamma,
                     SkImage::RescaleMode);

#if GR_TEST_UTILS
    bool testCopy(sk_sp<GrSurfaceProxy> src, const SkIRect& srcRect, const SkIPoint& dstPoint) {
        return this->copy(std::move(src), srcRect, dstPoint) != nullptr;
    }

    bool testCopy(sk_sp<GrSurfaceProxy> src) {
        auto rect = SkIRect::MakeSize(src->dimensions());
        return this->copy(std::move(src), rect, {0, 0}) != nullptr;
    }
#endif

protected:
    GrDrawingManager* drawingManager();
    const GrDrawingManager* drawingManager() const;

    SkDEBUGCODE(void validate() const;)

    SkDEBUGCODE(GrSingleOwner* singleOwner() const;)

    GrRecordingContext* fContext;

    GrSurfaceProxyView fReadView;

    // Inserts a transfer, part of the implementation of asyncReadPixels and
    // asyncRescaleAndReadPixelsYUV420().
    struct PixelTransferResult {
        using ConversionFn = void(void* dst, const void* mappedBuffer);
        // If null then the transfer could not be performed. Otherwise this buffer will contain
        // the pixel data when the transfer is complete.
        sk_sp<GrGpuBuffer> fTransferBuffer;
        // If this is null then the transfer buffer will contain the data in the requested
        // color type. Otherwise, when the transfer is done this must be called to convert
        // from the transfer buffer's color type to the requested color type.
        std::function<ConversionFn> fPixelConverter;
    };
    PixelTransferResult transferPixels(GrColorType colorType, const SkIRect& rect);

    // The async read step of asyncRescaleAndReadPixels()
    void asyncReadPixels(GrDirectContext*,
                         const SkIRect& srcRect,
                         SkColorType,
                         ReadPixelsCallback,
                         ReadPixelsContext);

private:
    friend class ::GrRecordingContextPriv; // for validate
    friend class ::GrSurfaceProxy; // for copy

    SkDEBUGCODE(virtual void onValidate() const {})

    /**
     * Copy 'src' into the proxy backing this context. This call will not do any draw fallback.
     * Currently only writePixels and replaceRenderTarget call this directly. All other copies
     * should go through GrSurfaceProxy::Copy.
     * @param src       src of pixels
     * @param dstPoint  the origin of the 'srcRect' in the destination coordinate space
     * @return          a task (that may be skippable by calling canSkip) if successful and
     *                  null otherwise.
     *
     * Note: Notionally, 'srcRect' is clipped to 'src's extent with 'dstPoint' being adjusted.
     *       Then the 'srcRect' offset by 'dstPoint' is clipped against the dst's extent.
     *       The end result is only valid src pixels and dst pixels will be touched but the copied
     *       regions will not be shifted. The 'src' must have the same origin as the backing proxy
     *       of fSurfaceContext.
     */
    sk_sp<GrRenderTask> copy(sk_sp<GrSurfaceProxy> src, SkIRect srcRect, SkIPoint dstPoint);

    bool internalWritePixels(GrDirectContext* dContext,
                             const GrCPixmap src[],
                             int numLevels,
                             SkIPoint);

    class AsyncReadResult;

    GrColorInfo fColorInfo;

    using INHERITED = SkRefCnt;
};

} // namespace skgpu

#endif // SurfaceContext_DEFINED
