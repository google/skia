// Copyright 2021 Google LLC.
#ifndef Selection_DEFINED
#define Selection_DEFINED
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

class Selection {
public:
    Selection(SkColor color) : fTextRanges(), fGlyphRanges(), fGlyphBoxes() {
        fPaint.setColor(color);
        fPaint.setAlphaf(0.3f);
    }

    void select(TextRange range, SkRect rect);

    void clear() {
        fGlyphBoxes.clear();
        fTextRanges.clear();
    }

    void paint(SkCanvas* canvas, SkPoint xy);

private:
    SkPaint fPaint;
    std::vector<TextRange> fTextRanges;
    std::vector<GlyphRange> fGlyphRanges;
    std::vector<SkRect> fGlyphBoxes;
};

} // namespace editor
} // namespace skia
#endif // Selection_DEFINED
