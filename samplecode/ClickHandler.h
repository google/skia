// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef ClickHandler_DEFINED
#define ClickHandler_DEFINED

#include "include/core/SkPoint.h"
#include "src/utils/SkMetaData.h"
#include "tools/ModifierKey.h"
#include "tools/ClickState.h"

#include <memory>

class ClickHandler {
public:
    ClickHandler() = default;
    virtual ~ClickHandler() = default;
    //  Click handling
    class Click {
    public:
        SkPoint fOrig = {0, 0};
        SkPoint fPrev = {0, 0};
        SkPoint fCurr = {0, 0};
        SkMetaData fMeta;

        Click() = default;
        virtual ~Click() = default;
    private:
        Click(const Click&) = delete;
        Click& operator=(const Click&) = delete;
    };

    bool click(SkPoint, ClickState, ModifierKey);

private:
    std::unique_ptr<Click> fClick;

    /** Override this if you might handle the click */
    virtual Click* onFindClickHandler(SkPoint, ModifierKey) { return nullptr; }

    /** Override to track clicks. Return true as long as you want to track the pen/mouse. */
    virtual bool onClick(Click*, ClickState, ModifierKey) { return false; }

    ClickHandler(const ClickHandler&) = delete;
    ClickHandler& operator=(const ClickHandler&) = delete;
};

#endif  // ClickHandler_DEFINED
