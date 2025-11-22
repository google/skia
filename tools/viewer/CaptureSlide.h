/*
* Copyright 2025 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef CaptureSlide_DEFINED
#define CaptureSlide_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "src/capture/SkCapture.h"
#include "tools/viewer/Slide.h"

class SkCanvas;
class SkPicture;
class SkStream;
class SkString;

class CaptureSlide : public Slide {
public:
    CaptureSlide(const SkString& name, const SkString& path);
    ~CaptureSlide() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    bool animate(double) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    bool onChar(SkUnichar) override;

private:
    sk_sp<SkCapture> fCapture;
    int fCurrentPictureIndex = 0;
    bool fInvalidate = false;
    SkCapture::Metadata fMetadata;
};

#endif
