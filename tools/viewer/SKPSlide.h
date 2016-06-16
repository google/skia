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
    SKPSlide(const SkString& name, const SkString& path);
    ~SKPSlide() override;

    SkISize getDimensions() const override { return fCullRect.size(); }

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

private:
    SkString               fPath;
    sk_sp<const SkPicture> fPic;
    SkIRect                fCullRect;
};

#endif
