/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ImageSlide_DEFINED
#define ImageSlide_DEFINED

#include "Slide.h"
#include "SkPicture.h"
#include "SkImage.h"

static const char* kImageColorXformMetaData = "ImageColorSpaceXform";

class ImageSlide : public Slide {
public:
    ImageSlide(const SkString& name, const SkString& path);

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

private:
    SkString               fPath;
    sk_sp<const SkImage>   fImage;
    SkBitmap               fOriginalBitmap;
    SkBitmap               fXformedBitmap;
};

#endif
