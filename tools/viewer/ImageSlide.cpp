/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/ImageSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/private/base/SkAssert.h"

#include <utility>

ImageSlide::ImageSlide(const SkString& name, const SkString& path) : fPath(path) {
    fName = name;
}

ImageSlide::ImageSlide(const SkString& name, sk_sp<SkImage> image)
        : fImage(std::move(image)), fRetainImage(true) {
    fName = name;
}

SkISize ImageSlide::getDimensions() const {
    return fImage ? fImage->dimensions() : SkISize::Make(0, 0);
}

void ImageSlide::draw(SkCanvas* canvas) {
    SkASSERT(fImage);
    canvas->drawImage(fImage, 0, 0);
}

void ImageSlide::load(SkScalar, SkScalar) {
    if (fRetainImage) {
        SkASSERT(fImage);
    } else {
        sk_sp<SkData> encoded = SkData::MakeFromFileName(fPath.c_str());
        fImage = SkImages::DeferredFromEncodedData(encoded);
    }
}

void ImageSlide::unload() {
    if (!fRetainImage) {
        fImage.reset(nullptr);
    }
}
