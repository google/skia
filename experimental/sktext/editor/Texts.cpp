// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Texts.h"

using namespace skia::text;

namespace skia {
namespace editor {

void DynamicText::paint(SkCanvas* canvas) {

    Paint painter;
    painter.paint(canvas, this->fOffset, this->fFormattedText.get(), this->fDecorations);
}

void EditableText::paint(SkCanvas* canvas) {

    DynamicText::paint(canvas);
    this->fSelection->paint(canvas, this->fOffset);
}

} // namespace editor
} // namespace skia
