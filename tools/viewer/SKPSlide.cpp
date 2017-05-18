/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SKPSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkStream.h"

SKPSlide::SKPSlide(const SkString& name, const SkString& path) : fPath(path) {
    fName = name;
}

SKPSlide::~SKPSlide() {}

void SKPSlide::draw(SkCanvas* canvas) {
    if (fPic.get()) {
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
}

static sk_sp<SkPicture> read_picture(const char path[]) {
    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path);
    if (!stream) {
        SkDebugf("Could not read %s.\n", path);
        return nullptr;
    }

    auto pic = SkPicture::MakeFromStream(stream.get());
    if (!pic) {
        SkDebugf("Could not read %s as an SkPicture.\n", path);
    }
    return pic;
}

void SKPSlide::load(SkScalar, SkScalar) {
    fPic = read_picture(fPath.c_str());
    fCullRect = fPic->cullRect().roundOut();
}

void SKPSlide::unload() {
    fPic.reset(nullptr);
}
