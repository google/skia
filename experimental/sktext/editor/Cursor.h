// Copyright 2021 Google LLC.
#ifndef Cursor_DEFINED
#define Cursor_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Defaults.h"
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/base/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace editor {

using namespace skia::text;

class Cursor {
public:
    static std::unique_ptr<Cursor> Make();
    Cursor();
    virtual ~Cursor() = default;
    void place(SkPoint xy, SkSize size) {
        if (size.width() < DEFAULT_CURSOR_WIDTH) {
            size.fWidth = DEFAULT_CURSOR_WIDTH;
        }
        fXY = xy;
        fSize = size;
    }

    void place(SkRect rect) {
        if (rect.width() < DEFAULT_CURSOR_WIDTH) {
            rect.fRight = rect.fLeft + DEFAULT_CURSOR_WIDTH;
        }
        fXY = SkPoint::Make(rect.fLeft, rect.fTop);
        fSize = SkSize::Make(rect.width(), rect.height());
    }

    void blink() {
        fBlink = !fBlink;
    }

    SkPoint getPosition() const { return fXY; }
    SkPoint getCenterPosition() const {
        return fXY + SkPoint::Make(0, fSize.fHeight / 2);
    }

    void paint(SkCanvas* canvas);

private:
    SkPaint fLinePaint;
    SkPaint fRectPaint;
    SkPoint fXY;
    SkSize fSize;
    bool fBlink;
};

} // namespace editor
} // namespace skia
#endif // Cursor_DEFINED
