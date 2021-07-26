// Copyright 2021 Google LLC.
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"

namespace skia {
namespace text {
Line::Line(const Stretch& stretch, const Stretch& spaces, SkSTArray<1, size_t, true> visualOrder)
    : fTextStart(stretch.glyphStart())
    , fTextEnd(stretch.glyphEnd())
    , fWhitespacesEnd (spaces.glyphEnd())
    , fText(stretch.textRange())
    , fWhitespaces(spaces.textRange())
    , fTextWidth(stretch.width())
    , fSpacesWidth(spaces.width())
    , fRunsInVisualOrder(std::move(visualOrder)) {

    SkASSERT(stretch.isEmpty() ||
                    spaces.isEmpty() ||
        (stretch.glyphEnd() == spaces.glyphStart()));

    if (!stretch.isEmpty()) {
        this->fTextMetrics.merge(stretch.textMetrics());
    }
    if (!spaces.isEmpty()) {
        this->fTextMetrics.merge(spaces.textMetrics());
    }
}
} // namespace text
} // namespace skia
