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
#include "include/core/SkTime.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

using namespace skia::text;
using namespace skia::editor;

namespace {
class TextSample_HelloWorld : public Sample {
protected:
    SkString name() override { return SkString("TextSample_HelloWorld"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        skia::text::Paint::drawText(u"Hello word", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
};

class TextSample_Align_Dir : public Sample {

public:
    TextSample_Align_Dir() : fUnicode(SkUnicode::Make()) { }
protected:
    SkString name() override { return SkString("TextSample_Align_Dir"); }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h,
                  const std::u16string& text,
                  TextAlign align,
                  TextDirection direction = TextDirection::kLtr) {
        const std::u16string& ellipsis = u"\u2026";
        SkScalar margin = 20;

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

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = this->width() / 4;
        SkScalar height = this->height() / 2;

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

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongLTR : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongLTR"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(u"A very_very_very_very_very_very_very_very_very_very "
                "very_very_very_very_very_very_very_very_very_very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very long text", canvas, this->width());

    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongRTL1 : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongRTL"); }

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

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Paint::drawText(mirror("LONG MIRRORED TEXT SHOULD SHOW RIGHT TO LEFT (AS NORMAL)"), canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

class TextSample_LongRTL2 : public Sample {
protected:
    SkString name() override { return SkString("TextSample_LongRTL"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        std::u16string utf16(u"يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُيَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُيَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ");
        Paint::drawText(utf16, canvas,
                        TextDirection::kRtl, TextAlign::kRight,
                        SkPaint(SkColors::kBlack), SkPaint(SkColors::kLtGray),
                        SkString("Noto Naskh Arabic"), 40.0f, SkFontStyle::Normal(),
                        SkSize::Make(800, 800), 0, 0);
    }

private:
    using INHERITED = Sample;
    std::unique_ptr<SkUnicode> fUnicode;
};

}  // namespace

DEF_SAMPLE(return new TextSample_HelloWorld();)
DEF_SAMPLE(return new TextSample_Align_Dir();)
DEF_SAMPLE(return new TextSample_LongLTR();)
DEF_SAMPLE(return new TextSample_LongRTL1();)
DEF_SAMPLE(return new TextSample_LongRTL2();)
