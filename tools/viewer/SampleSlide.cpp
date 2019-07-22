/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SampleSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"

namespace {
class SampleSlide : public Slide {
    const SampleFactory fSampleFactory;
    std::unique_ptr<Sample> fSample;

    SkISize getDimensions() const override { return fSample->windowSize().toCeil(); }

    void draw(SkCanvas* canvas) override {
        SkASSERT(fSample);
        fSample->drawSample(canvas);
    }
    void load(SkScalar winWidth, SkScalar winHeight) override {
        fSample.reset(fSampleFactory());
        fSample->setWindowSize({winWidth, winHeight});
    }
    void resize(SkScalar winWidth, SkScalar winHeight) override {
        fSample->setWindowSize({winWidth, winHeight});
    }
    void unload() override { fSample = nullptr; }

    bool animate(double nanos) override { return fSample->animate(nanos); }

    bool onChar(SkUnichar c) override { return fSample && fSample->onChar(c); }

    bool onMouse(SkScalar x, SkScalar y, InputState state, ModifierKey modifiers) override {
        return fSample && fSample->mouse({x, y}, state, modifiers);
    }

public:
    SampleSlide(SampleFactory factory) : fSampleFactory(factory) {
        std::unique_ptr<Sample> sample(factory());
        fName = sample->name();
    }
};
}  // namespace
sk_sp<Slide> MakeSampleSlide(SampleFactory f) { return sk_sp<Slide>(new SampleSlide(f)); }
