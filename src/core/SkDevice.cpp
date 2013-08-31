
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDevice.h"
#include "SkMetaData.h"

SK_DEFINE_INST_COUNT(SkBaseDevice)

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

SkBaseDevice* SkBaseDevice::createCompatibleDevice(SkBitmap::Config config,
                                                   int width, int height,
                                                   bool isOpaque) {
    return this->onCreateCompatibleDevice(config, width, height,
                                          isOpaque, kGeneral_Usage);
}

SkBaseDevice* SkBaseDevice::createCompatibleDeviceForSaveLayer(SkBitmap::Config config,
                                                               int width, int height,
                                                               bool isOpaque) {
    return this->onCreateCompatibleDevice(config, width, height,
                                          isOpaque, kSaveLayer_Usage);
}

SkMetaData& SkBaseDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
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
        tmp.setConfig(SkBitmap::kARGB_8888_Config, bitmap->width(),
                                                   bitmap->height());
        if (!tmp.allocPixels()) {
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
