/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCapabilities.h" // IWYU pragma: keep
#include "include/core/SkColorSpace.h"
#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkSurfacePriv.h"
#include "src/image/SkSurface_Base.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class GrBackendSemaphore;
class GrRecordingContext;
class SkPaint;
class SkSurfaceCharacterization;
namespace skgpu { class MutableTextureState; }
namespace skgpu { namespace graphite { class Recorder; } }

#if defined(SK_GANESH)
#include "include/gpu/GrBackendSurface.h"
#endif

SkSurfaceProps::SkSurfaceProps() : fFlags(0), fPixelGeometry(kUnknown_SkPixelGeometry) {}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, SkPixelGeometry pg)
    : fFlags(flags), fPixelGeometry(pg)
{}

SkSurfaceProps::SkSurfaceProps(const SkSurfaceProps&) = default;
SkSurfaceProps& SkSurfaceProps::operator=(const SkSurfaceProps&) = default;

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

sk_sp<const SkCapabilities> SkSurface::capabilities() {
    return asSB(this)->onCapabilities();
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

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Log.h"

sk_sp<SkImage> SkSurface::asImage() {
    if (asSB(this)->fCachedImage) {
        SKGPU_LOG_W("Intermingling makeImageSnapshot and asImage calls may produce "
                    "unexpected results. Please use either the old _or_ new API.");
    }

    return asSB(this)->onAsImage();
}

sk_sp<SkImage> SkSurface::makeImageCopy(const SkIRect* subset,
                                        skgpu::Mipmapped mipmapped) {
    if (asSB(this)->fCachedImage) {
        SKGPU_LOG_W("Intermingling makeImageSnapshot and makeImageCopy calls may produce "
                    "unexpected results. Please use either the old _or_ new API.");
    }

    return asSB(this)->onMakeImageCopy(subset, mipmapped);
}
#endif

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

skgpu::graphite::Recorder* SkSurface::recorder() {
    return asSB(this)->onGetRecorder();
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

#if defined(SK_GANESH)
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

void SkSurface::resolveMSAA() {
    asSB(this)->onResolveMSAA();
}

GrSemaphoresSubmitted SkSurface::flush(BackendSurfaceAccess access, const GrFlushInfo& flushInfo) {
    return asSB(this)->onFlush(access, flushInfo, nullptr);
}

GrSemaphoresSubmitted SkSurface::flush(const GrFlushInfo& info,
                                       const skgpu::MutableTextureState* newState) {
    return asSB(this)->onFlush(BackendSurfaceAccess::kNoAccess, info, newState);
}

void SkSurface::flush() {
    this->flush({});
}
#else
void SkSurface::flush() {} // Flush is a no-op for CPU surfaces

void SkSurface::flushAndSubmit(bool syncCpu) {}

// TODO(kjlubick, scroggo) Remove this once Android is updated.
sk_sp<SkSurface> SkSurface::MakeRenderTarget(GrRecordingContext*,
                                             skgpu::Budgeted,
                                             const SkImageInfo&,
                                             int,
                                             GrSurfaceOrigin,
                                             const SkSurfaceProps*,
                                             bool) {
    return nullptr;
}
#endif
