// Copyright 2021 Google LLC.
#ifndef Editor_DEFINED
#define Editor_DEFINED
#include <sstream>
#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/include/Types.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

namespace skia {
namespace text {

class Cursor {
public:
    static std::unique_ptr<Cursor> Make();
    Cursor();
    virtual ~Cursor() = default;
    void place(SkPoint xy, SkSize size) {
        fXY = xy;
        fSize = size;
    }

    void blink() {
        fBlink = !fBlink;
    }

    SkPoint getPosition() const { return fXY; }
    SkPoint getCenterPosition() const {
        return fXY + SkPoint::Make(0, fSize.fHeight / 2);
    }

    void paint(SkCanvas* canvas, SkPoint xy);

private:
    SkPaint fLinePaint;
    SkPaint fRectPaint;
    SkPoint fXY;
    SkSize fSize;
    bool fBlink;
};

class Mouse {
    const SkMSec MAX_DBL_TAP_INTERVAL = 300;
    const float MAX_DBL_TAP_DISTANCE = 100;
public:
    Mouse() : fMouseDown(false), fLastTouchPoint(), fLastTouchTime() { }

    void down();
    void up();
    void clearTouchInfo() {
        fLastTouchPoint = SkPoint::Make(0, 0);
        fLastTouchTime = 0.0;
    }
    bool isDown() { return fMouseDown; }
    bool isDoubleClick(SkPoint touch);

private:
    bool fMouseDown;
    SkPoint fLastTouchPoint;
    double fLastTouchTime;
};

class Selection {
public:
    Selection(SkColor color) : fGlyphBoxes(), fTextRanges() {
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
    std::vector<SkRect> fGlyphBoxes;
    std::vector<TextRange> fTextRanges;
};

struct Style {
    sk_sp<SkTypeface> fTypeface;
    SkScalar fFontSize;
    SkFontStyle fFontStyle;
    SkColor fForeground;
    size_t fLength;

    std::string toString() {
        SkString ff;
        fTypeface->getFamilyName(&ff);
        std::stringstream ss;
        ss << ff.c_str() << ", "
           << fFontSize
           << (fFontStyle.slant() == SkFontStyle::Slant::kItalic_Slant ? ", italic " : "");
        if (fFontStyle.weight() != SkFontStyle::Weight::kNormal_Weight) {
            ss << ", " << fFontStyle.weight();
        }
        return ss.str();
    }
};

class Editor : public sk_app::Window::Layer, FormattedText::Visitor {
public:
    static std::unique_ptr<Editor> Make(std::u16string text, SkSize size, SkSpan<Block> fontBlocks);
    static std::unique_ptr<Editor> MakeDemo(SkScalar width);
    void paint(SkCanvas* canvas, SkPoint point);
    void blink() { fCursor->blink(); }
    void onResize(int width, int height) override;
private:

    Editor(std::u16string text, SkSize size, SkSpan<Block> fontBlocks);

    void onAttach(sk_app::Window* w) override { fParent = w; }
    void onPaint(SkSurface* surface) override;

    bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) override;
    bool onKey(skui::Key, skui::InputState, skui::ModifierKey) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifier) override;
    void invalidate() { if (fParent) { fParent->inval(); } }

    void onBeginLine(TextRange, float baselineY) override;
    void onEndLine(TextRange, float baselineY) override;
    void onGlyphRun(SkFont font,
                    TextRange textRange,
                    int glyphCount,
                    const uint16_t glyphs[],
                    const SkPoint  positions[],
                    const SkPoint offsets[]) override;
    void onPlaceholder(TextRange, const SkRect& bounds) override;

    bool moveCursor(skui::Key key);

    std::unique_ptr<Cursor> fCursor;
    std::unique_ptr<Mouse> fMouse;
    std::unique_ptr<Selection> fSelection;
    std::u16string fText;
    std::unique_ptr<UnicodeText> fUnicodeText;
    std::unique_ptr<ShapedText> fShapedText;
    std::unique_ptr<WrappedText> fWrappedText;
    sk_sp<FormattedText> fFormattedText;
    SkCanvas* fCanvas;
    SkPoint fXY;
    sk_app::Window* fParent;
    int fWidth;
    int fHeight;
    //std::vector<Style> fStyles;
};
} // namespace text
} // namespace skia
#endif // Editor_DEFINED
