/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurface_Base.h"
#include "SkImagePriv.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkMallocPixelRef.h"

class SkSurface_Raster : public SkSurface_Base {
public:
    SkSurface_Raster(const SkImageInfo&, void*, size_t rb,
                     void (*releaseProc)(void* pixels, void* context), void* context,
                     const SkSurfaceProps*);
    SkSurface_Raster(const SkImageInfo& info, sk_sp<SkPixelRef>, const SkSurfaceProps*);

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot() override;
    void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) override;
    void onCopyOnWrite(ContentChangeMode) override;
    void onRestoreBackingMutability() override;

private:
    SkBitmap    fBitmap;
    size_t      fRowBytes;
    bool        fWeOwnThePixels;

    typedef SkSurface_Base INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool SkSurfaceValidateRasterInfo(const SkImageInfo& info, size_t rowBytes) {
    if (info.isEmpty()) {
        return false;
    }

    static const size_t kMaxTotalSize = SK_MaxS32;

    int shift = 0;
    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
            if (info.colorSpace()) {
                return false;
            }
            shift = 0;
            break;
        case kRGB_565_SkColorType:
            if (info.colorSpace()) {
                return false;
            }
            shift = 1;
            break;
        case kN32_SkColorType:
            if (info.colorSpace() && !info.colorSpace()->gammaCloseToSRGB()) {
                return false;
            }
            shift = 2;
            break;
        case kRGBA_F16_SkColorType:
            if (info.colorSpace() && !info.colorSpace()->gammaIsLinear()) {
                return false;
            }
            shift = 3;
            break;
        default:
            return false;
    }

    if (kIgnoreRowBytesValue == rowBytes) {
        return true;
    }

    uint64_t minRB = (uint64_t)info.width() << shift;
    if (minRB > rowBytes) {
        return false;
    }

    size_t alignedRowBytes = rowBytes >> shift << shift;
    if (alignedRowBytes != rowBytes) {
        return false;
    }

    uint64_t size = sk_64_mul(info.height(), rowBytes);
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
    fBitmap.installPixels(info, pixels, rb, nullptr, releaseProc, context);
    fRowBytes = 0;              // don't need to track the rowbytes
    fWeOwnThePixels = false;    // We are "Direct"
}

SkSurface_Raster::SkSurface_Raster(const SkImageInfo& info, sk_sp<SkPixelRef> pr,
                                   const SkSurfaceProps* props)
    : INHERITED(pr->width(), pr->height(), props)
{
    fBitmap.setInfo(info, pr->rowBytes());
    fRowBytes = pr->rowBytes(); // we track this, so that subsequent re-allocs will match
    fBitmap.setPixelRef(std::move(pr), 0, 0);
    fWeOwnThePixels = true;
}

SkCanvas* SkSurface_Raster::onNewCanvas() { return new SkCanvas(fBitmap, this->props()); }

sk_sp<SkSurface> SkSurface_Raster::onNewSurface(const SkImageInfo& info) {
    return SkSurface::MakeRaster(info, &this->props());
}

void SkSurface_Raster::onDraw(SkCanvas* canvas, SkScalar x, SkScalar y,
                              const SkPaint* paint) {
    canvas->drawBitmap(fBitmap, x, y, paint);
}

sk_sp<SkImage> SkSurface_Raster::onNewImageSnapshot() {
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

void SkSurface_Raster::onRestoreBackingMutability() {
    SkASSERT(!this->hasCachedImage());  // Shouldn't be any snapshots out there.
    if (SkPixelRef* pr = fBitmap.pixelRef()) {
        pr->restoreMutability();
    }
}

void SkSurface_Raster::onCopyOnWrite(ContentChangeMode mode) {
    // are we sharing pixelrefs with the image?
    sk_sp<SkImage> cached(this->refCachedImage());
    SkASSERT(cached);
    if (SkBitmapImageGetPixelRef(cached.get()) == fBitmap.pixelRef()) {
        SkASSERT(fWeOwnThePixels);
        if (kDiscard_ContentChangeMode == mode) {
            fBitmap.allocPixels();
        } else {
            SkBitmap prev(fBitmap);
            fBitmap.allocPixels();
            SkASSERT(prev.info() == fBitmap.info());
            SkASSERT(prev.rowBytes() == fBitmap.rowBytes());
            memcpy(fBitmap.getPixels(), prev.getPixels(), fBitmap.getSafeSize());
        }
        SkASSERT(fBitmap.rowBytes() == fRowBytes);  // be sure we always use the same value

        // Now fBitmap is a deep copy of itself (and therefore different from
        // what is being used by the image. Next we update the canvas to use
        // this as its backend, so we can't modify the image's pixels anymore.
        SkASSERT(this->getCachedCanvas());
        this->getCachedCanvas()->getDevice()->replaceBitmapBackendForRasterSurface(fBitmap);
    }
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

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeZeroed(info, rowBytes, nullptr);
    if (!pr) {
        return nullptr;
    }
    if (rowBytes) {
        SkASSERT(pr->rowBytes() == rowBytes);
    }
    return sk_make_sp<SkSurface_Raster>(info, std::move(pr), props);
}
