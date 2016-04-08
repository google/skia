/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKPSlide_DEFINED
#define SKPSlide_DEFINED

#include "Slide.h"
#include "SkPicture.h"

class SKPSlide : public Slide {
public:
    SKPSlide(const char* name, sk_sp<const SkPicture> pic);
    ~SKPSlide() override;

    void draw(SkCanvas* canvas) override;

private:
    sk_sp<const SkPicture> fPic;
    SkIRect                fCullRect;
};

#endif
