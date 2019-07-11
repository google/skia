/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SampleSlide_DEFINED
#define SampleSlide_DEFINED

#include "samplecode/Sample.h"
#include "tools/viewer/Slide.h"

class SampleSlide : public Slide {
public:
    SampleSlide(const SampleFactory factory);
    ~SampleSlide() override;

    SkISize getDimensions() const override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void resize(SkScalar winWidth, SkScalar winHeight) override {
        fSample->setSize(winWidth, winHeight);
    }
    void unload() override;
    bool animate(double) override;

    bool onChar(SkUnichar c) override;
    bool onMouse(SkScalar x, SkScalar y, sk_app::Window::InputState state,
                 ModifierKey modifiers) override;

private:
    const SampleFactory fSampleFactory;
    sk_sp<Sample> fSample;
    Sample::Click* fClick;
};

#endif
