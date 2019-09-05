// Copyright 2019 Google LLC.
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"

using namespace skia::textlayout;
namespace {

class ParagraphView_Base : public Sample {
protected:
    sk_sp<TestFontCollection> getFontCollection() {
        // If we reset font collection we need to reset paragraph cache
        static sk_sp<TestFontCollection> fFC = nullptr;
        if (fFC == nullptr) {
            fFC = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str());
        }
        return fFC;
    }
};

sk_sp<SkShader> setgrad(const SkRect& r, SkColor c0, SkColor c1) {
    SkColor colors[] = {c0, c1};
    SkPoint pts[] = {{r.fLeft, r.fTop}, {r.fRight, r.fTop}};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

}  // namespace

class ParagraphView1 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph1"); }

    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        const std::vector<
            std::tuple<std::string, bool, bool, int, SkColor, SkColor, bool, TextDecorationStyle>>
            gParagraph = {{"monospace", true, false, 14, SK_ColorWHITE, SK_ColorRED, true,
                           TextDecorationStyle::kDashed},
                          {"Assyrian", false, false, 20, SK_ColorWHITE, SK_ColorBLUE, false,
                           TextDecorationStyle::kDotted},
                          {"serif", true, true, 10, SK_ColorWHITE, SK_ColorRED, true,
                           TextDecorationStyle::kDouble},
                          {"Arial", false, true, 16, SK_ColorGRAY, SK_ColorGREEN, true,
                           TextDecorationStyle::kSolid},
                          {"sans-serif", false, false, 8, SK_ColorWHITE, SK_ColorRED, false,
                           TextDecorationStyle::kWavy}};
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(fg);

        SkPaint blue;
        blue.setColor(SK_ColorBLUE);

        TextStyle defaultStyle;
        defaultStyle.setBackgroundColor(blue);
        defaultStyle.setForegroundColor(paint);
        ParagraphStyle paraStyle;

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        for (auto i = 1; i < 5; ++i) {
            defaultStyle.setFontSize(24 * i);
            paraStyle.setTextStyle(defaultStyle);
            ParagraphBuilderImpl builder(paraStyle, fontCollection);
            std::string name = "Paragraph: " + std::to_string(24 * i);
            builder.addText(name.c_str(), name.length());
            for (auto para : gParagraph) {
                TextStyle style;
                style.setFontFamilies({SkString(std::get<0>(para).c_str())});
                SkFontStyle fontStyle(std::get<1>(para) ? SkFontStyle::Weight::kBold_Weight
                                                        : SkFontStyle::Weight::kNormal_Weight,
                                      SkFontStyle::Width::kNormal_Width,
                                      std::get<2>(para) ? SkFontStyle::Slant::kItalic_Slant
                                                        : SkFontStyle::Slant::kUpright_Slant);
                style.setFontStyle(fontStyle);
                style.setFontSize(std::get<3>(para) * i);
                SkPaint background;
                background.setColor(std::get<4>(para));
                style.setBackgroundColor(background);
                SkPaint foreground;
                foreground.setColor(std::get<5>(para));
                foreground.setAntiAlias(true);
                style.setForegroundColor(foreground);
                if (std::get<6>(para)) {
                    style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(5, 5), 2));
                }

                auto decoration = (i % 4);
                if (decoration == 3) {
                    decoration = 4;
                }

                bool test = (TextDecoration)decoration != TextDecoration::kNoDecoration;
                std::string deco = std::to_string((int)decoration);
                if (test) {
                    style.setDecoration((TextDecoration)decoration);
                    style.setDecorationStyle(std::get<7>(para));
                    style.setDecorationColor(std::get<5>(para));
                }
                builder.pushStyle(style);
                std::string name = " " + std::get<0>(para) + " " +
                                   (std::get<1>(para) ? ", bold" : "") +
                                   (std::get<2>(para) ? ", italic" : "") + " " +
                                   std::to_string(std::get<3>(para) * i) +
                                   (std::get<4>(para) != bg ? ", background" : "") +
                                   (std::get<5>(para) != fg ? ", foreground" : "") +
                                   (std::get<6>(para) ? ", shadow" : "") +
                                   (test ? ", decorations " + deco : "") + ";";
                builder.addText(name.c_str(), name.length());
                builder.pop();
            }

            auto paragraph = builder.Build();
            paragraph->layout(w - margin * 2);
            paragraph->paint(canvas, margin, margin);

            canvas->translate(0, paragraph->getHeight());
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        drawTest(canvas, this->width(), this->height(), SK_ColorRED, SK_ColorWHITE);
    }

private:

    typedef Sample INHERITED;
};

class ParagraphView2 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph2"); }

    void drawCode(SkCanvas* canvas, SkScalar w, SkScalar h) {
        SkPaint comment;
        comment.setColor(SK_ColorGRAY);
        SkPaint constant;
        constant.setColor(SK_ColorMAGENTA);
        SkPaint null;
        null.setColor(SK_ColorMAGENTA);
        SkPaint literal;
        literal.setColor(SK_ColorGREEN);
        SkPaint code;
        code.setColor(SK_ColorDKGRAY);
        SkPaint number;
        number.setColor(SK_ColorBLUE);
        SkPaint name;
        name.setColor(SK_ColorRED);

        SkPaint white;
        white.setColor(SK_ColorWHITE);

        TextStyle defaultStyle;
        defaultStyle.setBackgroundColor(white);
        defaultStyle.setForegroundColor(code);
        defaultStyle.setFontFamilies({SkString("monospace")});
        defaultStyle.setFontSize(30);
        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(defaultStyle);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        ParagraphBuilderImpl builder(paraStyle, fontCollection);

        const char* text1 = "RaisedButton";
        const char* text2 = "(\n";
        const char* text3 = "  child: ";
        const char* text4 = "const";
        const char* text5 = "Text";
        const char* text6 = "'BUTTON TITLE'";
        const char* text7 = "),\n";

        builder.pushStyle(style(name));
        builder.addText(text1, strlen(text1));
        builder.pop();
        builder.addText(text2, strlen(text2));
        builder.addText(text3, strlen(text3));
        builder.pushStyle(style(constant));
        builder.addText(text4, strlen(text4));
        builder.pop();
        builder.addText(" ", 1);
        builder.pushStyle(style(name));
        builder.addText(text5, strlen(text5));
        builder.pop();
        builder.addText("(", 1);
        builder.pushStyle(style(literal));
        builder.addText(text6, strlen(text6));
        builder.pop();
        builder.addText(text7, strlen(text7));

        auto paragraph = builder.Build();
        paragraph->layout(w - 20);

        paragraph->paint(canvas, 20, 20);
    }

    TextStyle style(SkPaint paint) {
        TextStyle style;
        paint.setAntiAlias(true);
        style.setForegroundColor(paint);
        style.setFontFamilies({SkString("monospace")});
        style.setFontSize(30);

        return style;
    }

    void drawText(SkCanvas* canvas, SkScalar w, SkScalar h, std::vector<const char*>& text,
                  SkColor fg = SK_ColorDKGRAY, SkColor bg = SK_ColorWHITE,
                  const char* ff = "sans-serif", SkScalar fs = 24,
                  size_t lineLimit = 30,
                  const std::u16string& ellipsis = u"\u2026") {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(bg);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(fg);

        SkPaint blue;
        blue.setColor(SK_ColorBLUE);

        SkPaint background;
        background.setColor(bg);

        TextStyle style;
        style.setBackgroundColor(blue);
        style.setForegroundColor(paint);
        style.setFontFamilies({SkString(ff)});
        style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight,
                                       SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));
        style.setFontSize(fs);
        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);
        TextStyle defaultStyle;
        defaultStyle.setFontSize(20);
        paraStyle.setTextStyle(defaultStyle);
        ParagraphBuilderImpl builder(paraStyle, getFontCollection());

        SkPaint foreground;
        foreground.setColor(fg);
        style.setForegroundColor(foreground);
        style.setBackgroundColor(background);

        for (auto& part : text) {
            builder.pushStyle(style);
            builder.addText(part, strlen(part));
            builder.pop();
        }

        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->paint(canvas, margin, margin);

        canvas->translate(0, paragraph->getHeight() + margin);
    }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h, const std::string& text,
                  TextAlign align) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);

        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);

        TextStyle style;
        style.setBackgroundColor(gray);
        style.setForegroundColor(paint);
        style.setFontFamilies({SkString("Arial")});
        style.setFontSize(30);
        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setTextAlign(align);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        ParagraphBuilderImpl builder(paraStyle, fontCollection);
        builder.addText(text.c_str(), text.length());

        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->layout(w - margin);
        paragraph->paint(canvas, margin, margin);

        canvas->translate(0, paragraph->getHeight() + margin);
    }

    void onDrawContent(SkCanvas* canvas) override {
        std::vector<const char*> cupertino = {
                "google_logogoogle_gsuper_g_logo 1 "
                "google_logogoogle_gsuper_g_logo 12 "
                "google_logogoogle_gsuper_g_logo 123 "
                "google_logogoogle_gsuper_g_logo 1234 "
                "google_logogoogle_gsuper_g_logo 12345 "
                "google_logogoogle_gsuper_g_logo 123456 "
                "google_logogoogle_gsuper_g_logo 1234567 "
                "google_logogoogle_gsuper_g_logo 12345678 "
                "google_logogoogle_gsuper_g_logo 123456789 "
                "google_logogoogle_gsuper_g_logo 1234567890 "
                "google_logogoogle_gsuper_g_logo 123456789 "
                "google_logogoogle_gsuper_g_logo 12345678 "
                "google_logogoogle_gsuper_g_logo 1234567 "
                "google_logogoogle_gsuper_g_logo 123456 "
                "google_logogoogle_gsuper_g_logo 12345 "
                "google_logogoogle_gsuper_g_logo 1234 "
                "google_logogoogle_gsuper_g_logo 123 "
                "google_logogoogle_gsuper_g_logo 12 "
                "google_logogoogle_gsuper_g_logo 1 "
                "google_logogoogle_gsuper_g_logo "
                "google_logogoogle_gsuper_g_logo "
                "google_logogoogle_gsuper_g_logo "
                "google_logogoogle_gsuper_g_logo "
                "google_logogoogle_gsuper_g_logo "
                "google_logogoogle_gsuper_g_logo"};
        std::vector<const char*> text = {
                "My neighbor came over to say,\n"
                "Although not in a neighborly way,\n\n"
                "That he'd knock me around,\n\n\n"
                "If I didn't stop the sound,\n\n\n\n"
                "Of the classical music I play."};

        std::vector<const char*> long_word = {
                "A_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_long_text"};

        std::vector<const char*> very_long = {
                "A very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very long text"};

        std::vector<const char*> very_word = {
                "A very_very_very_very_very_very_very_very_very_very "
                "very_very_very_very_very_very_very_very_very_very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very long text"};

        SkScalar width = this->width() / 5;
        SkScalar height = this->height();
        drawText(canvas, width, height, long_word, SK_ColorBLACK, SK_ColorWHITE, "Google Sans", 30);
        canvas->translate(width, 0);
        drawText(canvas, width, height, very_long, SK_ColorBLACK, SK_ColorWHITE, "Google Sans", 30);
        canvas->translate(width, 0);
        drawText(canvas, width, height, very_word, SK_ColorBLACK, SK_ColorWHITE, "Google Sans", 30);
        canvas->translate(width, 0);
        drawText(canvas, width, height / 2, text, SK_ColorBLACK, SK_ColorWHITE, "Roboto", 20, 100,
                 u"\u2026");
        canvas->translate(0, height / 2);
        drawCode(canvas, width, height / 2);
        canvas->translate(width, -height / 2);

        drawText(canvas, width, height, cupertino, SK_ColorBLACK, SK_ColorWHITE, "Google Sans", 30);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView3 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph3"); }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h, const std::string& text,
                  TextAlign align, size_t lineLimit = std::numeric_limits<size_t>::max(),
                  bool RTL = false, SkColor background = SK_ColorGRAY,
                  const std::u16string& ellipsis = u"\u2026") {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        SkPaint gray;
        gray.setColor(background);

        SkPaint yellow;
        yellow.setColor(SK_ColorYELLOW);

        TextStyle style;
        style.setBackgroundColor(gray);
        style.setForegroundColor(paint);
        style.setFontFamilies({SkString("sans-serif")});
        style.setFontSize(30);
        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setTextAlign(align);
        paraStyle.setMaxLines(lineLimit);
        paraStyle.setEllipsis(ellipsis);
        // paraStyle.setTextDirection(RTL ? SkTextDirection::rtl : SkTextDirection::ltr);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        ParagraphBuilderImpl builder(paraStyle, fontCollection);
        if (RTL) {
            builder.addText(mirror(text));
        } else {
            builder.addText(normal(text));
        }

        canvas->drawRect(SkRect::MakeXYWH(margin, margin, w - margin * 2, h - margin * 2), yellow);
        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->paint(canvas, margin, margin);
    }

    std::u16string mirror(const std::string& text) {
        std::u16string result;
        result += u"\u202E";
        // for (auto i = text.size(); i > 0; --i) {
        //  result += text[i - 1];
        //}

        for (auto i = text.size(); i > 0; --i) {
            auto ch = text[i - 1];
            if (ch == ',') {
                result += u"!";
            } else if (ch == '.') {
                result += u"!";
            } else {
                result += ch;
            }
        }

        result += u"\u202C";
        return result;
    }

    std::u16string normal(const std::string& text) {
        std::u16string result;
        result += u"\u202D";
        for (auto ch : text) {
            result += ch;
        }
        result += u"\u202C";
        return result;
    }

    void onDrawContent(SkCanvas* canvas) override {
        const std::string options =  // { "open-source open-source open-source open-source" };
                {"Flutter is an open-source project to help developers "
                 "build high-performance, high-fidelity, mobile apps for "
                 "iOS and Android "
                 "from a single codebase. This design lab is a playground "
                 "and showcase of Flutter's many widgets, behaviors, "
                 "animations, layouts, and more."};

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = this->width() / 4;
        SkScalar height = this->height() / 2;

        const std::string line =
                "World domination is such an ugly phrase - I prefer to call it world optimisation";

        drawLine(canvas, width, height, line, TextAlign::kLeft, 1, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, 2, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, 3, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, 4, false, SK_ColorLTGRAY);
        canvas->translate(-width * 3, height);

        drawLine(canvas, width, height, line, TextAlign::kLeft, 1, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kRight, 2, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kCenter, 3, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, TextAlign::kJustify, 4, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView4 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph4"); }

    void drawFlutter(SkCanvas* canvas, SkScalar w, SkScalar h,
                     const char* ff = "Google Sans", SkScalar fs = 30,
                     size_t lineLimit = std::numeric_limits<size_t>::max(),
                     const std::u16string& ellipsis = u"\u2026") {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));

        SkScalar margin = 20;

        SkPaint black;
        black.setAntiAlias(true);
        black.setColor(SK_ColorBLACK);

        SkPaint blue;
        blue.setAntiAlias(true);
        blue.setColor(SK_ColorBLUE);

        SkPaint red;
        red.setAntiAlias(true);
        red.setColor(SK_ColorRED);

        SkPaint green;
        green.setAntiAlias(true);
        green.setColor(SK_ColorGREEN);

        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);

        SkPaint yellow;
        yellow.setColor(SK_ColorYELLOW);

        SkPaint magenta;
        magenta.setAntiAlias(true);
        magenta.setColor(SK_ColorMAGENTA);

        TextStyle style;
        style.setFontFamilies({SkString(ff)});
        style.setFontSize(fs);

        TextStyle style0;
        style0.setForegroundColor(black);
        style0.setBackgroundColor(gray);
        style0.setFontFamilies({SkString(ff)});
        style0.setFontSize(fs);
        style0.setDecoration(TextDecoration::kUnderline);
        style0.setDecorationStyle(TextDecorationStyle::kDouble);
        style0.setDecorationColor(SK_ColorBLACK);

        TextStyle style1;
        style1.setForegroundColor(blue);
        style1.setBackgroundColor(yellow);
        style1.setFontFamilies({SkString(ff)});
        style1.setFontSize(fs);
        style1.setDecoration(TextDecoration::kOverline);
        style1.setDecorationStyle(TextDecorationStyle::kWavy);
        style1.setDecorationColor(SK_ColorBLACK);

        TextStyle style2;
        style2.setForegroundColor(red);
        style2.setFontFamilies({SkString(ff)});
        style2.setFontSize(fs);

        TextStyle style3;
        style3.setForegroundColor(green);
        style3.setFontFamilies({SkString(ff)});
        style3.setFontSize(fs);

        TextStyle style4;
        style4.setForegroundColor(magenta);
        style4.setFontFamilies({SkString(ff)});
        style4.setFontSize(fs);

        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);

        const char* logo1 = "google_";
        const char* logo2 = "logo";
        const char* logo3 = "go";
        const char* logo4 = "ogle_logo";
        const char* logo5 = "google_lo";
        const char* logo6 = "go";
        {
            ParagraphBuilderImpl builder(paraStyle, getFontCollection());

            builder.pushStyle(style0);
            builder.addText(logo1, strlen(logo1));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo2, strlen(logo2));
            builder.pop();

            builder.addText(" ", 1);

            builder.pushStyle(style0);
            builder.addText(logo3, strlen(logo3));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo4, strlen(logo4));
            builder.pop();

            builder.addText(" ", 1);

            builder.pushStyle(style0);
            builder.addText(logo5, strlen(logo5));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo6, strlen(logo6));
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(w - margin * 2);
            paragraph->paint(canvas, margin, margin);
            canvas->translate(0, h + margin);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->width();
        SkScalar height = this->height();

        drawFlutter(canvas, width, height / 2);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView5 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph5"); }

    void bidi(SkCanvas* canvas, SkScalar w, SkScalar h, const std::u16string& text,
              const std::u16string& expected, size_t lineLimit = std::numeric_limits<size_t>::max(),
              const char* ff = "Roboto", SkScalar fs = 30,
              const std::u16string& ellipsis = u"\u2026") {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));

        SkScalar margin = 20;

        SkPaint black;
        black.setColor(SK_ColorBLACK);
        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);

        TextStyle style;
        style.setForegroundColor(black);
        style.setFontFamilies({SkString(ff)});
        style.setFontSize(fs);

        TextStyle style0;
        style0.setForegroundColor(black);
        style0.setFontFamilies({SkString(ff)});
        style0.setFontSize(fs);
        style0.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));

        TextStyle style1;
        style1.setForegroundColor(gray);
        style1.setFontFamilies({SkString(ff)});
        style1.setFontSize(fs);
        style1.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kUpright_Slant));

        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);

        ParagraphBuilderImpl builder(paraStyle, getFontCollection());

        if (text.empty()) {
            const std::u16string text0 = u"\u202Dabc";
            const std::u16string text1 = u"\u202EFED";
            const std::u16string text2 = u"\u202Dghi";
            const std::u16string text3 = u"\u202ELKJ";
            const std::u16string text4 = u"\u202Dmno";
            builder.pushStyle(style0);
            builder.addText(text0);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(text1);
            builder.pop();
            builder.pushStyle(style0);
            builder.addText(text2);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(text3);
            builder.pop();
            builder.pushStyle(style0);
            builder.addText(text4);
            builder.pop();
        } else {
            // icu::UnicodeString unicode((UChar*) text.data(), SkToS32(text.size()));
            // std::string str;
            // unicode.toUTF8String(str);
            // SkDebugf("Text: %s\n", str.c_str());
            builder.addText(text + expected);
        }

        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->paint(canvas, margin, margin);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->width();
        SkScalar height = this->height() / 8;

        const std::u16string text1 =
                u"A \u202ENAC\u202Cner, exceedingly \u202ENAC\u202Cny,\n"
                "One morning remarked to his granny:\n"
                "A \u202ENAC\u202Cner \u202ENAC\u202C \u202ENAC\u202C,\n"
                "Anything that he \u202ENAC\u202C,\n"
                "But a \u202ENAC\u202Cner \u202ENAC\u202C't \u202ENAC\u202C a \u202ENAC\u202C, "
                "\u202ENAC\u202C he?";
        bidi(canvas, width, height * 3, text1, u"", 5);
        canvas->translate(0, height * 3);

        bidi(canvas, width, height, u"\u2067DETALOSI\u2069", u"");
        canvas->translate(0, height);

        bidi(canvas, width, height, u"\u202BDEDDEBME\u202C", u"");
        canvas->translate(0, height);

        bidi(canvas, width, height, u"\u202EEDIRREVO\u202C", u"");
        canvas->translate(0, height);

        bidi(canvas, width, height, u"\u200FTICILPMI\u200E", u"");
        canvas->translate(0, height);

        bidi(canvas, width, height, u"123 456 7890 \u202EZYXWV UTS RQP ONM LKJ IHG FED CBA\u202C.",
             u"", 2);
        canvas->translate(0, height);

        // bidi(canvas, width, height, u"", u"");
        // canvas->translate(0, height);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView6 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph6"); }

    void hangingS(SkCanvas* canvas, SkScalar w, SkScalar h, SkScalar fs = 60.0) {
        auto ff = "HangingS";

        canvas->drawColor(SK_ColorLTGRAY);

        SkPaint black;
        black.setAntiAlias(true);
        black.setColor(SK_ColorBLACK);

        SkPaint blue;
        blue.setAntiAlias(true);
        blue.setColor(SK_ColorBLUE);

        SkPaint red;
        red.setAntiAlias(true);
        red.setColor(SK_ColorRED);

        SkPaint green;
        green.setAntiAlias(true);
        green.setColor(SK_ColorGREEN);

        SkPaint gray;
        gray.setColor(SK_ColorCYAN);

        SkPaint yellow;
        yellow.setColor(SK_ColorYELLOW);

        SkPaint magenta;
        magenta.setAntiAlias(true);
        magenta.setColor(SK_ColorMAGENTA);

        SkFontStyle fontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                              SkFontStyle::kItalic_Slant);

        TextStyle style;
        style.setFontFamilies({SkString(ff)});
        style.setFontSize(fs);
        style.setFontStyle(fontStyle);

        TextStyle style0;
        style0.setForegroundColor(black);
        style0.setBackgroundColor(gray);
        style0.setFontFamilies({SkString(ff)});
        style0.setFontSize(fs);
        style0.setFontStyle(fontStyle);

        TextStyle style1;
        style1.setForegroundColor(blue);
        style1.setBackgroundColor(yellow);
        style1.setFontFamilies({SkString(ff)});
        style1.setFontSize(fs);
        style1.setFontStyle(fontStyle);

        TextStyle style2;
        style2.setForegroundColor(red);
        style2.setFontFamilies({SkString(ff)});
        style2.setFontSize(fs);
        style2.setFontStyle(fontStyle);

        TextStyle style3;
        style3.setForegroundColor(green);
        style3.setFontFamilies({SkString(ff)});
        style3.setFontSize(fs);
        style3.setFontStyle(fontStyle);

        TextStyle style4;
        style4.setForegroundColor(magenta);
        style4.setFontFamilies({SkString(ff)});
        style4.setFontSize(fs);
        style4.setFontStyle(fontStyle);

        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);

        const char* logo1 = "S";
        const char* logo2 = "kia";
        const char* logo3 = "Sk";
        const char* logo4 = "ia";
        const char* logo5 = "Ski";
        const char* logo6 = "a";
        {
            ParagraphBuilderImpl builder(paraStyle, getFontCollection());

            builder.pushStyle(style0);
            builder.addText(logo1, strlen(logo1));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo2, strlen(logo2));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo3, strlen(logo3));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo4, strlen(logo4));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo5, strlen(logo5));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo6, strlen(logo6));
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(w);
            paragraph->paint(canvas, 40, 40);
            canvas->translate(0, h);
        }

        const char* logo11 = "S";
        const char* logo12 = "S";
        const char* logo13 = "S";
        const char* logo14 = "S";
        const char* logo15 = "S";
        const char* logo16 = "S";
        {
            ParagraphBuilderImpl builder(paraStyle, getFontCollection());

            builder.pushStyle(style0);
            builder.addText(logo11, strlen(logo1));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo12, strlen(logo2));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo13, strlen(logo3));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo14, strlen(logo4));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo15, strlen(logo5));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo16, strlen(logo6));
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(w);
            paragraph->paint(canvas, 40, h);
            canvas->translate(0, h);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->width();
        SkScalar height = this->height() / 4;

        hangingS(canvas, width, height);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView7 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph7"); }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar letterSpace, SkScalar w,
                  SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* line =
                "World domination is such an ugly phrase - I prefer to call it world optimisation.";

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(TextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto")});
        textStyle.setFontSize(30);
        textStyle.setLetterSpacing(letterSpace);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        ParagraphBuilderImpl builder(paragraphStyle, getFontCollection());
        builder.pushStyle(textStyle);
        builder.addText(line, strlen(line));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(w - 20);
        paragraph->paint(canvas, 10, 10);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->height() / 4;
        auto w = this->width() / 2;

        drawText(canvas, SK_ColorGRAY, 1, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorLTGRAY, 2, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorCYAN, 3, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorGRAY, 4, w, h);
        canvas->translate(w, -3 * h);

        drawText(canvas, SK_ColorYELLOW, 5, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorGREEN, 10, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorRED, 15, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorBLUE, 20, w, h);
        canvas->translate(0, h);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView8 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph8"); }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar wordSpace, SkScalar w,
                  SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* line =
                "World domination is such an ugly phrase - I prefer to call it world optimisation.";

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(TextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto")});
        textStyle.setFontSize(30);
        textStyle.setWordSpacing(wordSpace);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        ParagraphBuilderImpl builder(paragraphStyle, getFontCollection());
        builder.pushStyle(textStyle);
        builder.addText(line, strlen(line));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(w - 20);
        paragraph->paint(canvas, 10, 10);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->height() / 4;
        auto w = this->width() / 2;

        drawText(canvas, SK_ColorGRAY, 1, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorLTGRAY, 2, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorCYAN, 3, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorGRAY, 4, w, h);
        canvas->translate(w, -3 * h);

        drawText(canvas, SK_ColorYELLOW, 5, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorGREEN, 10, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorRED, 15, w, h);
        canvas->translate(0, h);

        drawText(canvas, SK_ColorBLUE, 20, w, h);
        canvas->translate(0, h);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView9 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph9"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case 'w':
                    ++wordSpacing;
                    return true;
                case 'q':
                    if (wordSpacing > 0) --wordSpacing;
                    return true;
                case 'l':
                    ++letterSpacing;
                    return true;
                case 'k':
                    if (letterSpacing > 0) --letterSpacing;
                    return true;
                default:
                    break;
            }
            return false;
    }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar w, SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* text =
                "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(TextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto")});
        textStyle.setFontSize(50);
        textStyle.setHeight(1.3f);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        ParagraphBuilderImpl builder(paragraphStyle, getFontCollection());
        builder.pushStyle(textStyle);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(550);

        std::vector<size_t> sizes = {0, 1, 2, 8, 19, 21, 22, 30, 150};

        std::vector<size_t> colors = {SK_ColorBLUE, SK_ColorCYAN,  SK_ColorLTGRAY, SK_ColorGREEN,
                                      SK_ColorRED,  SK_ColorWHITE, SK_ColorYELLOW, SK_ColorMAGENTA};

        RectHeightStyle rect_height_style = RectHeightStyle::kTight;
        RectWidthStyle rect_width_style = RectWidthStyle::kTight;

        for (size_t i = 0; i < sizes.size() - 1; ++i) {
            size_t from = (i == 0 ? 0 : 1) + sizes[i];
            size_t to = sizes[i + 1];
            auto boxes = paragraph->getRectsForRange(from, to, rect_height_style, rect_width_style);
            if (boxes.empty()) {
                continue;
            }
            for (auto& box : boxes) {
                SkPaint paint;
                paint.setColor(colors[i % colors.size()]);
                paint.setShader(setgrad(box.rect, colors[i % colors.size()], SK_ColorWHITE));
                canvas->drawRect(box.rect, paint);
            }
        }

        paragraph->paint(canvas, 0, 0);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->height();
        auto w = this->width();

        drawText(canvas, SK_ColorGRAY, w, h);
    }

private:
    typedef Sample INHERITED;
    SkScalar letterSpacing;
    SkScalar wordSpacing;
};

class ParagraphView10 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph10"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, getFontCollection());

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto"),
                                    SkString("Noto Color Emoji"),
                                    SkString("Source Han Serif CN")});
        text_style.setColor(SK_ColorRED);
        text_style.setFontSize(60);
        text_style.setLetterSpacing(0);
        text_style.setWordSpacing(0);
        text_style.setColor(SK_ColorBLACK);
        text_style.setHeight(1);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(width());

        paragraph->paint(canvas, 0, 0);
        SkDEBUGCODE(auto impl = reinterpret_cast<ParagraphImpl*>(paragraph.get()));
        SkASSERT(impl->runs().size() == 3);
        SkASSERT(impl->runs()[0].textRange().end == impl->runs()[1].textRange().start);
        SkASSERT(impl->runs()[1].textRange().end == impl->runs()[2].textRange().start);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView11 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph11"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto text = "\U0001f469\U0000200D\U0001f469\U0000200D\U0001f466\U0001f469\U0000200D\U0001f469\U0000200D\U0001f467\U0000200D\U0001f467\U0001f1fa\U0001f1f8";
            TextStyle text_style;
            text_style.setFontFamilies({SkString("Ahem")});
            text_style.setColor(SK_ColorBLACK);
            text_style.setFontSize(60);
            text_style.setLetterSpacing(0);
            text_style.setWordSpacing(0);
            ParagraphStyle paragraph_style;
            paragraph_style.setTextStyle(text_style);
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.addText(text, strlen(text));
            auto paragraph = builder.Build();
            paragraph->layout(1000);
            paragraph->paint(canvas, 0, 0);

            SkColor colors[] = { SK_ColorRED, SK_ColorBLACK, SK_ColorBLUE, SK_ColorTRANSPARENT, SK_ColorTRANSPARENT };
            SkPoint queries[] = {{ 1, 3},{1, 5}, {1, 9}, { 1, 17}, {1, 33}};
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setAntiAlias(true);
            paint.setStrokeWidth(5);
            for (auto& query : queries) {
                auto rects = paragraph->getRectsForRange(query.fX, query.fY, RectHeightStyle::kTight, RectWidthStyle::kTight);
                paint.setColor(colors[&query - &queries[0]]);
                for (auto& rect: rects) {
                    canvas->drawRect(rect.rect, paint);
                }
            }
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView12 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph12"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text = "Atwater Peel Sherbrooke Bonaventure Angrignon Peel Côte-des-Neiges";
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontSize(16);
        //text_style.setLetterSpacing(-0.41);
        StrutStyle strut_style;
        strut_style.setStrutEnabled(false);
        ParagraphStyle paragraph_style;
        paragraph_style.setStrutStyle(strut_style);
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(1095.000000);
        auto result = paragraph->getRectsForRange(65, 66, RectHeightStyle::kTight, RectWidthStyle::kTight);
        paragraph->paint(canvas, 0, 0);

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        canvas->drawRect(result.front().rect, paint);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView13 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph13"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text = "This\n"
                           "is a wrapping test. It should wrap at manual newlines, and if softWrap is true, also at spaces.";
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontSize(10);

        auto relayout = [&](size_t lines, bool ellipsis,
                SkScalar width, SkScalar height, SkScalar minWidth, SkScalar maxWidth, SkColor bg) {
            ParagraphStyle paragraph_style;
            SkPaint paint;
            paint.setColor(bg);
            text_style.setForegroundColor(paint);
            paragraph_style.setTextStyle(text_style);
            paragraph_style.setMaxLines(lines);
            if (ellipsis) {
                paragraph_style.setEllipsis(u"\u2026");
            }
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.addText(text);
            auto paragraph = builder.Build();
            paragraph->layout(50);
            paragraph->paint(canvas, 0, 0);
            canvas->translate(0, paragraph->getHeight() + 10);

            SkASSERT(width == paragraph->getMaxWidth());
            SkASSERT(height == paragraph->getHeight());
            SkASSERT(minWidth == paragraph->getMinIntrinsicWidth());
            SkASSERT(maxWidth == paragraph->getMaxIntrinsicWidth());
        };

        SkPaint paint;
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 50, 500), paint);

        relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
        relayout(3, false, 50, 30,  50, 950, SK_ColorBLUE);
        relayout(std::numeric_limits<size_t>::max(), false, 50, 200,  50, 950, SK_ColorGREEN);

        relayout(1, true, 50, 10, 950, 950, SK_ColorYELLOW);
        relayout(3, true, 50, 30,  50, 950, SK_ColorMAGENTA);
        relayout(std::numeric_limits<size_t>::max(), true, 50, 20,  950, 950, SK_ColorCYAN);

        relayout(1, false, 50, 10, 950, 950, SK_ColorRED);
        relayout(3, false, 50, 30,  50, 950, SK_ColorBLUE);
        relayout(std::numeric_limits<size_t>::max(), false, 50, 200,  50, 950, SK_ColorGREEN);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView14 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph14"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontSize(25);
        text_style.setDecoration((TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
        text_style.setDecorationColor(SK_ColorBLUE);
        text_style.setDecorationStyle(TextDecorationStyle::kWavy);
        text_style.setDecorationThicknessMultiplier(4.0f);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
        builder.pushStyle(text_style);
        builder.addText("Hello, wor!\nabcd.");
        auto paragraph = builder.Build();
        paragraph->layout(300);
        paragraph->paint(canvas, 0, 0);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 300, 100), paint);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView15 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph15"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
        text_style.setFontSize(16);
        builder.pushStyle(text_style);
        builder.addText("C ");
        text_style.setFontSize(20);
        builder.pushStyle(text_style);
        builder.addText("He");
        builder.pop();
        PlaceholderStyle placeholderStyle;
        placeholderStyle.fHeight = 55.0f;
        placeholderStyle.fWidth = 50.0f;
        placeholderStyle.fBaseline = TextBaseline::kAlphabetic;
        placeholderStyle.fAlignment = PlaceholderAlignment::kBottom;
        builder.addPlaceholder(placeholderStyle);
        text_style.setFontSize(16);
        builder.pushStyle(text_style);
        builder.addText("hello world! sieze the day!");
        auto paragraph = builder.Build();
        paragraph->layout(400);
        paragraph->paint(canvas, 0, 0);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 400, 200), paint);

        auto phs = paragraph->getRectsForPlaceholders();
        for (auto& ph : phs) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(SK_ColorYELLOW);
            canvas->drawRect(ph.rect, paint);
        }
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE(return new ParagraphView1();)
DEF_SAMPLE(return new ParagraphView2();)
DEF_SAMPLE(return new ParagraphView3();)
DEF_SAMPLE(return new ParagraphView4();)
DEF_SAMPLE(return new ParagraphView5();)
DEF_SAMPLE(return new ParagraphView6();)
DEF_SAMPLE(return new ParagraphView7();)
DEF_SAMPLE(return new ParagraphView8();)
DEF_SAMPLE(return new ParagraphView9();)
DEF_SAMPLE(return new ParagraphView10();)
DEF_SAMPLE(return new ParagraphView11();)
DEF_SAMPLE(return new ParagraphView12();)
DEF_SAMPLE(return new ParagraphView13();)
DEF_SAMPLE(return new ParagraphView14();)
DEF_SAMPLE(return new ParagraphView15();)
