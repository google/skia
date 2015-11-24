/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "VisualSKPBench.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

VisualSKPBench::VisualSKPBench(const char* name, const SkPicture* pic)
    : fPic(SkRef(pic))
    , fCullRect(fPic->cullRect().roundOut())
    , fName(name) {
    fUniqueName.printf("%s", name);
}

const char* VisualSKPBench::onGetName() {
    return fName.c_str();
}

const char* VisualSKPBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

bool VisualSKPBench::isSuitableFor(Backend backend) {
    return backend != kNonRendering_Backend;
}

SkIPoint VisualSKPBench::onGetSize() {
    return SkIPoint::Make(fCullRect.width(), fCullRect.height());
}

void VisualSKPBench::onDraw(int loops, SkCanvas* canvas) {
    bool isOffset = SkToBool(fCullRect.left() | fCullRect.top());
    if (isOffset) {
        canvas->save();
        canvas->translate(SkIntToScalar(-fCullRect.left()), SkIntToScalar(-fCullRect.top()));
    }

    for (int i = 0; i < loops; i++) {
        canvas->drawPicture(fPic);
#if SK_SUPPORT_GPU
        // Ensure the GrContext doesn't batch across draw loops.
        if (GrContext* context = canvas->getGrContext()) {
            context->flush();
        }
#endif
    }

    if (isOffset) {
        canvas->restore();
    }
}
