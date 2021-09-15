// Copyright 2021 Google LLC.
#ifndef Texts_DEFINED
#define Texts_DEFINED
#include <sstream>
#include "experimental/sktext/editor/Cursor.h"
#include "experimental/sktext/editor/Defaults.h"
#include "experimental/sktext/editor/Mouse.h"
#include "experimental/sktext/editor/Selection.h"
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
// Shapes text once and then paints it at request (not keeping intermediate data)
// TODO: Keep only text blobs
class StaticText {
public:

    StaticText(std::u16string text, SkPoint offset, SkSize size, SkSpan<FontBlock> fontBlocks, TextDirection textDirection, TextAlign textAlign) {
        fOffset = offset;
        fSize = size;
        fFontBlocks = fontBlocks;
        fText = std::move(text);
        fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fShapedText = fUnicodeText->shape(fontBlocks, DEFAULT_TEXT_DIRECTION);
        fWrappedText = fShapedText->wrap(size.width(), size.height(), fUnicodeText->getUnicode());
        fFormattedText = fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
    }

    virtual ~StaticText() = default;

    virtual void paint(SkCanvas* canvas, SkSpan<DecoratedBlock> decors);

    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    sk_sp<FormattedText> fFormattedText;
    SkSize fSize;
    SkPoint fOffset;
    SkSpan<FontBlock> fFontBlocks;
};

// The text can change but it's not selectable/editable
class DynamicText {
public:
    DynamicText(std::u16string text,
                SkPoint offset, SkSize size,
                SkSpan<FontBlock> fontBlocks,
                SkSpan<DecoratedBlock> decorations,
                TextDirection textDirection, TextAlign textAlign) {
        fOffset = offset;
        fRequiredSize = size;
        fFontBlocks = fontBlocks;
        fDecorations = decorations;
        fTextAlign = textAlign;
        fTextDirection = textDirection;
        fText = std::move(text);
        fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fShapedText = fUnicodeText->shape(fontBlocks, fTextDirection);
        fWrappedText = fShapedText->wrap(size.width(), size.height(), fUnicodeText->getUnicode());
        fFormattedText = fWrappedText->format(fTextAlign, fTextDirection);
        fInvalidated = false;
    }

    virtual ~DynamicText() = default;

    bool contains(SkScalar x, SkScalar y) {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fRequiredSize.fWidth, fRequiredSize.fHeight).contains(x, y);
    }

    void invalidate() { fInvalidated = true; }
    bool isInvalidated() { return fInvalidated; }

    bool rebuild(std::u16string text) {
        if (!this->fFontBlocks.empty()) {
            SkASSERT(this->fFontBlocks.size() == 1);
            this->fFontBlocks[0].charCount = text.size();
        }
        this->fText = std::move(text);
        this->fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)this->fText.data(), this->fText.size()));
        this->fShapedText = fUnicodeText->shape(this->fFontBlocks, fTextDirection);
        this->fWrappedText = fShapedText->wrap(this->fRequiredSize.fWidth, this->fRequiredSize.fHeight, this->fUnicodeText->getUnicode());
        this->fFormattedText = fWrappedText->format(fTextAlign, fTextDirection);
        this->fActualSize = fFormattedText->actualSize();
        this->fInvalidated = false;

        return true;
    }

    SkScalar lineHeight(size_t index) const { return fFormattedText->line(index)->height(); }
    size_t lineCount() const { return fFormattedText->countLines(); }

    SkRect actualSize() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fActualSize.fWidth, fActualSize.fHeight);
    }

    virtual void paint(SkCanvas* canvas);

protected:
    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    sk_sp<FormattedText> fFormattedText;
    SkSize fRequiredSize;
    SkSize fActualSize;
    SkPoint fOffset;
    SkSpan<FontBlock> fFontBlocks;
    SkSpan<DecoratedBlock> fDecorations;
    TextAlign fTextAlign;
    TextDirection fTextDirection;
    bool fInvalidated;
};

// Text can change;  supports select/copy/paste
class EditableText : public DynamicText {
public:
    EditableText(std::u16string text, SkPoint offset, SkSize size, SkSpan<FontBlock> fontBlocks, SkSpan<DecoratedBlock> decorations, TextDirection textDirection, TextAlign textAlign)
        : DynamicText(text, offset, size, fontBlocks, decorations, textDirection, textAlign)
        , fSelection(std::make_unique<Selection>(DEFAULT_SELECTION_COLOR)) {
    }

    bool isEmpty() { return fText.empty(); }

    Position adjustedPosition(PositionType positionType, SkPoint point) const {
        return fFormattedText->adjustedPosition(positionType, point - fOffset);
    }

    Position adjustedPosition(PositionType positionType, TextIndex textIndex) const {
        return fFormattedText->adjustedPosition(positionType, textIndex);
    }

    bool recalculateBoundaries(Position& position) const {
        return fFormattedText->recalculateBoundaries(position);
    }

    const Line* line(size_t index) const { return fFormattedText->line(index); }

    Position previousElement(Position element) const { return fFormattedText->previousElement(element); }
    Position nextElement(Position current) const { return fFormattedText->nextElement(current); }
    Position firstElement(PositionType positionType) const { return fFormattedText->firstElement(positionType); }
    Position lastElement(PositionType positionType) const { return fFormattedText->lastElement(positionType); }

    bool isFirstOnTheLine(Position element) const { return fFormattedText->isFirstOnTheLine(element); }
    bool isLastOnTheLine(Position element) const { return fFormattedText->isLastOnTheLine(element); }

    void removeElement(TextRange toRemove) {
        std::u16string text;
        text.append(this->fText.substr(0, toRemove.fStart));
        text.append(this->fText.substr(toRemove.fEnd, std::u16string::npos));
        update(text);
    }

    void insertElement(SkUnichar unichar, TextIndex toInsert) {
        std::u16string text;
        text.append(fText.substr(0, toInsert));
        text.append(1, (unsigned)unichar);
        text.append(fText.substr(toInsert, std::u16string::npos));
        update(text);
    }

    void replaceElement(SkUnichar unichar, TextRange toReplace) {
        std::u16string text;
        text.append(fText.substr(0, toReplace.fStart));
        text.append(1, (unsigned)unichar);
        text.append(fText.substr(toReplace.fEnd, std::u16string::npos));
    }

    void update(std::u16string& text) {
        fText = text;
        // TODO: Update text styles
        SkASSERT(fFontBlocks.size() == 1);
        fFontBlocks[0].charCount = fText.size();
        SkASSERT(fDecorations.size() == 1);
        fDecorations[0].charCount = fText.size();
        // TODO: update all objects rather than recreate them
        rebuild(text);

        // TODO: Update the text selection
        fSelection->clear();
    }

    void select(TextRange text, SkRect boundaries) { fSelection->select(text, boundaries); }
    void clearSelection() { fSelection->clear(); }

    void paint(SkCanvas* canvas) override;

    SkTArray<DecoratedBlock> mergeSelectionIntoDecorations();

protected:
    std::unique_ptr<Selection> fSelection;
};

} // namespace editor
} // namespace skia
#endif // Texts_DEFINED
