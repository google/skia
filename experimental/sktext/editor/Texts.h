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

        auto unicode = SkUnicode::Make();
        fUnicodeText = std::make_unique<UnicodeText>(std::move(unicode), SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fFontResolvedText = fUnicodeText->resolveFonts(fontBlocks);
        fShapedText = fFontResolvedText->shape(fUnicodeText.get(), DEFAULT_TEXT_DIRECTION);
        fWrappedText = fShapedText->wrap(fUnicodeText.get(), size.width(), size.height());
        fWrappedText->format(DEFAULT_TEXT_ALIGN, DEFAULT_TEXT_DIRECTION);
    }

    virtual ~StaticText() = default;

    virtual void paint(SkCanvas* canvas, SkSpan<DecoratedBlock> decors);

    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<FontResolvedText> fFontResolvedText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
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

        auto unicode = SkUnicode::Make();
        fUnicodeText = std::make_unique<UnicodeText>(std::move(unicode), SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fFontResolvedText = fUnicodeText->resolveFonts(fontBlocks);
        fShapedText = fFontResolvedText->shape(fUnicodeText.get(), fTextDirection);
        fWrappedText = fShapedText->wrap(fUnicodeText.get(), size.width(), size.height());
        fWrappedText->format(fTextAlign, fTextDirection);
    }

    virtual ~DynamicText() = default;

    bool contains(SkScalar x, SkScalar y) {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fRequiredSize.fWidth, fRequiredSize.fHeight).contains(x, y);
    }

    void invalidate() { fDrawableText = nullptr; }
    bool isValid() { return fDrawableText != nullptr; }

    std::vector<TextIndex> getDecorationChunks(SkSpan<DecoratedBlock> decorations) const;

    bool rebuild(std::u16string text) {
        if (!this->fFontBlocks.empty()) {
            SkASSERT(this->fFontBlocks.size() == 1);
            this->fFontBlocks[0].charCount = text.size();
        }
        this->fText = std::move(text);

        auto unicode = SkUnicode::Make();
        fUnicodeText = std::make_unique<UnicodeText>(std::move(unicode), SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
        fFontResolvedText = fUnicodeText->resolveFonts(fFontBlocks);
        fShapedText = fFontResolvedText->shape(fUnicodeText.get(), fTextDirection);
        fWrappedText = fShapedText->wrap(fUnicodeText.get(), this->fRequiredSize.fWidth, this->fRequiredSize.fHeight);
        fWrappedText->format(fTextAlign, fTextDirection);
        fSelectableText = fWrappedText->prepareToEdit(fUnicodeText.get());
        fDrawableText = nullptr;
        fActualSize = fWrappedText->actualSize();

        return true;
    }

    size_t lineCount() const { return fSelectableText->countLines(); }
    BoxLine getLine(size_t lineIndex) {
        SkASSERT(lineIndex < fSelectableText->countLines());
        return fSelectableText->getLine(lineIndex);
    }

    SkRect actualSize() const {
        return SkRect::MakeXYWH(fOffset.fX, fOffset.fY, fActualSize.fWidth, fActualSize.fHeight);
    }

    virtual void paint(SkCanvas* canvas);

protected:
    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<FontResolvedText> fFontResolvedText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    std::unique_ptr<DrawableText> fDrawableText;
    std::unique_ptr<SelectableText> fSelectableText;
    SkSize fRequiredSize;
    SkSize fActualSize;
    SkPoint fOffset;
    SkSpan<FontBlock> fFontBlocks;
    SkSpan<DecoratedBlock> fDecorations;
    TextAlign fTextAlign;
    TextDirection fTextDirection;
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
        return fSelectableText->adjustedPosition(positionType, point - fOffset);
    }

    //Position adjustedPosition(PositionType positionType, TextIndex textIndex) const {
    //    return fSelectableText->adjustedPosition(positionType, textIndex);
    //}

    Position previousElement(Position element) const { return fSelectableText->previousPosition(element); }
    Position nextElement(Position current) const { return fSelectableText->nextPosition(current); }
    Position firstElement(PositionType positionType) const { return fSelectableText->firstPosition(positionType); }
    Position lastElement(PositionType positionType) const { return fSelectableText->lastPosition(positionType); }

    bool isFirstOnTheLine(Position element) const { return fSelectableText->isFirstOnTheLine(element); }
    bool isLastOnTheLine(Position element) const { return fSelectableText->isLastOnTheLine(element); }

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
