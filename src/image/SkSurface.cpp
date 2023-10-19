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
class GrRecordingContext;  // IWYU pragma: keep
class SkPaint;
class GrSurfaceCharacterization;
namespace skgpu { namespace graphite { class Recorder; } }

SkSurfaceProps::SkSurfaceProps() : fFlags(0), fPixelGeometry(kUnknown_SkPixelGeometry) {}

SkSurfaceProps::SkSurfaceProps(uint32_t flags, SkPixelGeometry pg)
    : fFlags(flags), fPixelGeometry(pg)
{}

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
                                                  /*readAlpha=*/false,
                                                  std::move(dstColorSpace),
                                                  srcRect,
                                                  dstSize,
                                                  rescaleGamma,
                                                  rescaleMode,
                                                  callback,
                                                  context);
}

void SkSurface::asyncRescaleAndReadPixelsYUVA420(SkYUVColorSpace yuvColorSpace,
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
                                                  /*readAlpha=*/true,
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

GrRecordingContext* SkSurface::recordingContext() const {
    return asConstSB(this)->onGetRecordingContext();
}

skgpu::graphite::Recorder* SkSurface::recorder() const { return asConstSB(this)->onGetRecorder(); }

bool SkSurface::wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                     bool deleteSemaphoresAfterWait) {
    return asSB(this)->onWait(numSemaphores, waitSemaphores, deleteSemaphoresAfterWait);
}

bool SkSurface::characterize(GrSurfaceCharacterization* characterization) const {
    return asConstSB(this)->onCharacterize(characterization);
}

bool SkSurface::isCompatible(const GrSurfaceCharacterization& characterization) const {
    return asConstSB(this)->onIsCompatible(characterization);
}
