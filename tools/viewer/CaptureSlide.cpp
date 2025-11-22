/*
* Copyright 2025 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkDebug.h"
#include "src/capture/SkCapture.h"
#include "tools/viewer/CaptureSlide.h"

CaptureSlide::CaptureSlide(const SkString& name, const SkString& path) {
    auto data = SkData::MakeFromFileName(path.c_str());
    fCapture = SkCapture::MakeFromData(data);
    if (fCapture) {
        fMetadata = fCapture->getMetadata();
    } else {
        SkDebugf("Couldn't load capture %s", path.c_str());
    }
}

CaptureSlide::~CaptureSlide() {}

void CaptureSlide::draw(SkCanvas* canvas) {
    if (fCapture) {
        auto focusPicture = fCapture->getPicture(fCurrentPictureIndex);
        auto bounds = focusPicture->cullRect().roundOut();
        canvas->clipIRect(bounds, SkClipOp::kIntersect);
        canvas->drawPicture(fCapture->getPicture(fCurrentPictureIndex));
    }
}

bool CaptureSlide::animate(double) {
    if (fInvalidate) {
        fInvalidate = false;
        return true;
    }
    return fInvalidate;
}

void CaptureSlide::load(SkScalar, SkScalar) {
}

void CaptureSlide::unload() {
    fCapture.reset(nullptr);
}

SkISize CaptureSlide::getDimensions() const {
    return {0, 0};
}

bool CaptureSlide::onChar(SkUnichar c) {
    switch (c) {
    case 'N':
        fCurrentPictureIndex = (fCurrentPictureIndex + 1) % fMetadata.numPictures;
        fInvalidate = true;
        return true;
    case 'P':
        fCurrentPictureIndex = (fCurrentPictureIndex + fMetadata.numPictures - 1) % fMetadata.numPictures;
        fInvalidate = true;
        return true;
    }

    return Slide::onChar(c);
}
