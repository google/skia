/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkSurface_Raster.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCapabilities.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkSurfacePriv.h"

#include <cstdint>
#include <cstring>
#include <utility>

class SkImage;
class SkPaint;
class SkPixmap;
class SkSurfaceProps;

bool SkSurfaceValidateRasterInfo(const SkImageInfo& info, size_t rowBytes) {
    if (!SkImageInfoIsValid(info)) {
        return false;
    }

    if (kIgnoreRowBytesValue == rowBytes) {
        return true;
    }

    if (!info.validRowBytes(rowBytes)) {
        return false;
    }

    uint64_t size = sk_64_mul(info.height(), rowBytes);
    static const size_t kMaxTotalSize = SK_MaxS32;
    if (size > kMaxTotalSize) {
        return false;
    }

    return true;
}

SkSurface_Raster::SkSurface_Raster(const SkImageInfo& info, void* pixels, size_t rb,
                                   void (*releaseProc)(void* pixels, void* context), void* context,
                                   const SkSurfaceProps* props)
    : INHERITED(info, props)
{
    fBitmap.installPixels(info, pixels, rb, releaseProc, context);
    fWeOwnThePixels = false;    // We are "Direct"
}

SkSurface_Raster::SkSurface_Raster(const SkImageInfo& info, sk_sp<SkPixelRef> pr,
                                   const SkSurfaceProps* props)
    : INHERITED(pr->width(), pr->height(), props)
{
    fBitmap.setInfo(info, pr->rowBytes());
    fBitmap.setPixelRef(std::move(pr), 0, 0);
    fWeOwnThePixels = true;
}

SkCanvas* SkSurface_Raster::onNewCanvas() { return new SkCanvas(fBitmap, this->props()); }

sk_sp<SkSurface> SkSurface_Raster::onNewSurface(const SkImageInfo& info) {
    return SkSurface::MakeRaster(info, &this->props());
}

void SkSurface_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkSamplingOptions& sampling, const SkPaint* paint) {
    canvas->drawImage(fBitmap.asImage().get(), x, y, sampling, paint);
}

sk_sp<SkImage> SkSurface_Raster::onNewImageSnapshot(const SkIRect* subset) {
    if (subset) {
        SkASSERT(SkIRect::MakeWH(fBitmap.width(), fBitmap.height()).contains(*subset));
        SkBitmap dst;
        dst.allocPixels(fBitmap.info().makeDimensions(subset->size()));
        SkAssertResult(fBitmap.readPixels(dst.pixmap(), subset->left(), subset->top()));
        dst.setImmutable(); // key, so MakeFromBitmap doesn't make a copy of the buffer
        return dst.asImage();
    }

    SkCopyPixelsMode cpm = kIfMutable_SkCopyPixelsMode;
    if (fWeOwnThePixels) {
        // SkImage_raster requires these pixels are immutable for its full lifetime.
        // We'll undo this via onRestoreBackingMutability() if we can avoid the COW.
        if (SkPixelRef* pr = fBitmap.pixelRef()) {
            pr->setTemporarilyImmutable();
        }
    } else {
        cpm = kAlways_SkCopyPixelsMode;
    }

    // Our pixels are in memory, so read access on the snapshot SkImage could be cheap.
    // Lock the shared pixel ref to ensure peekPixels() is usable.
    return SkMakeImageFromRasterBitmap(fBitmap, cpm);
}

void SkSurface_Raster::onWritePixels(const SkPixmap& src, int x, int y) {
    fBitmap.writePixels(src, x, y);
}

void SkSurface_Raster::onRestoreBackingMutability() {
    SkASSERT(!this->hasCachedImage());  // Shouldn't be any snapshots out there.
    if (SkPixelRef* pr = fBitmap.pixelRef()) {
        pr->restoreMutability();
    }
}

bool SkSurface_Raster::onCopyOnWrite(ContentChangeMode mode) {
    // are we sharing pixelrefs with the image?
    sk_sp<SkImage> cached(this->refCachedImage());
    SkASSERT(cached);
    if (SkBitmapImageGetPixelRef(cached.get()) == fBitmap.pixelRef()) {
        SkASSERT(fWeOwnThePixels);
        if (kDiscard_ContentChangeMode == mode) {
            if (!fBitmap.tryAllocPixels()) {
                return false;
            }
        } else {
            SkBitmap prev(fBitmap);
            if (!fBitmap.tryAllocPixels()) {
                return false;
            }
            SkASSERT(prev.info() == fBitmap.info());
            SkASSERT(prev.rowBytes() == fBitmap.rowBytes());
            memcpy(fBitmap.getPixels(), prev.getPixels(), fBitmap.computeByteSize());
        }

        // Now fBitmap is a deep copy of itself (and therefore different from
        // what is being used by the image. Next we update the canvas to use
        // this as its backend, so we can't modify the image's pixels anymore.
        SkASSERT(this->getCachedCanvas());
        this->getCachedCanvas()->baseDevice()->replaceBitmapBackendForRasterSurface(fBitmap);
    }
    return true;
}

sk_sp<const SkCapabilities> SkSurface_Raster::onCapabilities() {
    return SkCapabilities::RasterBackend();
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkSurface::MakeRasterDirectReleaseProc(const SkImageInfo& info, void* pixels,
        size_t rb, void (*releaseProc)(void* pixels, void* context), void* context,
        const SkSurfaceProps* props) {
    if (nullptr == releaseProc) {
        context = nullptr;
    }
    if (!SkSurfaceValidateRasterInfo(info, rb)) {
        return nullptr;
    }
    if (nullptr == pixels) {
        return nullptr;
    }

    return sk_make_sp<SkSurface_Raster>(info, pixels, rb, releaseProc, context, props);
}

sk_sp<SkSurface> SkSurface::MakeRasterDirect(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                             const SkSurfaceProps* props) {
    return MakeRasterDirectReleaseProc(info, pixels, rowBytes, nullptr, nullptr, props);
}

sk_sp<SkSurface> SkSurface::MakeRaster(const SkImageInfo& info, size_t rowBytes,
                                       const SkSurfaceProps* props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, rowBytes);
    if (!pr) {
        return nullptr;
    }
    if (rowBytes) {
        SkASSERT(pr->rowBytes() == rowBytes);
    }
    return sk_make_sp<SkSurface_Raster>(info, std::move(pr), props);
}

sk_sp<SkSurface> SkSurface::MakeRasterN32Premul(int width, int height,
                                                const SkSurfaceProps* surfaceProps) {
    return MakeRaster(SkImageInfo::MakeN32Premul(width, height), surfaceProps);
}
