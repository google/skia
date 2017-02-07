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
#include "SkColorSpace_Base.h"
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
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(encoded));
    if (!codec) {
        return;
    }

    fOriginalBitmap.allocPixels(codec->getInfo());
    codec->getPixels(codec->getInfo(), fOriginalBitmap.getPixels(), fOriginalBitmap.rowBytes());

    SkImageInfo xformedInfo = codec->getInfo().makeColorSpace(
            SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named));
    fXformedBitmap.allocPixels(xformedInfo);
    codec->getPixels(xformedInfo, fXformedBitmap.getPixels(), fXformedBitmap.rowBytes());
}

void ImageSlide::unload() {
    fImage.reset(nullptr);
}
