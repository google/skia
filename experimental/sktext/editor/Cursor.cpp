// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Cursor.h"

using namespace skia::text;

namespace skia {
namespace editor {

std::unique_ptr<Cursor> Cursor::Make() { return std::make_unique<Cursor>(); }

Cursor::Cursor() {
    fLinePaint.setColor(SK_ColorGRAY);
    fLinePaint.setAntiAlias(true);

    fRectPaint.setColor(DEFAULT_CURSOR_COLOR);
    fRectPaint.setStyle(SkPaint::kStroke_Style);
    fRectPaint.setStrokeWidth(2);
    fRectPaint.setAntiAlias(true);

    fXY = SkPoint::Make(0, 0);
    fSize = SkSize::Make(0, 0);
    fBlink = true;
}

void Cursor::paint(SkCanvas* canvas) {

    if (fBlink) {
       canvas->drawRect(SkRect::MakeXYWH(fXY.fX, fXY.fY, DEFAULT_CURSOR_WIDTH, fSize.fHeight), fRectPaint);
    } else {
        //canvas->drawLine(fXY + xy, fXY + xy + SkPoint::Make(1, fSize.fHeight), fLinePaint);
    }
}

} // namespace editor
} // namespace skia
