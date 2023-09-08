// Copyright 2021 Google LLC.

#include "experimental/sktext/editor/Editor.h"
#include "experimental/sktext/src/Paint.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "src/base/SkTime.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/viewer/Slide.h"

using namespace skia::text;
using namespace skia::editor;

namespace {
class TextSlide_HelloWorld : public Slide {
public:
    TextSlide_HelloWorld() { fName = "TextSlide_HelloWorld"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        skia::text::Paint::drawText(u"Hello word", canvas, 0, 0);
    }
};

class TextSlide_Align_Dir : public Slide {
public:
    TextSlide_Align_Dir() : fUnicode(SkUnicode::Make()) { fName = "TextSlide_Align_Dir"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = fSize.width() / 4;
        SkScalar height = fSize.height() / 2;

        const std::u16string line = u"One line of text";

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kLtr);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kLtr);
        canvas->translate(-width * 3, height);

        drawLine(canvas, width, height, line, TextAlign::kLeft, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, TextDirection::kRtl);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, TextDirection::kRtl);
        canvas->translate(width, 0);

    }

    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

private:
    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h,
                  const std::u16string& text,
                  TextAlign align,
                  TextDirection direction = TextDirection::kLtr) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkPaint foregroundPaint(SkColors::kBlack);
        SkPaint backgroundPaint(SkColors::kLtGray);
        Paint::drawText(direction == TextDirection::kRtl ? mirror(text) : normal(text),
                        canvas,
                        direction, align,
                        foregroundPaint, backgroundPaint,
                        SkString("Roboto"), 12.0f, SkFontStyle::Normal(),
                        0, 0);
    }

    std::u16string mirror(const std::u16string& text) {
        std::u16string result;
        result += u"\u202E";
        for (auto i = text.size(); i > 0; --i) {
            result += text[i - 1];
        }
        //for (auto ch : text) {
        //    result += ch;
        //}
        result += u"\u202C";
        return result;
    }

    std::u16string normal(const std::u16string& text) {
        std::u16string result;
        //result += u"\u202D";
        for (auto ch : text) {
            result += ch;
        }
        //result += u"\u202C";
        return result;
    }

    std::unique_ptr<SkUnicode> fUnicode;
    SkSize fSize;
};

class TextSlide_LongLTR : public Slide {
public:
    TextSlide_LongLTR() { fName = "TextSlide_LongLTR"; }

    void load(SkScalar w, SkScalar h) override { fWidth = w; }

    void resize(SkScalar w, SkScalar h) override { fWidth = w; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(u"A very_very_very_very_very_very_very_very_very_very "
                "very_very_very_very_very_very_very_very_very_very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very long text",
                canvas, fWidth);

    }

private:
    std::unique_ptr<SkUnicode> fUnicode;
    SkScalar fWidth;
};

class TextSlide_LongRTL1 : public Slide {
public:
    TextSlide_LongRTL1() { fName = "TextSlide_LongRTL1"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(mirror("LONG MIRRORED TEXT SHOULD SHOW RIGHT TO LEFT (AS NORMAL)"),
                        canvas, 0, 0);
    }

private:
    std::u16string mirror(const std::string& text) {
        std::u16string result;
        result += u"\u202E";
        for (auto i = text.size(); i > 0; --i) {
            result += text[i - 1];
        }
        for (auto ch : text) {
            result += ch;
        }
        result += u"\u202C";
        return result;
    }

    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSlide_LongRTL2 : public Slide {
public:
    TextSlide_LongRTL2() { fName = "TextSlide_LongRTL2"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        std::u16string utf16(u"يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُيَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُيَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ");
        Paint::drawText(utf16, canvas,
                        TextDirection::kRtl, TextAlign::kRight,
                        SkPaint(SkColors::kBlack), SkPaint(SkColors::kLtGray),
                        SkString("Noto Naskh Arabic"), 40.0f, SkFontStyle::Normal(),
                        SkSize::Make(800, 800), 0, 0);
    }

private:
    std::unique_ptr<SkUnicode> fUnicode;
};

}  // namespace

DEF_SLIDE(return new TextSlide_HelloWorld();)
DEF_SLIDE(return new TextSlide_Align_Dir();)
DEF_SLIDE(return new TextSlide_LongLTR();)
DEF_SLIDE(return new TextSlide_LongRTL1();)
DEF_SLIDE(return new TextSlide_LongRTL2();)
