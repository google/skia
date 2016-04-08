/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SKPSlide.h"

#include "SkCanvas.h"

SKPSlide::SKPSlide(const char* name, sk_sp<const SkPicture> pic)
    : fPic(pic)
    , fCullRect(fPic->cullRect().roundOut()) {
    fName = name;
}

SKPSlide::~SKPSlide() {}

void SKPSlide::draw(SkCanvas* canvas) {
    bool isOffset = SkToBool(fCullRect.left() | fCullRect.top());
    if (isOffset) {
        canvas->save();
        canvas->translate(SkIntToScalar(-fCullRect.left()), SkIntToScalar(-fCullRect.top()));
    }

    canvas->drawPicture(fPic.get());

    if (isOffset) {
        canvas->restore();
    }
}
