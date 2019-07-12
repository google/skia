/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SampleSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "src/core/SkOSFile.h"

using namespace sk_app;

SampleSlide::SampleSlide(const SampleFactory factory) : fSampleFactory(factory) {
    std::unique_ptr<Sample> sample(factory());
    fName = sample->name();
}

SampleSlide::~SampleSlide() {}

SkISize SampleSlide::getDimensions() const  {
    return SkISize::Make(SkScalarCeilToInt(fSample->width()), SkScalarCeilToInt(fSample->height()));
}

bool SampleSlide::animate(double nanos) { return fSample->animate(nanos); }

void SampleSlide::draw(SkCanvas* canvas) {
    SkASSERT(fSample);
    fSample->draw(canvas);
}

void SampleSlide::load(SkScalar winWidth, SkScalar winHeight) {
    fSample.reset(fSampleFactory());
    fSample->setSize(winWidth, winHeight);
}

void SampleSlide::unload() {
    fSample.reset();
}

bool SampleSlide::onChar(SkUnichar c) {
    return fSample && fSample->onChar(c);
}

bool SampleSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state,
                          ModifierKey modifierKeys) {
    static_assert((Sample::InputState)Window::kDown_InputState == Sample::InputState::kDown, "");
    static_assert((Sample::InputState)Window::kUp_InputState   == Sample::InputState::kUp,   "");
    static_assert((Sample::InputState)Window::kMove_InputState == Sample::InputState::kMove, "");
    return fSample && fSample->mouse({x, y}, (Sample::InputState)state, modifierKeys);
}
