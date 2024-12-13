/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef AnimatedImageSlide_DEFINED
#define AnimatedImageSlide_DEFINED

#include "include/core/SkString.h"
#include "modules/skresources/include/SkResources.h"
#include "tools/viewer/Slide.h"

class SkCanvas;

class AnimatedImageSlide final : public Slide {
public:
    AnimatedImageSlide(const SkString& name, const SkString& path);

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    void draw(SkCanvas*) override;
    bool animate(double nanos) override;

private:
    const SkString                           fPath;
    sk_sp<skresources::MultiFrameImageAsset> fImageAsset;
    SkSize                                   fWinSize;

    double                                   fTimeBase = 0;
    float                                    fFrameMs  = 0;
};

#endif  // AnimatedImageSlide_DEFINED
