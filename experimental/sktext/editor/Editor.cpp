// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Editor.h"
#include "experimental/sktext/src/Paint.h"

using namespace skia::text;

namespace skia {
namespace editor {

std::unique_ptr<Editor> Editor::Make(const std::u16string& text, SkSize size) {
    return std::make_unique<Editor>(text, size);
}

Editor::Editor(const std::u16string& text, SkSize size)
        : fDefaultPositionType(PositionType::kGraphemeCluster)
        , fInsertMode(true) {

    fParent = nullptr;
    fCursor = Cursor::Make();
    fMouse = std::make_unique<Mouse>();
    {
        SkPaint foreground; foreground.setColor(DEFAULT_TEXT_FOREGROUND);
        SkPaint background; background.setColor(DEFAULT_TEXT_BACKGROUND);
        static FontBlock textBlock(text.size(), sk_make_sp<TrivialFontChain>("Roboto", 40, SkFontStyle::Normal()));
        static DecoratedBlock textDecor(text.size(), foreground, background);
        auto textSize = SkSize::Make(size.width(), size.height() - DEFAULT_STATUS_HEIGHT);
        fEditableText = std::make_unique<EditableText>(
                text, SkPoint::Make(0, 0), textSize,
                SkSpan<FontBlock>(&textBlock, 1), SkSpan<DecoratedBlock>(&textDecor, 1),
                DEFAULT_TEXT_DIRECTION, DEFAULT_TEXT_ALIGN);
    }
    {
        SkPaint foreground; foreground.setColor(DEFAULT_STATUS_FOREGROUND);
        SkPaint background; background.setColor(DEFAULT_STATUS_BACKGROUND);
        std::u16string status = u"This is the status line";
        static FontBlock statusBlock(status.size(), sk_make_sp<TrivialFontChain>("Roboto", 20, SkFontStyle::Normal()));
        static DecoratedBlock statusDecor(status.size(), foreground, background);
        auto statusPoint = SkPoint::Make(0, size.height() - DEFAULT_STATUS_HEIGHT);
        fStatus = std::make_unique<DynamicText>(
                status, statusPoint, SkSize::Make(size.width(), SK_ScalarInfinity),
                SkSpan<FontBlock>(&statusBlock, 1), SkSpan<DecoratedBlock>(&statusDecor, 1),
                        DEFAULT_TEXT_DIRECTION, TextAlign::kCenter);
    }
    // Place the cursor at the end of the output text
    // (which is the end of the text for LTR and the beginning of the text for RTL
    // or possibly something in the middle for a combination of LTR & RTL)
    // In order to get that position we look for a position outside of the text
    // and that will give us the last glyph on the line
    auto endOfText = fEditableText->lastElement(fDefaultPositionType);
    //fEditableText->recalculateBoundaries(endOfText);
    fCursor->place(endOfText.fBoundaries);
}

void Editor::update() {

    if (fEditableText->isValid()) {
        return;
    }

    // Update the (shift it to point at the grapheme edge)
    auto position = fEditableText->adjustedPosition(fDefaultPositionType, fCursor->getCenterPosition());
    //fEditableText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);

    // TODO: Update the mouse
    fMouse->clearTouchInfo();
}

// Moving the cursor by the output grapheme clusters (shifting to another line if necessary)
// We don't want to move by the input text indexes because then we will have to take in account LTR/RTL
bool Editor::moveCursor(skui::Key key) {
    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fEditableText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);

    if (key == skui::Key::kLeft) {
        position = fEditableText->previousElement(position);
    } else if (key == skui::Key::kRight) {
        position = fEditableText->nextElement(position);
    } else if (key == skui::Key::kHome) {
        position = fEditableText->firstElement(PositionType::kGraphemeCluster);
    } else if (key == skui::Key::kEnd) {
        position = fEditableText->lastElement(PositionType::kGraphemeCluster);
    } else if (key == skui::Key::kUp) {
        // Move one line up (if possible)
        if (position.fLineIndex == 0) {
            return false;
        }
        auto prevLine = fEditableText->getLine(position.fLineIndex - 1);
        cursorPosition.offset(0, - prevLine.fBounds.height());
        position = fEditableText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);
    } else if (key == skui::Key::kDown) {
        // Move one line down (if possible)
        if (position.fLineIndex == fEditableText->lineCount() - 1) {
            return false;
        }
        auto nextLine = fEditableText->getLine(position.fLineIndex + 1);
        cursorPosition.offset(0, nextLine.fBounds.height());
        position = fEditableText->adjustedPosition(PositionType::kGraphemeCluster, cursorPosition);
     }

    // Place the cursor at the new position
    //fEditableText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);
    this->invalidate();

    return true;
}

void Editor::onPaint(SkSurface* surface) {
    SkCanvas* canvas = surface->getCanvas();
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
    canvas->drawColor(SK_ColorWHITE);
    this->paint(canvas);
}

void Editor::onResize(int width, int height) {
    if (SkISize{fWidth, fHeight} != SkISize{width, height}) {
        fHeight = height;
        if (width != fWidth) {
            fWidth = width;
        }
        this->invalidate();
    }
}

bool Editor::onChar(SkUnichar c, skui::ModifierKey modi) {
    using sknonstd::Any;

    modi &= ~skui::ModifierKey::kFirstPress;
    if (!Any(modi & (skui::ModifierKey::kControl |
                     skui::ModifierKey::kOption |
                     skui::ModifierKey::kCommand))) {
        if (((unsigned)c < 0x7F && (unsigned)c >= 0x20) || c == 0x000A) {
            insertCodepoint(c);
            return true;
        }
    }
    static constexpr skui::ModifierKey kCommandOrControl =
            skui::ModifierKey::kCommand | skui::ModifierKey::kControl;
    if (Any(modi & kCommandOrControl) && !Any(modi & ~kCommandOrControl)) {
        return false;
    }
    return false;
}

bool Editor::deleteElement(skui::Key key) {

    if (fEditableText->isEmpty()) {
        return false;
    }

    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fEditableText->adjustedPosition(fDefaultPositionType, cursorPosition);
    TextRange textRange = position.fTextRange;

    // IMPORTANT: We assume that a single element (grapheme cluster) does not cross the run boundaries;
    // It's not exactly true but we are going to enforce in by breaking the grapheme by the run boundaries
    if (key == skui::Key::kBack) {
        // TODO: Make sure previous element moves smoothly over the line break
        position = fEditableText->previousElement(position);
        textRange = position.fTextRange;
        fCursor->place(position.fBoundaries);
    } else {
        // The cursor stays the the same place
    }

    fEditableText->removeElement(textRange);

    // Find the grapheme the cursor points to
    position = fEditableText->adjustedPosition(fDefaultPositionType, SkPoint::Make(position.fBoundaries.fLeft, position.fBoundaries.fTop));
    fCursor->place(position.fBoundaries);
    this->invalidate();

    return true;
}

bool Editor::insertCodepoint(SkUnichar unichar) {
    auto cursorPosition = fCursor->getCenterPosition();
    auto position = fEditableText->adjustedPosition(fDefaultPositionType, cursorPosition);

    if (fInsertMode) {
        fEditableText->insertElement(unichar, position.fTextRange.fStart);
    } else {
        fEditableText->replaceElement(unichar, position.fTextRange);
    }

    this->update();

    // Find the element the cursor points to
    position = fEditableText->adjustedPosition(fDefaultPositionType, cursorPosition);

    // Move the cursor to the next element
    position = fEditableText->nextElement(position);
    //fEditableText->recalculateBoundaries(position);
    fCursor->place(position.fBoundaries);

    this->invalidate();

    return true;
}

bool Editor::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {

    if (state != skui::InputState::kDown) {
        return false;
    }
    using sknonstd::Any;
    skui::ModifierKey ctrlAltCmd = modifiers & (skui::ModifierKey::kControl |
                                                skui::ModifierKey::kOption  |
                                                skui::ModifierKey::kCommand);
    //bool shift = Any(modifiers & (skui::ModifierKey::kShift));
    if (!Any(ctrlAltCmd)) {
        // no modifiers
        switch (key) {
            case skui::Key::kLeft:
            case skui::Key::kRight:
            case skui::Key::kUp:
            case skui::Key::kDown:
            case skui::Key::kHome:
            case skui::Key::kEnd:
                this->moveCursor(key);
                break;
            case skui::Key::kDelete:
            case skui::Key::kBack:
                this->deleteElement(key);
                return true;
            case skui::Key::kOK:
                return this->onChar(0x000A, modifiers);
            default:
                break;
        }
    }
    return false;
}

bool Editor::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {

    if (!fEditableText->contains(x, y)) {
        // We only support mouse on an editable area
    }
    if (skui::InputState::kDown == state) {
        auto position = fEditableText->adjustedPosition(fDefaultPositionType, SkPoint::Make(x, y));
        if (fMouse->isDoubleClick(SkPoint::Make(x, y))) {
            // Select the element
            fEditableText->select(position.fTextRange, position.fBoundaries);
            position.fBoundaries.fLeft = position.fBoundaries.fRight - DEFAULT_CURSOR_WIDTH;
            // Clear mouse
            fMouse->up();
        } else {
            // Clear selection
            fMouse->down();
            fEditableText->clearSelection();
        }

        fCursor->place(position.fBoundaries);
        this->invalidate();
        return true;
    }
    fMouse->up();
    return false;
}

void Editor::paint(SkCanvas* canvas) {

    fEditableText->paint(canvas);
    fCursor->paint(canvas);

    SkPaint background; background.setColor(DEFAULT_STATUS_BACKGROUND);
    canvas->drawRect(SkRect::MakeXYWH(0, fHeight - DEFAULT_STATUS_HEIGHT, fWidth, DEFAULT_STATUS_HEIGHT), background);
    fStatus->paint(canvas);
}

std::unique_ptr<Editor> Editor::MakeDemo(SkScalar width, SkScalar height) {

    std::u16string text0 = u"In a hole in the ground there lived a hobbit. Not a nasty, dirty, "
                            "wet hole full of worms and oozy smells.\nThis was a hobbit-hole and "
                            "that means good food, a warm hearth, and all the comforts of home.";

    return Editor::Make(text0, SkSize::Make(width, height));
}
} // namespace editor
} // namespace skia
