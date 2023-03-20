/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/ClickHandlerSlide.h"

bool ClickHandlerSlide::onMouse(SkScalar x, SkScalar y,
                                skui::InputState clickState,
                                skui::ModifierKey modifierKeys) {
    auto dispatch = [this](Click* c) {
        return c->fHasFunc ? c->fFunc(c) : this->onClick(c);
    };

    switch (clickState) {
        case skui::InputState::kDown:
            fClick = nullptr;
            fClick.reset(this->onFindClickHandler(x, y, modifierKeys));
            if (!fClick) {
                return false;
            }
            fClick->fPrev = fClick->fCurr = fClick->fOrig = {x, y};
            fClick->fState = skui::InputState::kDown;
            fClick->fModifierKeys = modifierKeys;
            dispatch(fClick.get());
            return true;
        case skui::InputState::kMove:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = {x, y};
                fClick->fState = skui::InputState::kMove;
                fClick->fModifierKeys = modifierKeys;
                return dispatch(fClick.get());
            }
            return false;
        case skui::InputState::kUp:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = {x, y};
                fClick->fState = skui::InputState::kUp;
                fClick->fModifierKeys = modifierKeys;
                bool result = dispatch(fClick.get());
                fClick = nullptr;
                return result;
            }
            return false;
        default:
            // Ignore other cases
            SK_ABORT("Unexepected InputState");
    }

    SkUNREACHABLE;
}
