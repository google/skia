// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include  "samplecode/ClickHandler.h"

bool ClickHandler::click(SkPoint point, ClickState clickState, ModifierKey modifierKeys) {
    switch (clickState) {
        case ClickState::kDown:
            fClick.reset(this->onFindClickHandler(point, modifierKeys));
            if (!fClick) {
                return false;
            }
            fClick->fPrev = fClick->fCurr = fClick->fOrig = point;
            this->onClick(fClick.get(), clickState, modifierKeys);
            return true;
        case ClickState::kMoved:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                return this->onClick(fClick.get(), clickState, modifierKeys);
            }
            return false;
        case ClickState::kUp:
            if (fClick) {
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr = point;
                bool result = this->onClick(fClick.get(), clickState, modifierKeys);
                fClick = nullptr;
                return result;
            }
            return false;
    }
    SkASSERT(false);
    return false;
}
