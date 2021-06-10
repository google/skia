// Copyright 2021 Google LLC.
#include "experimental/sktext/editor/Editor.h"
#include "experimental/sktext/src/Paint.h"

namespace skia {
namespace text {

const TextDirection TEXT_DIRECTION = TextDirection::kLtr;
const TextAlign TEXT_ALIGN = TextAlign::kLeft;
const SkColor DEFAULT_FOREGROUND = SK_ColorBLACK;
const SkScalar DEFAULT_CURSOR_WIDTH = 2;
const SkColor DEFAULT_CURSOR_COLOR = SK_ColorGRAY;

std::unique_ptr<Cursor> Cursor::Make() { return std::unique_ptr<Cursor>(new Cursor()); }

Cursor::Cursor() {
    fLinePaint.setColor(SK_ColorGRAY);
    fLinePaint.setAntiAlias(true);

    fRectPaint.setColor(DEFAULT_CURSOR_COLOR);
    fRectPaint.setStyle(SkPaint::kStroke_Style);
    fRectPaint.setStrokeWidth(2);
    fRectPaint.setAntiAlias(true);

    fXY = SkPoint::Make(0, 0);
    fSize = SkSize::Make(0, 0);
    fBlink = true;
}

void Cursor::paint(SkCanvas* canvas, SkPoint xy) {

    if (fBlink) {
       canvas->drawRect(SkRect::MakeXYWH(fXY.fX + xy.fX, fXY.fY + xy.fY, DEFAULT_CURSOR_WIDTH, fSize.fHeight),
                fRectPaint);
    } else {
        canvas->drawLine(fXY + xy, fXY + xy + SkPoint::Make(1, fSize.fHeight), fLinePaint);
    }
}

std::unique_ptr<Editor> Editor::Make(std::u16string text, SkSize size, SkSpan<Block> fontBlocks) {
    return std::unique_ptr<Editor>(new Editor(text, size, fontBlocks));
}

Editor::Editor(std::u16string text, SkSize size, SkSpan<Block> fontBlocks) {
    fParent = nullptr;
    fText = std::move(text);
    fCursor = Cursor::Make();
    fMouse = std::make_unique<Mouse>();
    fUnicodeText = Text::parse(SkSpan<uint16_t>((uint16_t*)fText.data(), fText.size()));
    fShapedText = fUnicodeText->shape(fontBlocks, TEXT_DIRECTION);
    fWrappedText = fShapedText->wrap(size.width(), size.height(), fUnicodeText->getUnicode());
    fFormattedText = fWrappedText->format(TEXT_ALIGN, TEXT_DIRECTION);

    auto endOfText = fFormattedText->indexToAdjustedGraphemePosition(fText.size());
    auto rect = std::get<3>(endOfText);
    fCursor->place(SkPoint::Make(rect.fLeft, rect.fTop),
                   SkSize::Make(std::max(DEFAULT_CURSOR_WIDTH, rect.width()), rect.height()));
                   //SkSize::Make(DEFAULT_CURSOR_WIDTH, rect.height()));
}

bool Editor::moveCursor(skui::Key key) {
    auto cursorPosition = fCursor->getCenterPosition();
    auto textIndex = fFormattedText->positionToAdjustedGraphemeIndex(cursorPosition);

    if (key == skui::Key::kLeft) {
        if (textIndex == 0) {
            return false;
        } else {
            --textIndex;
        }
    } else if (key == skui::Key::kRight) {
        if (textIndex >= fText.size()) {
            return false;
        } else {
            ++textIndex;
        }
    } else if (key == skui::Key::kHome) {
        textIndex = 0;
    } else if (key == skui::Key::kEnd) {
        textIndex = fText.size();
    } else if (key == skui::Key::kUp) {
        auto currentPosition = fFormattedText->indexToAdjustedGraphemePosition(textIndex);
        auto line = std::get<0>(currentPosition);
        auto lineIndex = fFormattedText->lineIndex(line);
        if (lineIndex == 0) {
            return false;
        }
        cursorPosition.offset(0, - line->getMetrics().height());
        textIndex = fFormattedText->positionToAdjustedGraphemeIndex(cursorPosition);
    } else if (key == skui::Key::kDown) {
        auto currentPosition = fFormattedText->indexToAdjustedGraphemePosition(textIndex);
        auto line = std::get<0>(currentPosition);
        auto lineIndex = fFormattedText->lineIndex(line);
        if (lineIndex == fFormattedText->countLines()) {
            return false;
        }
        cursorPosition.offset(0, line->getMetrics().height());
        textIndex = fFormattedText->positionToAdjustedGraphemeIndex(cursorPosition);
    }
    auto nextPosition = fFormattedText->indexToAdjustedGraphemePosition(textIndex);
    auto rect = std::get<3>(nextPosition);
    fCursor->place(SkPoint::Make(rect.fLeft, rect.fTop), SkSize::Make(rect.width(), rect.height()));

    return true;
}

void Editor::onPaint(SkSurface* surface) {
    SkCanvas* canvas = surface->getCanvas();
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect({0, 0, (float)fWidth, (float)fHeight});
    canvas->drawColor(SK_ColorWHITE);
    this->paint(canvas, SkPoint::Make(0, 0));
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
    if (!Any(modi & (skui::ModifierKey::kControl | skui::ModifierKey::kOption |
                     skui::ModifierKey::kCommand))) {
        /*
        if (((unsigned)c < 0x7F && (unsigned)c >= 0x20) || c == '\n') {
            char ch = (char)c;
            fEditor.insert(fTextPos, &ch, 1);
            #ifdef SK_EDITOR_DEBUG_OUT
            SkDebugf("insert: %X'%c'\n", (unsigned)c, ch);
            #endif  // SK_EDITOR_DEBUG_OUT
            return this->moveCursor(Editor::Movement::kRight);
        }
        */
    }
    static constexpr skui::ModifierKey kCommandOrControl =
            skui::ModifierKey::kCommand | skui::ModifierKey::kControl;
    if (Any(modi & kCommandOrControl) && !Any(modi & ~kCommandOrControl)) {
        return false;
    }
    return false;
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
            /*
            case skui::Key::kDelete:
                if (fMarkPos != Editor::TextPosition()) {
                    (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
                } else {
                    auto pos = fEditor.move(Editor::Movement::kRight, fTextPos);
                    (void)this->move(fEditor.remove(fTextPos, pos), false);
                }
                this->inval();
                return true;
            case skui::Key::kBack:
                if (fMarkPos != Editor::TextPosition()) {
                    (void)this->move(fEditor.remove(fMarkPos, fTextPos), false);
                } else {
                    auto pos = fEditor.move(Editor::Movement::kLeft, fTextPos);
                    (void)this->move(fEditor.remove(fTextPos, pos), false);
                }
                this->inval();
                return true;
            case skui::Key::kOK:
                return this->onChar('\n', modifiers);
            */
            default:
                break;
        }
    }
    return false;
}

void Editor::onBeginLine(TextRange, float baselineY) {}
void Editor::onEndLine(TextRange, float baselineY) {}
void Editor::onPlaceholder(TextRange, const SkRect& bounds) {}
void Editor::onGlyphRun(SkFont font,
                        TextRange textRange,
                        int glyphCount,
                        const uint16_t glyphs[],
                        const SkPoint positions[],
                        const SkPoint offsets[]) {
    SkTextBlobBuilder builder;
    const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
    sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
    sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
    SkPaint foreground;
    foreground.setColor(DEFAULT_FOREGROUND);
    fCanvas->drawTextBlob(builder.make(), fXY.fX, fXY.fY, foreground);
}

void Editor::paint(SkCanvas* canvas, SkPoint xy) {
    fCanvas = canvas;
    fXY = xy;
    this->fFormattedText->visit(this);

    fCursor->paint(canvas, xy);
}

std::unique_ptr<Editor> Editor::MakeDemo(SkScalar width) {

    std::u16string text0 = u"In a hole in the ground there lived a hobbit. Not a nasty, dirty, "
                            "wet hole full of worms and oozy smells. This was a hobbit-hole and "
                            "that means good food, a warm hearth, and all the comforts of home.";

    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40);
    Block block(text0.size(), fontChain);
    return Editor::Make(text0, SkSize::Make(width, SK_ScalarInfinity), SkSpan<Block>(&block, 1));
}
} // namespace text
} // namespace skia
