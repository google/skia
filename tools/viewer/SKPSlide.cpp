/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SKPSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"

SKPSlide::SKPSlide(const SkString& name, const SkString& path)
        : SKPSlide(name, SkStream::MakeFromFile(path.c_str())) {
}

SKPSlide::SKPSlide(const SkString& name, std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream)) {
    fName = name;
}

SKPSlide::~SKPSlide() {}

void SKPSlide::draw(SkCanvas* canvas) {
    if (fPic) {
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

void SKPSlide::load(SkScalar, SkScalar) {
    if (!fStream) {
        SkDebugf("No skp stream for slide %s.\n", fName.c_str());
        return;
    }
    fStream->rewind();
    fPic = SkPicture::MakeFromStream(fStream.get());
    if (!fPic) {
        SkDebugf("Could parse SkPicture from skp stream for slide %s.\n", fName.c_str());
        return;
    }
    fCullRect = fPic->cullRect().roundOut();
}

void SKPSlide::unload() {
    fPic.reset(nullptr);
}
