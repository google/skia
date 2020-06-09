/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRiveSlide_DEFINED
#define SkRiveSlide_DEFINED

#include "tools/viewer/Slide.h"

#if defined(SK_ENABLE_SKRIVE)

#include "experimental/skrive/include/SkRive.h"

class SkRiveSlide final : public Slide {
public:
    SkRiveSlide(const SkString& name, const SkString& path);
    ~SkRiveSlide() override;

private:
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void resize(SkScalar, SkScalar) override;

    SkISize getDimensions() const override;

    void draw(SkCanvas*) override;

    const SkString        fPath;

    sk_sp<skrive::SkRive> fRive;
    SkRect                fRiveBounds;
    SkSize                fWinSize;

    using INHERITED = Slide;
};

#endif // defined(SK_ENABLE_SKRIVE)
#endif // SkRiveSlide_DEFINED
