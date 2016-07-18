/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "ImageSlide.h"

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpace.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMetaData.h"

ImageSlide::ImageSlide(const SkString& name, const SkString& path) : fPath(path) {
    fName = name;
}

SkISize ImageSlide::getDimensions() const {
    return fImage ? fImage->dimensions() : SkISize::Make(0, 0);
}

void ImageSlide::draw(SkCanvas* canvas) {
    if (canvas->getMetaData().hasBool(kImageColorXformMetaData, true)) {
        canvas->drawBitmap(fXformedBitmap, 0, 0);
    } else {
        // skbug.com/5428
        // drawImage() and drawBitmap() behave differently in sRGB mode.
        // canvas->drawImage(fImage.get(), 0, 0);
        canvas->drawBitmap(fOriginalBitmap, 0, 0);
    }
}

void ImageSlide::load(SkScalar, SkScalar) {
    sk_sp<SkData> encoded = SkData::MakeFromFileName(fPath.c_str());
    fImage = SkImage::MakeFromEncoded(encoded);
    fImage->asLegacyBitmap(&fOriginalBitmap, SkImage::kRO_LegacyBitmapMode);

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded.get()));
    sk_sp<SkColorSpace> srcSpace = sk_ref_sp(codec->getColorSpace());
    sk_sp<SkColorSpace> dstSpace = SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named);
    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcSpace, dstSpace);
    fOriginalBitmap.deepCopyTo(&fXformedBitmap);
    uint32_t* row = (uint32_t*) fXformedBitmap.getPixels();
    for (int y = 0; y < fXformedBitmap.height(); y++) {
        xform->xform_RGB1_8888(row, row, fXformedBitmap.width());
        row = SkTAddOffset<uint32_t>(row, fXformedBitmap.rowBytes());
    }
    fXformedBitmap.notifyPixelsChanged(); // This is needed for the deepCopy
}

void ImageSlide::unload() {
    fImage.reset(nullptr);
}
