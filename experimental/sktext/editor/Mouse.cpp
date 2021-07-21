// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Mouse.h"

using namespace skia::text;

namespace skia {
namespace editor {

void Mouse::down() {
    fMouseDown = true;
}

void Mouse::up() {
    fMouseDown = false;
}

bool Mouse::isDoubleClick(SkPoint touch) {
    if ((touch - fLastTouchPoint).length() > MAX_DBL_TAP_DISTANCE) {
        fLastTouchPoint = touch;
        fLastTouchTime = SkTime::GetMSecs();
        return false;
    }
    double now = SkTime::GetMSecs();
    if (now - fLastTouchTime > MAX_DBL_TAP_INTERVAL) {
        fLastTouchPoint = touch;
        fLastTouchTime = SkTime::GetMSecs();
        return false;
    }

    clearTouchInfo();
    return true;
}
} // namespace editor
} // namespace skia
