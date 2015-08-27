/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTestImageFilters.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// Simple helper canvas that "takes ownership" of the provided device, so that
// when this canvas goes out of scope, so will its device. Could be replaced
// with the following:
//
//  SkCanvas canvas(device);
//  SkAutoTUnref<SkBaseDevice> aur(device);
//
class OwnDeviceCanvas : public SkCanvas {
public:
    OwnDeviceCanvas(SkBaseDevice* device) : SkCanvas(device) {
        SkSafeUnref(device);
    }
};

///////////////////////////////////////////////////////////////////////////////

bool SkDownSampleImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                            const Context&,
                                            SkBitmap* result, SkIPoint*) const {
    SkScalar scale = fScale;
    if (scale > SK_Scalar1 || scale <= 0) {
        return false;
    }

    int dstW = SkScalarRoundToInt(src.width() * scale);
    int dstH = SkScalarRoundToInt(src.height() * scale);
    if (dstW < 1) {
        dstW = 1;
    }
    if (dstH < 1) {
        dstH = 1;
    }

    SkBitmap tmp;

    // downsample
    {
        SkBaseDevice* dev = proxy->createDevice(dstW, dstH);
        if (nullptr == dev) {
            return false;
        }
        OwnDeviceCanvas canvas(dev);
        SkPaint paint;

        paint.setFilterQuality(kLow_SkFilterQuality);
        canvas.scale(scale, scale);
        canvas.drawBitmap(src, 0, 0, &paint);
        tmp = dev->accessBitmap(false);
    }

    // upscale
    {
        SkBaseDevice* dev = proxy->createDevice(src.width(), src.height());
        if (nullptr == dev) {
            return false;
        }
        OwnDeviceCanvas canvas(dev);

        canvas.drawBitmapRect(tmp, SkRect::MakeIWH(src.width(), src.height()), nullptr);
        *result = dev->accessBitmap(false);
    }
    return true;
}

SkFlattenable* SkDownSampleImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return Create(buffer.readScalar(), common.getInput(0));
}

void SkDownSampleImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fScale);
}

#ifndef SK_IGNORE_TO_STRING
void SkDownSampleImageFilter::toString(SkString* str) const {
    str->appendf("SkDownSampleImageFilter: (");
    str->append(")");
}
#endif
