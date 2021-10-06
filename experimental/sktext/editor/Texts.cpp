// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Texts.h"

using namespace skia::text;

namespace skia {
namespace editor {

void DynamicText::paint(SkCanvas* canvas) {
    if (!fDrawableText) {
        auto chunks = this->getDecorationChunks(fDecorations);
        fDrawableText = fWrappedText->prepareToDraw<DrawableText>(fUnicodeText.get(),
                                                                  PositionType::kGraphemeCluster,
                                                                  SkSpan<TextIndex>(chunks.data(), chunks.size()));
    }

    auto foregroundPaint = fDecorations[0].foregroundPaint;
    auto textBlobs = fDrawableText->getTextBlobs();
    for (auto& textBLob : textBlobs) {
        canvas->drawTextBlob(textBLob, 0, 0, foregroundPaint);
    }
}

std::vector<TextIndex> DynamicText::getDecorationChunks(SkSpan<DecoratedBlock> decorations) const {
    std::vector<TextIndex> result;
    TextIndex textIndex = 0;
    for (auto& decoration : decorations) {
        textIndex += decoration.charCount;
        result.emplace_back(textIndex);
    }
    return result;
}

void EditableText::paint(SkCanvas* canvas) {

    if (fSelection->isEmpty()) {
        DynamicText::paint(canvas);
    } else {
        auto decorations = mergeSelectionIntoDecorations();
        auto chunks = this->getDecorationChunks(SkSpan<DecoratedBlock>(decorations.data(), decorations.size()));
        fDrawableText = fWrappedText->prepareToDraw<DrawableText>(fUnicodeText.get(),
                                                                  PositionType::kGraphemeCluster,
                                                                  SkSpan<TextIndex>(chunks.data(), chunks.size()));
    }
    auto foregroundPaint = fDecorations[0].foregroundPaint;
    auto textBlobs = fDrawableText->getTextBlobs();
    for (auto& textBLob : textBlobs) {
        canvas->drawTextBlob(textBLob, 0, 0, foregroundPaint);
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

        // So the next decoration intersects the selection (and the selection wins)
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
