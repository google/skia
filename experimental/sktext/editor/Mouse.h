// Copyright 2021 Google LLC.
#ifndef Mouse_DEFINED
#define Mouse_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Defaults.h"
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace editor {

using namespace skia::text;

class Mouse {
    const SkMSec MAX_DBL_TAP_INTERVAL = 300;
    const float MAX_DBL_TAP_DISTANCE = 100;
public:
    Mouse() : fMouseDown(false), fLastTouchPoint(), fLastTouchTime() { }

    void down();
    void up();
    void clearTouchInfo() {
        fLastTouchPoint = SkPoint::Make(0, 0);
        fLastTouchTime = 0.0;
    }
    bool isDown() { return fMouseDown; }
    bool isDoubleClick(SkPoint touch);

private:
    bool fMouseDown;
    SkPoint fLastTouchPoint;
    double fLastTouchTime;
};

} // namespace editor
} // namespace skia
#endif // Mouse_DEFINED
