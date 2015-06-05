/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkConfig8888.h"
#include "SkMask.h"
#include "SkPixmap.h"

void SkAutoPixmapUnlock::reset(const SkPixmap& pm, void (*unlock)(void*), void* ctx) {
    SkASSERT(pm.addr() != NULL);

    this->unlock();
    fPixmap = pm;
    fUnlockProc = unlock;
    fUnlockContext = ctx;
    fIsLocked = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void SkPixmap::reset() {
    fPixels = NULL;
    fCTable = NULL;
    fRowBytes = 0;
    fInfo = SkImageInfo::MakeUnknown();
}

void SkPixmap::reset(const SkImageInfo& info, const void* addr, size_t rowBytes, SkColorTable* ct) {
    if (addr) {
        SkASSERT(info.validRowBytes(rowBytes));
    }
    fPixels = addr;
    fCTable = ct;
    fRowBytes = rowBytes;
    fInfo = info;
}

bool SkPixmap::reset(const SkMask& src) {
    if (SkMask::kA8_Format == src.fFormat) {
        this->reset(SkImageInfo::MakeA8(src.fBounds.width(), src.fBounds.height()),
                    src.fImage, src.fRowBytes, NULL);
        return true;
    }
    this->reset();
    return false;
}

bool SkPixmap::extractSubset(SkPixmap* result, const SkIRect& subset) const {
    SkIRect srcRect, r;
    srcRect.set(0, 0, this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }
    
    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    const void* pixels = NULL;
    if (fPixels) {
        const size_t bpp = fInfo.bytesPerPixel();
        pixels = (const uint8_t*)fPixels + r.fTop * fRowBytes + r.fLeft * bpp;
    }
    result->reset(fInfo.makeWH(r.width(), r.height()), pixels, fRowBytes, fCTable);
    return true;
}

bool SkPixmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    if (kUnknown_SkColorType == requestedDstInfo.colorType()) {
        return false;
    }
    if (NULL == dstPixels || dstRB < requestedDstInfo.minRowBytes()) {
        return false;
    }
    if (0 == requestedDstInfo.width() || 0 == requestedDstInfo.height()) {
        return false;
    }
    
    SkIRect srcR = SkIRect::MakeXYWH(x, y, requestedDstInfo.width(), requestedDstInfo.height());
    if (!srcR.intersect(0, 0, this->width(), this->height())) {
        return false;
    }
    
    // the intersect may have shrunk info's logical size
    const SkImageInfo dstInfo = requestedDstInfo.makeWH(srcR.width(), srcR.height());
    
    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    dstPixels = ((char*)dstPixels - y * dstRB - x * dstInfo.bytesPerPixel());

    const SkImageInfo srcInfo = this->info().makeWH(dstInfo.width(), dstInfo.height());
    const void* srcPixels = this->addr(srcR.x(), srcR.y());
    return SkPixelInfo::CopyPixels(dstInfo, dstPixels, dstRB,
                                   srcInfo, srcPixels, this->rowBytes(), this->ctable());
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkAutoPixmapStorage::SkAutoPixmapStorage() : fStorage(NULL) {}

SkAutoPixmapStorage::~SkAutoPixmapStorage() {
    sk_free(fStorage);
}

bool SkAutoPixmapStorage::tryAlloc(const SkImageInfo& info) {
    if (fStorage) {
        sk_free(fStorage);
        fStorage = NULL;
    }
    
    size_t rb = info.minRowBytes();
    size_t size = info.getSafeSize(rb);
    if (0 == size) {
        return false;
    }
    fStorage = sk_malloc_flags(size, 0);
    if (NULL == fStorage) {
        return false;
    }
    this->reset(info, fStorage, rb);
    return true;
}

void SkAutoPixmapStorage::alloc(const SkImageInfo& info) {
    if (!this->tryAlloc(info)) {
        sk_throw();
    }
}
