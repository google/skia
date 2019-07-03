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

SampleSlide::SampleSlide(const SampleFactory factory)
    : fSampleFactory(factory)
    , fClick(nullptr)
{
    sk_sp<Sample> sample(factory());
    fName = sample->name();
}

SampleSlide::~SampleSlide() { delete fClick; }

SkISize SampleSlide::getDimensions() const  {
    return SkISize::Make(SkScalarCeilToInt(fSample->width()), SkScalarCeilToInt(fSample->height()));
}

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
    if (!fSample) {
        return false;
    }
    Sample::Event evt(Sample::kCharEvtName);
    evt.setFast32(c);
    return fSample->doQuery(&evt);
}

bool SampleSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state,
                          uint32_t modifiers) {
    // map to Sample::Click modifiers
    unsigned modifierKeys = 0;
    modifierKeys |= (modifiers & Window::kShift_ModifierKey) ? Sample::Click::kShift_ModifierKey : 0;
    modifierKeys |= (modifiers & Window::kControl_ModifierKey) ? Sample::Click::kControl_ModifierKey : 0;
    modifierKeys |= (modifiers & Window::kOption_ModifierKey) ? Sample::Click::kOption_ModifierKey : 0;
    modifierKeys |= (modifiers & Window::kCommand_ModifierKey) ? Sample::Click::kCommand_ModifierKey : 0;

    bool handled = false;
    switch (state) {
        case Window::kDown_InputState: {
            delete fClick;
            fClick = fSample->findClickHandler(SkIntToScalar(x), SkIntToScalar(y), modifierKeys);
            if (fClick) {
                Sample::DoClickDown(fClick, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Window::kMove_InputState: {
            if (fClick) {
                Sample::DoClickMoved(fClick, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Window::kUp_InputState: {
            if (fClick) {
                Sample::DoClickUp(fClick, x, y, modifierKeys);
                delete fClick;
                fClick = nullptr;
                handled = true;
            }
            break;
        }
    }

    return handled;
}
