// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef skui_ViewLayer_DEFINED
#define skui_ViewLayer_DEFINED

#include "tools/skui/InputState.h"
#include "tools/skui/Key.h"
#include "tools/skui/ModifierKey.h"

#include <cstdint>

class SkSurface;

namespace skui {
class ViewLayer {
public:
    ViewLayer() {}
    virtual ~ViewLayer() {}

    // return value of 'true' means 'I have handled this event'
    virtual bool onChar(int32_t unicodeCodepoint, skui::ModifierKey) { return false; }
    virtual bool onKey(skui::Key, skui::InputState, skui::ModifierKey) { return false; }
    virtual bool onMouse(int x, int y, skui::InputState, skui::ModifierKey) { return false; }
    virtual bool onMouseWheel(float delta, skui::ModifierKey) { return false; }
    virtual bool onTouch(intptr_t owner, skui::InputState, float x, float y) { return false; }

    virtual void onUIStateChanged(const char* stateName, const char* stateValue) {}
    virtual void onPrePaint() {}
    virtual void onPaint(SkSurface*) {}
    virtual void onResize(int width, int height) {}

private:
    ViewLayer(const ViewLayer&) = delete;
    ViewLayer& operator=(const ViewLayer&) = delete;
};
}
#endif  // skui_ViewLayer_DEFINED
