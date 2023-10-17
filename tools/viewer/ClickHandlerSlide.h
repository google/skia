/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ClickHandlerSlide_DEFINED
#define ClickHandlerSlide_DEFINED

#include "tools/SkMetaData.h"
#include "tools/viewer/Slide.h"

#include "include/core/SkPoint.h"

#include <functional>

/**
 * Provides a higher level abstraction for click handling than the Slide base class. A Click object
 * is is used to track the state of the mouse over time.
 */
class ClickHandlerSlide : public Slide {
public:
    // Click handling
    class Click {
    public:
        Click() {}
        Click(std::function<bool(Click*)> f) : fFunc(std::move(f)), fHasFunc(true) {}
        virtual ~Click() = default;

        SkPoint fOrig = {0, 0};
        SkPoint fPrev = {0, 0};
        SkPoint fCurr = {0, 0};

        skui::InputState  fState        = skui::InputState::kDown;
        skui::ModifierKey fModifierKeys = skui::ModifierKey::kNone;

        SkMetaData fMeta;

        std::function<bool(Click*)> fFunc;

        bool fHasFunc = false;
    };

    bool onMouse(SkScalar x, SkScalar y,
                 skui::InputState clickState,
                 skui::ModifierKey modifierKeys) final;

protected:
    /**
     * Return a Click object to handle the click. onClick will be called repeatedly with the latest
     * mouse state tracked on the Click object until it returns false.
     */
    virtual Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) = 0;

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*) = 0;

private:
    std::unique_ptr<Click> fClick;
};

#endif
