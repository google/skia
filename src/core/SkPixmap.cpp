/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkConfig8888.h"
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
