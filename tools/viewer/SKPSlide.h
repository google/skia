/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKPSlide_DEFINED
#define SKPSlide_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "tools/viewer/Slide.h"

#include <memory>

class SkCanvas;
class SkPicture;
class SkStream;
class SkString;

class SKPSlide : public Slide {
public:
    SKPSlide(const SkString& name, const SkString& path);
    SKPSlide(const SkString& name, std::unique_ptr<SkStream>);
    ~SKPSlide() override;

    SkISize getDimensions() const override { return fCullRect.size(); }

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

private:
    std::unique_ptr<SkStream> fStream;
    sk_sp<const SkPicture> fPic;
    SkIRect fCullRect;
};

#endif
