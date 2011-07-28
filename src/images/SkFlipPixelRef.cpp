
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkFlipPixelRef.h"
#include "SkFlattenable.h"
#include "SkRegion.h"

SkFlipPixelRef::SkFlipPixelRef(SkBitmap::Config config, int width, int height)
: fFlipper(width, height) {
    fConfig = config;
    fSize = SkBitmap::ComputeSize(config, width, height);
    fStorage = sk_malloc_throw(fSize << 1);
    fPage0 = fStorage;
    fPage1 = (char*)fStorage + fSize;
}

SkFlipPixelRef::~SkFlipPixelRef() {
    sk_free(fStorage);
}

const SkRegion& SkFlipPixelRef::beginUpdate(SkBitmap* device) {
    void*       writeAddr;
    const void* readAddr;
    this->getFrontBack(&readAddr, &writeAddr);

    device->setConfig(fConfig, fFlipper.width(), fFlipper.height());
    device->setPixels(writeAddr);

    SkRegion    copyBits;
    const SkRegion& dirty = fFlipper.update(&copyBits);

    SkFlipPixelRef::CopyBitsFromAddr(*device, copyBits, readAddr);
    return dirty;
}

void SkFlipPixelRef::endUpdate() {
    this->swapPages();
}

///////////////////////////////////////////////////////////////////////////////

void* SkFlipPixelRef::onLockPixels(SkColorTable** ct) {
    fMutex.acquire();
    *ct = NULL;
    return fPage0;
}

void SkFlipPixelRef::onUnlockPixels() {
    fMutex.release();
}

void SkFlipPixelRef::swapPages() {
    fMutex.acquire();
    SkTSwap<void*>(fPage0, fPage1);
    this->notifyPixelsChanged();
    fMutex.release();
}

void SkFlipPixelRef::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    
    buffer.write32(fSize);
    // only need to write page0
    buffer.writePad(fPage0, fSize);
}

SkFlipPixelRef::SkFlipPixelRef(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer, NULL) {
    fSize = buffer.readU32();
    fStorage = sk_malloc_throw(fSize << 1);
    fPage0 = fStorage;
    fPage1 = (char*)fStorage + fSize;
    buffer.read(fPage0, fSize);
}

SkPixelRef* SkFlipPixelRef::Create(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkFlipPixelRef, (buffer));
}

static SkPixelRef::Registrar reg("SkFlipPixelRef",
                                 SkFlipPixelRef::Create);

///////////////////////////////////////////////////////////////////////////////

static void copyRect(const SkBitmap& dst, const SkIRect& rect,
                     const void* srcAddr, int shift) {
    const size_t offset = rect.fTop * dst.rowBytes() + (rect.fLeft << shift);
    char* dstP = static_cast<char*>(dst.getPixels()) + offset;
    const char* srcP = static_cast<const char*>(srcAddr) + offset;
    const size_t rb = dst.rowBytes();
    const size_t bytes = rect.width() << shift;
    
    int height = rect.height();
    while (--height >= 0) {
        memcpy(dstP, srcP, bytes);
        dstP += rb;
        srcP += rb;
    }
}

static int getShift(SkBitmap::Config config) {
    switch (config) {
        case SkBitmap::kARGB_8888_Config:
            return 2;
        case SkBitmap::kRGB_565_Config:
        case SkBitmap::kARGB_4444_Config:
            return 1;
        case SkBitmap::kIndex8_Config:
        case SkBitmap::kA8_Config:
            return 0;
        default:
            return -1;  // signal not supported
    }
}

void SkFlipPixelRef::CopyBitsFromAddr(const SkBitmap& dst, const SkRegion& clip,
                                      const void* srcAddr) {
    const int shift = getShift(dst.config());
    if (shift < 0) {
        return;
    }
    
    const SkIRect bounds = {0, 0, dst.width(), dst.height()};
    SkRegion::Cliperator iter(clip, bounds);
    
    while (!iter.done()) {
        copyRect(dst, iter.rect(), srcAddr, shift);
        iter.next();
    }
}
