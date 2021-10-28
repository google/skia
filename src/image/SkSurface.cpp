/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <atomic>
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkRescaleAndReadPixels.h"
#include "src/image/SkSurface_Base.h"

SkSurfaceProps::SkSurfaceProps() : fFlags(0), fPixelGeometry(kUnknown_SkPixelGeometry) {}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, SkPixelGeometry pg)
    : fFlags(flags), fPixelGeometry(pg)
{}

SkSurfaceProps::SkSurfaceProps(const SkSurfaceProps&) = default;
SkSurfaceProps& SkSurfaceProps::operator=(const SkSurfaceProps&) = default;

///////////////////////////////////////////////////////////////////////////////

SkSurface_Base::SkSurface_Base(int width, int height, const SkSurfaceProps* props)
    : INHERITED(width, height, props) {
}

SkSurface_Base::SkSurface_Base(const SkImageInfo& info, const SkSurfaceProps* props)
    : INHERITED(info, props) {
}

SkSurface_Base::~SkSurface_Base() {
    // in case the canvas outsurvives us, we null the callback
    if (fCachedCanvas) {
        fCachedCanvas->setSurfaceBase(nullptr);
    }
#if SK_SUPPORT_GPU
    if (fCachedImage) {
        as_IB(fCachedImage.get())->generatingSurfaceIsDeleted();
    }
#endif
}

GrRecordingContext* SkSurface_Base::onGetRecordingContext() {
    return nullptr;
}

GrBackendTexture SkSurface_Base::onGetBackendTexture(BackendHandleAccess) {
    return GrBackendTexture(); // invalid
}

GrBackendRenderTarget SkSurface_Base::onGetBackendRenderTarget(BackendHandleAccess) {
    return GrBackendRenderTarget(); // invalid
}

bool SkSurface_Base::onReplaceBackendTexture(const GrBackendTexture&,
                                             GrSurfaceOrigin, ContentChangeMode,
                                             TextureReleaseProc,
                                             ReleaseContext) {
    return false;
}

void SkSurface_Base::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                            const SkSamplingOptions& sampling, const SkPaint* paint) {
    auto image = this->makeImageSnapshot();
    if (image) {
        canvas->drawImage(image.get(), x, y, sampling, paint);
    }
}

void SkSurface_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                                 const SkIRect& origSrcRect,
                                                 SkSurface::RescaleGamma rescaleGamma,
                                                 RescaleMode rescaleMode,
                                                 SkSurface::ReadPixelsCallback callback,
                                                 SkSurface::ReadPixelsContext context) {
    SkBitmap src;
    SkPixmap peek;
    SkIRect srcRect;
    if (this->peekPixels(&peek)) {
        src.installPixels(peek);
        srcRect = origSrcRect;
    } else {
        src.setInfo(this->imageInfo().makeDimensions(origSrcRect.size()));
        src.allocPixels();
        if (!this->readPixels(src, origSrcRect.x(), origSrcRect.y())) {
            callback(context, nullptr);
            return;
        }
        srcRect = SkIRect::MakeSize(src.dimensions());
    }
    return SkRescaleAndReadPixels(src, info, srcRect, rescaleGamma, rescaleMode, callback,
                                  context);
}

void SkSurface_Base::onAsyncRescaleAndReadPixelsYUV420(
        SkYUVColorSpace yuvColorSpace, sk_sp<SkColorSpace> dstColorSpace, const SkIRect& srcRect,
        const SkISize& dstSize, RescaleGamma rescaleGamma, RescaleMode,
        ReadPixelsCallback callback, ReadPixelsContext context) {
    // TODO: Call non-YUV asyncRescaleAndReadPixels and then make our callback convert to YUV and
    // call client's callback.
    callback(context, nullptr);
}

bool SkSurface_Base::outstandingImageSnapshot() const {
    return fCachedImage && !fCachedImage->unique();
}

bool SkSurface_Base::aboutToDraw(ContentChangeMode mode) {
    this->dirtyGenerationID();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);

    if (fCachedImage) {
        // the surface may need to fork its backend, if its sharing it with
        // the cached image. Note: we only call if there is an outstanding owner
        // on the image (besides us).
        bool unique = fCachedImage->unique();
        if (!unique) {
#ifdef SK_SURFACE_COPY_ON_WRITE_CRASHES
            this->onCopyOnWrite(mode);
#else
            if (!this->onCopyOnWrite(mode)) {
                return false;
            }
#endif
        }

        // regardless of copy-on-write, we must drop our cached image now, so
        // that the next request will get our new contents.
        fCachedImage.reset();

        if (unique) {
            // Our content isn't held by any image now, so we can consider that content mutable.
            // Raster surfaces need to be told it's safe to consider its pixels mutable again.
            // We make this call after the ->unref() so the subclass can assert there are no images.
            this->onRestoreBackingMutability();
        }
    } else if (kDiscard_ContentChangeMode == mode) {
        this->onDiscard();
    }
    return true;
}

uint32_t SkSurface_Base::newGenerationID() {
    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    static std::atomic<uint32_t> nextID{1};
    return nextID.fetch_add(1, std::memory_order_relaxed);
}

static SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

static const SkSurface_Base* asConstSB(const SkSurface* surface) {
    return static_cast<const SkSurface_Base*>(surface);
}

///////////////////////////////////////////////////////////////////////////////

SkSurface::SkSurface(int width, int height, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(width), fHeight(height)
{
    SkASSERT(fWidth > 0);
    SkASSERT(fHeight > 0);
    fGenerationID = 0;
}

SkSurface::SkSurface(const SkImageInfo& info, const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props)), fWidth(info.width()), fHeight(info.height())
{
    SkASSERT(fWidth > 0);
    SkASSERT(fHeight > 0);
    fGenerationID = 0;
}

SkImageInfo SkSurface::imageInfo() {
    // TODO: do we need to go through canvas for this?
    return this->getCanvas()->imageInfo();
}

uint32_t SkSurface::generationID() {
    if (0 == fGenerationID) {
        fGenerationID = asSB(this)->newGenerationID();
    }
    return fGenerationID;
}

void SkSurface::notifyContentWillChange(ContentChangeMode mode) {
    sk_ignore_unused_variable(asSB(this)->aboutToDraw(mode));
}

SkCanvas* SkSurface::getCanvas() {
    return asSB(this)->getCachedCanvas();
}

sk_sp<SkImage> SkSurface::makeImageSnapshot() {
    return asSB(this)->refCachedImage();
}

sk_sp<SkImage> SkSurface::makeImageSnapshot(const SkIRect& srcBounds) {
    const SkIRect surfBounds = { 0, 0, fWidth, fHeight };
    SkIRect bounds = srcBounds;
    if (!bounds.intersect(surfBounds)) {
        return nullptr;
    }
    SkASSERT(!bounds.isEmpty());
    if (bounds == surfBounds) {
        return this->makeImageSnapshot();
    } else {
        return asSB(this)->onNewImageSnapshot(&bounds);
    }
}

sk_sp<SkSurface> SkSurface::makeSurface(const SkImageInfo& info) {
    return asSB(this)->onNewSurface(info);
}

sk_sp<SkSurface> SkSurface::makeSurface(int width, int height) {
    return this->makeSurface(this->imageInfo().makeWH(width, height));
}

void SkSurface::draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkSamplingOptions& sampling,
                     const SkPaint* paint) {
    asSB(this)->onDraw(canvas, x, y, sampling, paint);
}

bool SkSurface::peekPixels(SkPixmap* pmap) {
    return this->getCanvas()->peekPixels(pmap);
}

bool SkSurface::readPixels(const SkPixmap& pm, int srcX, int srcY) {
    return this->getCanvas()->readPixels(pm, srcX, srcY);
}

bool SkSurface::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                           int srcX, int srcY) {
    return this->readPixels({dstInfo, dstPixels, dstRowBytes}, srcX, srcY);
}

bool SkSurface::readPixels(const SkBitmap& bitmap, int srcX, int srcY) {
    SkPixmap pm;
    return bitmap.peekPixels(&pm) && this->readPixels(pm, srcX, srcY);
}

void SkSurface::asyncRescaleAndReadPixels(const SkImageInfo& info,
                                          const SkIRect& srcRect,
                                          RescaleGamma rescaleGamma,
                                          RescaleMode rescaleMode,
                                          ReadPixelsCallback callback,
                                          ReadPixelsContext context) {
    if (!SkIRect::MakeWH(this->width(), this->height()).contains(srcRect) ||
        !SkImageInfoIsValid(info)) {
        callback(context, nullptr);
        return;
    }
    asSB(this)->onAsyncRescaleAndReadPixels(
            info, srcRect, rescaleGamma, rescaleMode, callback, context);
}

void SkSurface::asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                sk_sp<SkColorSpace> dstColorSpace,
                                                const SkIRect& srcRect,
                                                const SkISize& dstSize,
                                                RescaleGamma rescaleGamma,
                                                RescaleMode rescaleMode,
                                                ReadPixelsCallback callback,
                                                ReadPixelsContext context) {
    if (!SkIRect::MakeWH(this->width(), this->height()).contains(srcRect) || dstSize.isZero() ||
        (dstSize.width() & 0b1) || (dstSize.height() & 0b1)) {
        callback(context, nullptr);
        return;
    }
    asSB(this)->onAsyncRescaleAndReadPixelsYUV420(yuvColorSpace,
                                                  std::move(dstColorSpace),
                                                  srcRect,
                                                  dstSize,
                                                  rescaleGamma,
                                                  rescaleMode,
                                                  callback,
                                                  context);
}

void SkSurface::writePixels(const SkPixmap& pmap, int x, int y) {
    if (pmap.addr() == nullptr || pmap.width() <= 0 || pmap.height() <= 0) {
        return;
    }

    const SkIRect srcR = SkIRect::MakeXYWH(x, y, pmap.width(), pmap.height());
    const SkIRect dstR = SkIRect::MakeWH(this->width(), this->height());
    if (SkIRect::Intersects(srcR, dstR)) {
        ContentChangeMode mode = kRetain_ContentChangeMode;
        if (srcR.contains(dstR)) {
            mode = kDiscard_ContentChangeMode;
        }
        if (!asSB(this)->aboutToDraw(mode)) {
            return;
        }
        asSB(this)->onWritePixels(pmap, x, y);
    }
}

void SkSurface::writePixels(const SkBitmap& src, int x, int y) {
    SkPixmap pm;
    if (src.peekPixels(&pm)) {
        this->writePixels(pm, x, y);
    }
}

GrRecordingContext* SkSurface::recordingContext() {
    return asSB(this)->onGetRecordingContext();
}

GrBackendTexture SkSurface::getBackendTexture(BackendHandleAccess access) {
    return asSB(this)->onGetBackendTexture(access);
}

GrBackendRenderTarget SkSurface::getBackendRenderTarget(BackendHandleAccess access) {
    return asSB(this)->onGetBackendRenderTarget(access);
}

bool SkSurface::replaceBackendTexture(const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, ContentChangeMode mode,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext) {
    return asSB(this)->onReplaceBackendTexture(backendTexture, origin, mode, textureReleaseProc,
                                               releaseContext);
}

GrSemaphoresSubmitted SkSurface::flush(BackendSurfaceAccess access, const GrFlushInfo& flushInfo) {
    return asSB(this)->onFlush(access, flushInfo, nullptr);
}

GrSemaphoresSubmitted SkSurface::flush(const GrFlushInfo& info,
                                       const GrBackendSurfaceMutableState* newState) {
    return asSB(this)->onFlush(BackendSurfaceAccess::kNoAccess, info, newState);
}

bool SkSurface::wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                     bool deleteSemaphoresAfterWait) {
    return asSB(this)->onWait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

bool SkSurface::characterize(SkSurfaceCharacterization* characterization) const {
    return asConstSB(this)->onCharacterize(characterization);
}

bool SkSurface::isCompatible(const SkSurfaceCharacterization& characterization) const {
    return asConstSB(this)->onIsCompatible(characterization);
}

bool SkSurface::draw(sk_sp<const SkDeferredDisplayList> ddl, int xOffset, int yOffset) {
    if (xOffset != 0 || yOffset != 0) {
        return false; // the offsets currently aren't supported
    }

    return asSB(this)->onDraw(std::move(ddl), { xOffset, yOffset });
}

//////////////////////////////////////////////////////////////////////////////////////
#include "include/utils/SkNoDrawCanvas.h"

class SkNullSurface : public SkSurface_Base {
public:
    SkNullSurface(int width, int height) : SkSurface_Base(width, height, nullptr) {}

protected:
    SkCanvas* onNewCanvas() override {
        return new SkNoDrawCanvas(this->width(), this->height());
    }
    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info) override {
        return MakeNull(info.width(), info.height());
    }
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subsetOrNull) override { return nullptr; }
    void onWritePixels(const SkPixmap&, int x, int y) override {}
    void onDraw(SkCanvas*, SkScalar, SkScalar, const SkSamplingOptions&, const SkPaint*) override {}
#ifdef SK_SURFACE_COPY_ON_WRITE_CRASHES
    void onCopyOnWrite(ContentChangeMode) override {}
#else
    bool onCopyOnWrite(ContentChangeMode) override { return true; }
#endif
};

sk_sp<SkSurface> SkSurface::MakeNull(int width, int height) {
    if (width < 1 || height < 1) {
        return nullptr;
    }
    return sk_sp<SkSurface>(new SkNullSurface(width, height));
}

//////////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext*, SkBudgeted, const SkImageInfo&,
                                             int, GrSurfaceOrigin, const SkSurfaceProps*, bool) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext*, const SkSurfaceCharacterization&,
                                             SkBudgeted) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendTexture(GrRecordingContext*, const GrBackendTexture&,
                                                   GrSurfaceOrigin origin, int sampleCnt,
                                                   SkColorType, sk_sp<SkColorSpace>,
                                                   const SkSurfaceProps*,
                                                   TextureReleaseProc, ReleaseContext) {
    return nullptr;
}

sk_sp<SkSurface> SkSurface::MakeFromBackendRenderTarget(GrRecordingContext*,
                                                        const GrBackendRenderTarget&,
                                                        GrSurfaceOrigin origin,
                                                        SkColorType,
                                                        sk_sp<SkColorSpace>,
                                                        const SkSurfaceProps*,
                                                        RenderTargetReleaseProc, ReleaseContext) {
    return nullptr;
}

void SkSurface::flushAndSubmit(bool syncCpu) {
    this->flush(BackendSurfaceAccess::kNoAccess, GrFlushInfo());
}

#endif
