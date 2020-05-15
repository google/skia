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

#include "include/utils/SkDumpCanvas.h"

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
        SkDebugf("--- begin\n");
        SkDumpCanvas dc(1000, 1000);
        fPic->playback(&dc);
        SkDebugf("--- end\n");

        if (isOffset) {
            canvas->restore();
        }
    }
}

#include "include/core/SkStream.h"

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

    if (false) {
        SkString name(path);
        for (size_t i = 0; i < name.size(); ++i) {
            if (name[i] == '/') {
                name[i] = '_';
            }
        }
        name.append(".txt");
        SkDebugf("opening %s\n", name.c_str());

        SkDumpCanvas dc(1000, 1000);
        pic->playback(&dc);
        SkDebugf("\n");
    }

    return pic;
}

void SKPSlide::load(SkScalar, SkScalar) {
    fPic = read_picture(fPath.c_str());
    if (fPic) {
        fCullRect = fPic->cullRect().roundOut();
    }
}

void SKPSlide::unload() {
    fPic.reset(nullptr);
}
