/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDevice.h"
#include "SkMetaData.h"

#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    const SkCanvas::Config8888 SkBaseDevice::kPMColorAlias = SkCanvas::kBGRA_Premul_Config8888;
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    const SkCanvas::Config8888 SkBaseDevice::kPMColorAlias = SkCanvas::kRGBA_Premul_Config8888;
#else
    const SkCanvas::Config8888 SkBaseDevice::kPMColorAlias = (SkCanvas::Config8888) -1;
#endif

///////////////////////////////////////////////////////////////////////////////

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
#ifdef SK_SUPPORT_LEGACY_COMPATIBLEDEVICE_CONFIG
    // We call the old method to support older subclasses.
    // If they have, we return their device, else we use the new impl.
    SkBitmap::Config config = SkColorTypeToBitmapConfig(info.colorType());
    SkBaseDevice* dev = this->onCreateCompatibleDevice(config,
                                                       info.width(),
                                                       info.height(),
                                                       info.isOpaque(),
                                                       kGeneral_Usage);
    if (dev) {
        return dev;
    }
    // fall through to new impl
#endif
    return this->onCreateDevice(info, kGeneral_Usage);
}

SkBaseDevice* SkBaseDevice::createCompatibleDeviceForSaveLayer(const SkImageInfo& info) {
#ifdef SK_SUPPORT_LEGACY_COMPATIBLEDEVICE_CONFIG
    // We call the old method to support older subclasses.
    // If they have, we return their device, else we use the new impl.
    SkBitmap::Config config = SkColorTypeToBitmapConfig(info.colorType());
    SkBaseDevice* dev = this->onCreateCompatibleDevice(config,
                                                       info.width(),
                                                       info.height(),
                                                       info.isOpaque(),
                                                       kSaveLayer_Usage);
    if (dev) {
        return dev;
    }
    // fall through to new impl
#endif
    return this->onCreateDevice(info, kSaveLayer_Usage);
}

#ifdef SK_SUPPORT_LEGACY_COMPATIBLEDEVICE_CONFIG
SkBaseDevice* SkBaseDevice::createCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque) {
    SkImageInfo info = SkImageInfo::Make(width, height,
                                         SkBitmapConfigToColorType(config),
                                         isOpaque ? kOpaque_SkAlphaType
                                                  : kPremul_SkAlphaType);
    return this->createCompatibleDevice(info);
}
#endif

SkMetaData& SkBaseDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

// TODO: should make this guy pure-virtual.
SkImageInfo SkBaseDevice::imageInfo() const {
    return SkImageInfo::MakeUnknown(this->width(), this->height());
}

const SkBitmap& SkBaseDevice::accessBitmap(bool changePixels) {
    const SkBitmap& bitmap = this->onAccessBitmap();
    if (changePixels) {
        bitmap.notifyPixelsChanged();
    }
    return bitmap;
}

bool SkBaseDevice::readPixels(SkBitmap* bitmap, int x, int y,
                              SkCanvas::Config8888 config8888) {
    if (SkBitmap::kARGB_8888_Config != bitmap->config() ||
        NULL != bitmap->getTexture()) {
        return false;
    }

    const SkBitmap& src = this->accessBitmap(false);

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bitmap->width(),
                                              bitmap->height());
    SkIRect devbounds = SkIRect::MakeWH(src.width(), src.height());
    if (!srcRect.intersect(devbounds)) {
        return false;
    }

    SkBitmap tmp;
    SkBitmap* bmp;
    if (bitmap->isNull()) {
        if (!tmp.allocPixels(SkImageInfo::MakeN32Premul(bitmap->width(),
                                                        bitmap->height()))) {
            return false;
        }
        bmp = &tmp;
    } else {
        bmp = bitmap;
    }

    SkIRect subrect = srcRect;
    subrect.offset(-x, -y);
    SkBitmap bmpSubset;
    bmp->extractSubset(&bmpSubset, subrect);

    bool result = this->onReadPixels(bmpSubset,
                                     srcRect.fLeft,
                                     srcRect.fTop,
                                     config8888);
    if (result && bmp == &tmp) {
        tmp.swap(*bitmap);
    }
    return result;
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

bool SkBaseDevice::writePixelsDirect(const SkImageInfo& info, const void* pixels, size_t rowBytes,
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

bool SkBaseDevice::onReadPixels(const SkBitmap&, int x, int y, SkCanvas::Config8888) {
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

#ifdef SK_SUPPORT_LEGACY_WRITEPIXELSCONFIG
void SkBaseDevice::writePixels(const SkBitmap&, int x, int y, SkCanvas::Config8888) {}
#endif

void SkBaseDevice::EXPERIMENTAL_optimize(SkPicture* picture) {
    // The base class doesn't perform any analysis but derived classes may
}

bool SkBaseDevice::EXPERIMENTAL_drawPicture(const SkPicture& picture) {
    // The base class doesn't perform any accelerated picture rendering
    return false;
}
