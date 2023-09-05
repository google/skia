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
#include "src/base/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace editor {

using namespace skia::text;

class Selection {
public:
    Selection(SkColor color) : fTextRanges(), fGlyphRanges(), fGlyphBoxes() {
        fBackground.setColor(color);
        fBackground.setAlphaf(0.3f);
    }

    void select(TextRange range, SkRect rect);

    void clear() {
        fGlyphBoxes.clear();
        fTextRanges.clear();
    }

    bool isEmpty() const { return fTextRanges.empty(); }
    size_t count() const { return fTextRanges.size(); }
    DecoratedBlock selected(size_t index) const { return DecoratedBlock(fTextRanges[index].width(), fForeground, fBackground); }

    void paint(SkCanvas* canvas, SkPoint xy);

private:
    friend class EditableText;
    SkPaint fForeground;
    SkPaint fBackground;
    std::vector<TextRange> fTextRanges;
    std::vector<GlyphRange> fGlyphRanges;
    std::vector<SkRect> fGlyphBoxes;
};

} // namespace editor
} // namespace skia
#endif // Selection_DEFINED
