/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef ImageSlide_DEFINED
#define ImageSlide_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "tools/viewer/Slide.h"

class ImageSlide : public Slide {
public:
    ImageSlide(const SkString& name, const SkString& path);

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

private:
    SkString         fPath;
    sk_sp<SkImage>   fImage;
};

#endif
