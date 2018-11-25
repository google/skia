/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SampleSlide.h"

#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkKey.h"
#include "SkOSFile.h"
#include "SkStream.h"

using namespace sk_app;

SampleSlide::SampleSlide(const SkViewFactory* factory)
    : fViewFactory(factory)
    , fClick(nullptr) {
    SkView* view = (*factory)();
    SampleCode::RequestTitle(view, &fName);
    view->unref();
}

SampleSlide::~SampleSlide() { delete fClick; }

void SampleSlide::draw(SkCanvas* canvas) {
    SkASSERT(fView);
    fView->draw(canvas);
}

void SampleSlide::load(SkScalar winWidth, SkScalar winHeight) {
    fView.reset((*fViewFactory)());
    fView->setVisibleP(true);
    fView->setClipToBounds(false);
    fView->setSize(winWidth, winHeight);
}

void SampleSlide::unload() {
    fView.reset();
}

bool SampleSlide::onChar(SkUnichar c) {
    if (!fView) {
        return false;
    }
    SkEvent evt(gCharEvtName);
    evt.setFast32(c);
    return fView->doQuery(&evt);
}

bool SampleSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state,
                          uint32_t modifiers) {
    // map to SkView modifiers
    unsigned modifierKeys = 0;
    modifierKeys |= (modifiers & Window::kShift_ModifierKey) ? kShift_SkModifierKey : 0;
    modifierKeys |= (modifiers & Window::kControl_ModifierKey) ? kControl_SkModifierKey : 0;
    modifierKeys |= (modifiers & Window::kOption_ModifierKey) ? kOption_SkModifierKey : 0;
    modifierKeys |= (modifiers & Window::kCommand_ModifierKey) ? kCommand_SkModifierKey : 0;

    bool handled = false;
    switch (state) {
        case Window::kDown_InputState: {
            delete fClick;
            fClick = fView->findClickHandler(SkIntToScalar(x), SkIntToScalar(y), modifierKeys);
            if (fClick) {
                SkView::DoClickDown(fClick, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Window::kMove_InputState: {
            if (fClick) {
                SkView::DoClickMoved(fClick, x, y, modifierKeys);
                handled = true;
            }
            break;
        }
        case Window::kUp_InputState: {
            if (fClick) {
                SkView::DoClickUp(fClick, x, y, modifierKeys);
                delete fClick;
                fClick = nullptr;
                handled = true;
            }
            break;
        }
    }

    return handled;
}
