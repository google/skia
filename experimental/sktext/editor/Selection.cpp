// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Selection.h"

using namespace skia::text;

namespace skia {
namespace editor {

void Selection::select(TextRange range, SkRect rect) {
    fGlyphBoxes.clear();
    fTextRanges.clear();

    fGlyphBoxes.emplace_back(rect);
    fTextRanges.emplace_back(range);
}

void Selection::paint(SkCanvas* canvas, SkPoint xy) {
    for (auto& box : fGlyphBoxes) {
        canvas->drawRect(
                SkRect::MakeXYWH(box.fLeft + xy.fX, box.fTop + xy.fY, box.width(), box.height()),
                fBackground);
    }
}

} // namespace editor
} // namespace skia
