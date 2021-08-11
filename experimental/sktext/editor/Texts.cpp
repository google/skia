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

    Paint painter;

    if (fSelection->isEmpty()) {
        painter.paint(canvas, this->fOffset, this->fFormattedText.get(), fDecorations);
    } else {
        auto decorations = mergeSelectionIntoDecorations();
        painter.paint(canvas, this->fOffset, this->fFormattedText.get(), SkSpan<DecoratedBlock>(decorations.data(), decorations.size()));
    }
}

SkTArray<DecoratedBlock> EditableText::mergeSelectionIntoDecorations() {
    SkTArray<DecoratedBlock> merged;
    merged.reserve_back(fDecorations.size() + fSelection->count());

    size_t indexDecor = 0ul;                        // index in fDecorations
    size_t decorPos = 0ul;
    for (auto& selected : fSelection->fTextRanges) {
        // Add all the decoration blocks that are placed before the selected block
        DecoratedBlock& decor = fDecorations[indexDecor];
        while (indexDecor < fDecorations.size()) {
            decor = fDecorations[indexDecor++];
            if (decorPos + decor.charCount >= selected.fStart) {
                break;
            }
            // The entire decoration block is before
            merged.emplace_back(decor);
            decorPos += decor.charCount;
        }

        auto lastDecorPos = decorPos;
        if (selected.fStart > decorPos) {
            // The decoration block is has a part that is before the selection so we add it
            merged.emplace_back(selected.fStart - decorPos, decor.foregroundPaint, decor.backgroundPaint);
            decorPos = selected.fStart;
        }
        SkASSERT(decorPos == selected.fStart);

        // So the next decoration intesects the selection (and the selection wins)
        merged.emplace_back(selected.width(), fSelection->fForeground, fSelection->fBackground);
        decorPos += selected.width();
        SkASSERT(decorPos == selected.fEnd);

        if (lastDecorPos + decor.charCount > selected.fEnd) {
            // We still need to add the rest of the decoration block
            merged.emplace_back(lastDecorPos + decor.charCount - selected.fEnd, decor.foregroundPaint, decor.backgroundPaint);
            decorPos += lastDecorPos + decor.charCount - selected.fEnd;
        }
    }
    return merged;
}

} // namespace editor
} // namespace skia
