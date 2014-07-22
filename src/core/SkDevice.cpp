/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDevice.h"
#include "SkMetaData.h"

SkBaseDevice::SkBaseDevice()
    : fLeakyProperties(SkDeviceProperties::MakeDefault())
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;
}

SkBaseDevice::SkBaseDevice(const SkDeviceProperties& deviceProperties)
    : fLeakyProperties(deviceProperties)
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;
}

SkBaseDevice::~SkBaseDevice() {
    delete fMetaData;
}

SkBaseDevice* SkBaseDevice::createCompatibleDevice(const SkImageInfo& info) {
    return this->onCreateDevice(info, kGeneral_Usage);
}

SkBaseDevice* SkBaseDevice::createCompatibleDeviceForSaveLayer(const SkImageInfo& info) {
    return this->onCreateDevice(info, kSaveLayer_Usage);
}

SkMetaData& SkBaseDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

SkImageInfo SkBaseDevice::imageInfo() const {
    return SkImageInfo::MakeUnknown();
}

const SkBitmap& SkBaseDevice::accessBitmap(bool changePixels) {
    const SkBitmap& bitmap = this->onAccessBitmap();
    if (changePixels) {
        bitmap.notifyPixelsChanged();
    }
    return bitmap;
}

SkSurface* SkBaseDevice::newSurface(const SkImageInfo&) { return NULL; }

const void* SkBaseDevice::peekPixels(SkImageInfo*, size_t*) { return NULL; }

void SkBaseDevice::drawDRRect(const SkDraw& draw, const SkRRect& outer,
                              const SkRRect& inner, const SkPaint& paint) {
    SkPath path;
    path.addRRect(outer);
    path.addRRect(inner);
    path.setFillType(SkPath::kEvenOdd_FillType);

    const SkMatrix* preMatrix = NULL;
    const bool pathIsMutable = true;
    this->drawPath(draw, path, paint, preMatrix, pathIsMutable);
}

bool SkBaseDevice::readPixels(const SkImageInfo& info, void* dstP, size_t rowBytes, int x, int y) {
#ifdef SK_DEBUG
    SkASSERT(info.width() > 0 && info.height() > 0);
    SkASSERT(dstP);
    SkASSERT(rowBytes >= info.minRowBytes());
    SkASSERT(x >= 0 && y >= 0);

    const SkImageInfo& srcInfo = this->imageInfo();
    SkASSERT(x + info.width() <= srcInfo.width());
    SkASSERT(y + info.height() <= srcInfo.height());
#endif
    return this->onReadPixels(info, dstP, rowBytes, x, y);
}

bool SkBaseDevice::writePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                               int x, int y) {
#ifdef SK_DEBUG
    SkASSERT(info.width() > 0 && info.height() > 0);
    SkASSERT(pixels);
    SkASSERT(rowBytes >= info.minRowBytes());
    SkASSERT(x >= 0 && y >= 0);

    const SkImageInfo& dstInfo = this->imageInfo();
    SkASSERT(x + info.width() <= dstInfo.width());
    SkASSERT(y + info.height() <= dstInfo.height());
#endif
    return this->onWritePixels(info, pixels, rowBytes, x, y);
}

bool SkBaseDevice::onWritePixels(const SkImageInfo&, const void*, size_t, int, int) {
    return false;
}

bool SkBaseDevice::onReadPixels(const SkImageInfo&, void*, size_t, int x, int y) {
    return false;
}

void* SkBaseDevice::accessPixels(SkImageInfo* info, size_t* rowBytes) {
    SkImageInfo tmpInfo;
    size_t tmpRowBytes;
    if (NULL == info) {
        info = &tmpInfo;
    }
    if (NULL == rowBytes) {
        rowBytes = &tmpRowBytes;
    }
    return this->onAccessPixels(info, rowBytes);
}

void* SkBaseDevice::onAccessPixels(SkImageInfo* info, size_t* rowBytes) {
    return NULL;
}

void SkBaseDevice::EXPERIMENTAL_optimize(const SkPicture* picture) {
    // The base class doesn't perform any analysis but derived classes may
}

bool SkBaseDevice::EXPERIMENTAL_drawPicture(SkCanvas* canvas, const SkPicture* picture) {
    // The base class doesn't perform any accelerated picture rendering
    return false;
}
