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
#include "src/utils/SkOSPath.h"
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
            fFC = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
        }
        return fFC;
    }
};

sk_sp<SkShader> setgrad(const SkRect& r, SkColor c0, SkColor c1) {
    SkColor colors[] = {c0, c1};
    SkPoint pts[] = {{r.fLeft, r.fTop}, {r.fRight, r.fTop}};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}
/*
void writeHtml(const char* name, Paragraph* paragraph) {
        SkString tmpDir = skiatest::GetTmpDir();
        if (!tmpDir.isEmpty()) {
            SkString path = SkOSPath::Join(tmpDir.c_str(), name);
            SkFILEWStream file(path.c_str());
            file.write(nullptr, 0);
        }
}
*/
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

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        const char* text =
                "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";
        auto multiplier = 5.67;
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(TextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        TextStyle textStyle;
        textStyle.setFontFamilies({SkString("Roboto")});
        textStyle.setFontSize(5 * multiplier);
        textStyle.setHeight(1.3f);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
        builder.pushStyle(textStyle);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(200 * multiplier);

        std::vector<size_t> sizes = {0, 1, 2, 8, 19, 21, 22, 30, 150};

        std::vector<size_t> colors = {SK_ColorBLUE, SK_ColorCYAN,  SK_ColorLTGRAY, SK_ColorGREEN,
                                      SK_ColorRED,  SK_ColorWHITE, SK_ColorYELLOW, SK_ColorMAGENTA};

        RectHeightStyle rect_height_style = RectHeightStyle::kTight;
        RectWidthStyle rect_width_style = RectWidthStyle::kTight;

        for (size_t i = 0; i < sizes.size() - 1; ++i) {
            size_t from = sizes[i];
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
        auto multiplier = 5.67;
        const char* text = "English English 字典 字典 😀😃😄 😀😃😄";

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto"),
                                    SkString("Noto Color Emoji"),
                                    SkString("Noto Serif CJK JP")});
        text_style.setFontSize(10 * multiplier);
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

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true, true);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.addText(text, strlen(text));
        auto paragraph = builder.Build();
        paragraph->layout(1000);
        paragraph->paint(canvas, 0, 0);

        struct pair {
            unsigned fX;
            unsigned fY;
        };

        pair hit1[] =
              {{ 0, 8},{1, 33}, {2, 34}, { 3, 19}, {4, 20},
               { 5, 21}, { 6, 22 }, { 7, 23 }, {8, 24 }, { 9, 25},
               { 10, 26}, { 11, 27}, {12, 28}, { 13, 21}, {14, 22 },
               { 15, 23}, {16, 24}, {17, 21}, { 18, 22}, {19, 21},
               { 20, 24}, { 21, 23}, };

        pair miss[] =
              {{ 0, 4},{1, 17}, {2, 18}, { 3, 11}, {4, 12},
               { 5, 13}, { 6, 14 }, { 7, 15 }, {8, 16 }, { 9, 17},
               { 10, 18}, { 11, 19}, {12, 20}, { 13, 17}, {14, 18 },
               { 15, 19}, {16, 20}, {17, 19}, { 18, 20},
               { 20, 22}, };

        auto rects = paragraph->getRectsForRange(7, 9, RectHeightStyle::kTight, RectWidthStyle::kTight);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        if (!rects.empty()) {
            canvas->drawRect(rects[0].rect, paint);
        }

        for (auto& query : hit1) {
            auto rects = paragraph->getRectsForRange(query.fX, query.fY, RectHeightStyle::kTight, RectWidthStyle::kTight);
            if (rects.size() >= 1 && rects[0].rect.width() > 0) {
            } else {
                SkDebugf("+[%d:%d): Bad\n", query.fX, query.fY);
            }
        }

        for (auto& query : miss) {
            auto miss = paragraph->getRectsForRange(query.fX, query.fY, RectHeightStyle::kTight, RectWidthStyle::kTight);
            if (miss.empty()) {
            } else {
                SkDebugf("-[%d:%d): Bad\n", query.fX, query.fY);
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
        text_style.setFontFamilies({SkString("abc.ttf")});
        text_style.setFontSize(50);

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);

        fontCollection->addFontFromFile("abc/abc.ttf", "abc");
        fontCollection->addFontFromFile("abc/abc+grave.ttf", "abc+grave");
        fontCollection->addFontFromFile("abc/abc+agrave.ttf", "abc+agrave");

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        text_style.setFontFamilies({SkString("abc"), SkString("abc+grave")});
        text_style.setColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText(u"a\u0300");
        text_style.setColor(SK_ColorMAGENTA);
        builder.pushStyle(text_style);
        builder.addText(u"à");

        text_style.setFontFamilies({SkString("abc"), SkString("abc+agrave")});

        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText(u"a\u0300");
        text_style.setColor(SK_ColorGREEN);
        builder.pushStyle(text_style);
        builder.addText(u"à");

        auto paragraph = builder.Build();
        paragraph->layout(800);
        paragraph->paint(canvas, 50, 50);

    }

private:
    typedef Sample INHERITED;
};

class ParagraphView16 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph16"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text = "content";

        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(1);
        paragraph_style.setEllipsis(u"\u2026");
        //auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        TextStyle text_style;
        text_style.setFontFamilies({SkString(".SF Pro Text")});
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontSize(17.0f * 99.0f);
        text_style.setLetterSpacing(0.41f);
        builder.pushStyle(text_style);
        builder.addText(text);

        auto paragraph = builder.Build();
        paragraph->layout(800);
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView17 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph17"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        auto navy = SkColorSetRGB(0, 0, 139);
        auto ltgray = SkColorSetRGB(211, 211, 211);
        auto multiplier = 5.67;

        const char* text = ">Sͬ͑̀͐̈͒̈́̋̎ͮͩ̽̓ͬ̂̆̔͗́̓ͣͧ͊ͫ͛̉͌̐̑ͪ͗̚͝҉̴͉͢k̡̊̓ͫͭͩ͂͊ͨͪͬ̑ͫ̍̌̄͛̌̂̑̂̋̊̔ͫ͛̽̑ͨ̍ͭ̓̀ͪͪ̉͐͗̌̓̃̚͟͝҉̢͏̫̞̙͇͖̮͕̗̟͕͇͚̻͈̣̻̪͉̰̲̣̫ͅͅP̴̅̍͒̿͗͗̇ͩ̃͆͌̀̽͏̧̡͕͖̝̖̼̺̰̣̬͔͖͔̼͙̞̦̫͓̘͜a̸̴̸̴̢̢̨̨̫͍͓̥̼̭̼̻̤̯̙̤̻̠͚̍̌͋̂ͦͨ̽̇͌͌͆̀̽̎͒̄ͪ̐ͦ̈ͫ͐͗̓̚̚͜ͅr͐͐ͤͫ̐ͥ͂̈́̿́ͮ̃͗̓̏ͫ̀̿͏̸̵̧́͘̕͟͝͠͞͠҉̷̧͚͢͟a̓̽̎̄͗̔͛̄̐͊͛ͫ͂͌̂̂̈̈̓̔̅̅̄͊̉́ͪ̑̄͆ͬ̍͆ͭ͋̐ͬ͏̷̵̨̢̩̹̖͓̥̳̰͔̱̬͖̙͓̙͇̀̀̕͜͟͟͢͟͜͠͡g̨̅̇ͦ͋̂ͦͨͭ̓͐͆̏̂͛̉ͧ̑ͫ̐̒͛ͫ̍̒͛́̚҉̷̨̛̛̀͜͢͞҉̩̘̲͍͎̯̹̝̭̗̱͇͉̲̱͔̯̠̹̥̻͉̲̜̤̰̪̗̺̖̺r̷͌̓̇̅ͭ̀̐̃̃ͭ͑͗̉̈̇̈́ͥ̓ͣ́ͤ͂ͤ͂̏͌̆̚҉̴̸̧̢̢̛̫͉̦̥̤̙͈͉͈͉͓̙̗̟̳̜͈̗̺̟̠̠͖͓̖̪͕̠̕̕͝ͅả̸̴̡̡̧͠͞͡͞҉̛̕͟͏̷̘̪̱͈̲͉̞̠̞̪̫͎̲̬̖̀̀͟͝͞͞͠p̛͂̈͐̚͠҉̵̸̡̢̢̩̹͙̯͖̙̙̮̥̙͚̠͔̥̭̮̞̣̪̬̥̠̖̝̥̪͎́̀̕͜͡͡ͅͅh̵̷̵̡̛ͤ̂͌̐̓̐̋̋͊̒̆̽́̀̀̀͢͠͞͞҉̷̸̢̕҉͚̯͖̫̜̞̟̠̱͉̝̲̹̼͉̟͉̩̮͔̤͖̞̭̙̹̬ͅ<";

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        SkPaint paint;
        paint.setColor(ltgray);
        TextStyle text_style;
        text_style.setBackgroundColor(paint);
        text_style.setColor(navy);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20 * multiplier);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(10000);
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class Zalgo {
    private:
    std::u16string COMBINING_DOWN = u"\u0316\u0317\u0318\u0319\u031c\u031d\u031e\u031f\u0320\u0324\u0325\u0326\u0329\u032a\u032b\u032c\u032d\u032e\u032f\u0330\u0331\u0332\u0333\u0339\u033a\u033b\u033c\u0345\u0347\u0348\u0349\u034d\u034e\u0353\u0354\u0355\u0356\u0359\u035a\u0323";
    std::u16string COMBINING_UP = u"\u030d\u030e\u0304\u0305\u033f\u0311\u0306\u0310\u0352\u0357\u0351\u0307\u0308\u030a\u0342\u0343\u0344\u034a\u034b\u034c\u0303\u0302\u030c\u0350\u0300\u0301\u030b\u030f\u0312\u0313\u0314\u033d\u0309\u0363\u0364\u0365\u0366\u0367\u0368\u0369\u036a\u036b\u036c\u036d\u036e\u035b\u0346\u031a";
    std::u16string COMBINING_MIDDLE = u"\u0315\u031b\u0340\u0341\u0358\u0321\u0322\u0327\u0328\u0334\u0335\u0336\u034f\u035c\u035d\u035e\u035f\u0360\u0362\u0338\u0337\u0361\u0489";

    std::u16string randomMarks(std::u16string& combiningMarks) {
        std::u16string result;
        auto num = std::rand() % (combiningMarks.size() / 1);
        for (size_t i = 0; i < num; ++i) {
            auto index = std::rand() % combiningMarks.size();
            result += combiningMarks[index];
        }
        return result;
    }

public:
    std::u16string zalgo(std::string victim) {
        std::u16string result;
        for (auto& c : victim) {
            result += c;
            result += randomMarks(COMBINING_UP);
            result += randomMarks(COMBINING_MIDDLE);
            result += randomMarks(COMBINING_DOWN);
        }
        return result;
    }
};

class ParagraphView18 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph18"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case ' ':
                    fLimit = 400;
                    return true;
                case 's':
                    fLimit += 10;
                    return true;
                case 'f':
                    if (fLimit > 10) {
                        fLimit -= 10;
                    }
                    return true;
                default:
                    break;
            }
            return false;
    }

    bool onAnimate(double nanos) override {
        if (++fIndex > fLimit) {
            fRedraw = true;
            fIndex = 0;
        } else {
            fRepeat = true;
        }
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto navy = SkColorSetRGB(0, 0, 139);
        auto ltgray = SkColorSetRGB(211, 211, 211);

        auto multiplier = 5.67;
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20 * multiplier);
        text_style.setColor(navy);
        SkPaint paint;
        paint.setColor(ltgray);
        text_style.setBackgroundColor(paint);

        Zalgo zalgo;

        if (fRedraw || fRepeat) {

            if (fRedraw || fParagraph.get() == nullptr) {
                ParagraphBuilderImpl builder(paragraph_style, fontCollection);
                builder.pushStyle(text_style);
                auto utf16text = zalgo.zalgo("SkParagraph");
                icu::UnicodeString unicode((UChar*)utf16text.data(), SkToS32(utf16text.size()));
                std::string str;
                unicode.toUTF8String(str);
                SkDebugf("Text:>%s<\n", str.data());
                builder.addText(utf16text);
                fParagraph = builder.Build();
            }

            auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
            impl->setState(InternalState::kUnknown);
            fParagraph->layout(1000);
            fParagraph->paint(canvas, 300, 200);

            for (auto& run : impl->runs()) {
                SkString fontFamily("unresolved");
                if (run.font().getTypeface() != nullptr) {
                    run.font().getTypeface()->getFamilyName(&fontFamily);
                }
                if (run.font().getTypeface() != nullptr) {
                    for (size_t i = 0; i < run.size(); ++i) {
                        auto glyph = run.glyphs().begin() + i;
                        if (*glyph == 0) {
                            SkDebugf("Run[%d] @pos=%d\n", run.index(), i);
                            SkASSERT(false);
                        }
                    }
                } else {
                    SkDebugf("Run[%d]: %s\n", run.index(), fontFamily.c_str());
                    SkASSERT(false);
                }
            }
            fRedraw = false;
            fRepeat = false;
        }
    }

private:
    bool fRedraw = true;
    bool fRepeat = false;
    size_t fIndex = 0;
    size_t fLimit = 20;
    std::unique_ptr<Paragraph> fParagraph;
    typedef Sample INHERITED;
};

class ParagraphView19 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph19"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);

        std::u16string text = u"\u0068\u0301\u0350\u0312\u0357\u030C\u0369\u0305\u036C\u0304\u0310\u033F\u0366\u0350\u0343\u0364\u0369\u0311\u0309\u030E\u0365\u031B\u0340\u0337\u0335\u035E\u0334\u0328\u0360\u0360\u0315\u035F\u0340\u0340\u0362\u0360\u0322\u031B\u031B\u0337\u0340\u031E\u031F\u032A\u0331\u0345\u032F\u0332\u032E\u0333\u0353\u0320\u0345\u031C\u031F\u033C\u0325\u0355\u032C\u0325\u033Aa\u0307\u0312\u034B\u0308\u0312\u0346\u0313\u0346\u0304\u0307\u0344\u0305\u0342\u0368\u0346\u036A\u035B\u030F\u0365\u0307\u0340\u0328\u0322\u0361\u0489\u034F\u0328\u0334\u035F\u0335\u0362\u0489\u0360\u0358\u035E\u0360\u035D\u0341\u0337\u0337\u032E\u0326\u032D\u0359\u0318\u033C\u032F\u0333\u035A\u034D\u0319\u031C\u0353\u033C\u0345\u0359\u0331\u033B\u0331\u033C";
        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(this->width());
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView20 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph20"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);

        const char* text =  "Manage your google account";
        ParagraphStyle paragraph_style;
        paragraph_style.setEllipsis(u"\u2026");
        paragraph_style.setMaxLines(1);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(50);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(this->width());
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView21 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph21"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text =  "Referral Code";
        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Google Sans")});
        text_style.setFontSize(24);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(0);
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView22 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph22"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case 'l':
                    direction = true;
                    return true;
                case 'r':
                    direction = false;
                    return true;
                default:
                    break;
            }
            return false;
    }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(direction ? TextDirection::kLtr : TextDirection::kRtl);
        auto collection = getFontCollection();
        ParagraphBuilderImpl builder(paragraph_style, collection);
        collection->getParagraphCache()->reset();
        collection->getParagraphCache()->turnOn(false);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(12);
        builder.pushStyle(text_style);
        builder.addText("I have got a ");
        text_style.setFontStyle(SkFontStyle::Bold());
        builder.pushStyle(text_style);
        builder.addText("lovely bunch");
        text_style.setFontStyle(SkFontStyle::Normal());
        builder.pushStyle(text_style);
        builder.addText(" of coconuts.");
        auto paragraph = builder.Build();
        paragraph->layout(this->width());
        paragraph->paint(canvas, 0, 0);
        collection->getParagraphCache()->turnOn(true);
    }

private:
    typedef Sample INHERITED;
    bool direction;
};

class ParagraphView23 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph23"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text =  "Text with shadow";
        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Google Sans")});
        text_style.setFontSize(24);

        auto draw = [&](SkScalar h, SkScalar v, SkScalar b) {
            text_style.resetShadows();
            text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(h, v), b));
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText(text);
            auto paragraph = builder.Build();
            paragraph->layout(300);
            paragraph->paint(canvas, 0, 0);

            auto rect = SkRect::MakeXYWH(0, 0, paragraph->getMaxWidth(), paragraph->getHeight());
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setAntiAlias(true);
            paint.setStrokeWidth(1);
            canvas->drawRect(rect, paint);
        };

        draw(10, 10, 5);
        canvas->translate(0, 100);

        draw(10, -10, 5);
        canvas->translate(0, 100);

        draw(-10, -10, 5);
        canvas->translate(0, 100);

        draw(-10, 10, 5);
        canvas->translate(0, 100);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView24 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph24"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kRtl);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Google Sans")});
        text_style.setFontSize(24);
        {
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText("Right_to_left:");
            auto paragraph = builder.Build();
            paragraph->layout(this->width());
            paragraph->paint(canvas, 0, 0);
        }
        canvas->translate(0, 200);
        {
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText("Right_to_left+");
            auto paragraph = builder.Build();
            paragraph->layout(this->width());
            paragraph->paint(canvas, 0, 0);
        }
        canvas->translate(0, 200);
        {
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText("Right_to_left.");
            auto paragraph = builder.Build();
            paragraph->layout(this->width());
            paragraph->paint(canvas, 0, 0);
        }
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView25 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph25"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
/*
 * Shell: ParagraphStyle: 1.000000 1
Shell: Strut enabled: 0 1.000000 14.000000 400 5 0
Shell: Font Families: 0
Shell: DefaultTextStyle: 16.000000 500 5 0
Shell: Font Families: 1 Roboto
Shell: Font Features: 0
Shell: TextStyle#0: [0:22) 16.000000 500 5 0
Shell: Font Families: 1 Roboto
Shell: Font Features: 0
Shell: TextStyle#1: [25:49) 16.000000 500 5 0
Shell: Font Families: 1 Roboto
Shell: Font Features: 0
Shell: Placeholder#0: [22:25) 32.000000 32.000000 32.000000 0 5
Shell: Placeholder#1: [49:52) 19.000000 41.000000 19.000000 0 4
Shell: Placeholder#2: [52:52) 0.000000 0.000000 0.000000 0 5
Shell: layout('Go to device settings ￼ and set up a passcode. ￼', 280.000000): 280.000000 * 38.000000
 */
        auto fontCollection = getFontCollection();
        //fontCollection->getParagraphCache()->turnOn(false);
        const char* text1 =  "Go to device settings ";
        const char* text2 = "and set up a passcode.";
        ParagraphStyle paragraph_style;
        StrutStyle strut_style;
        strut_style.setStrutEnabled(false);
        strut_style.setFontSize(14);
        strut_style.setForceStrutHeight(false);
        strut_style.setHeight(14);
        paragraph_style.setStrutStyle(strut_style);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(16);
        PlaceholderStyle placeholder_style;
        {
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style);
            builder.addText(text1);
            placeholder_style.fHeight = 32;
            placeholder_style.fWidth = 32;
            placeholder_style.fBaselineOffset = 32;
            placeholder_style.fBaseline = TextBaseline::kAlphabetic;
            placeholder_style.fAlignment = PlaceholderAlignment::kMiddle;
            builder.addPlaceholder(placeholder_style);
            builder.addText(text2);
            placeholder_style.fHeight = 19;
            placeholder_style.fWidth = 41;
            placeholder_style.fBaselineOffset = 19;
            placeholder_style.fBaseline = TextBaseline::kAlphabetic;
            placeholder_style.fAlignment = PlaceholderAlignment::kTop;
            builder.addPlaceholder(placeholder_style);
            auto paragraph = builder.Build();
            paragraph->layout(280);
            paragraph->paint(canvas, 0, 0);
        }
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView26 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph26"); }

    void onDrawContent(SkCanvas* canvas) override {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        //fontCollection->enableFontFallback();

        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        TextStyle textStyle;
        textStyle.setForegroundColor(paint);
        textStyle.setFontFamilies({ SkString("Roboto") });
        textStyle.setFontSize(42.0f);
        textStyle.setLetterSpacing(-0.05f);
        textStyle.setHeightOverride(true);

        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextStyle(textStyle);
        paragraphStyle.setTextAlign(TextAlign::kLeft);

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
        builder.addText(u"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam ut dolor ornare, fermentum nibh in, consectetur libero. Ut id semper est. Sed malesuada, est id bibendum egestas, urna risus tristique nibh, euismod interdum risus turpis nec purus. Maecenas dolor nisl, consectetur in vestibulum et, tincidunt id leo. Duis maximus, odio eget tristique commodo, lacus tellus dapibus leo, consequat pellentesque arcu nisi sit amet diam. Quisque euismod venenatis egestas. Mauris posuere volutpat iaculis. Suspendisse finibus tempor urna, dignissim venenatis sapien finibus eget. Donec interdum lacus ac venenatis fringilla. Curabitur eget lacinia augue. Vestibulum eu vulputate odio. Quisque nec imperdiet");

        auto paragraph = builder.Build();
        paragraph->layout(this->width() / 2);

        std::vector<LineMetrics> lines;
        paragraph->getLineMetrics(lines); // <-- error happens here

        canvas->translate(10, 10);
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView27 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph27"); }

    void onDrawContent(SkCanvas* canvas) override {
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        fontCollection->getParagraphCache()->turnOn(false);

        SkPaint red;
        red.setColor(SK_ColorRED);
        red.setStyle(SkPaint::kStroke_Style);
        red.setAntiAlias(true);
        red.setStrokeWidth(1);

        SkPaint blue;
        blue.setColor(SK_ColorRED);
        blue.setStyle(SkPaint::kStroke_Style);
        blue.setAntiAlias(true);
        blue.setStrokeWidth(1);

        SkPaint black;
        black.setColor(SK_ColorBLACK);
        black.setStyle(SkPaint::kStroke_Style);
        black.setAntiAlias(true);
        black.setStrokeWidth(1);

        SkPaint whiteSpaces;
        whiteSpaces.setColor(SK_ColorLTGRAY);

        SkPaint breakingSpace;
        breakingSpace.setColor(SK_ColorYELLOW);

        SkPaint text;
        text.setColor(SK_ColorWHITE);

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kRight);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});

        // RTL + right align + arabic
        // RTL + right align + latin
        // LTR + right align + arabic
        // LTR + right align + latin
        // RTL + left align + arabic
        // RTL + left align + latin
        // arabic and latin should not differ at all
        // check: line breaking and trailing spaces

        canvas->drawColor(SK_ColorWHITE);
        auto h = 60;
        auto w = 300;

        auto draw = [&](SkScalar width, SkScalar height, TextDirection td, TextAlign ta, const char* t) {
            SkDebugf("draw '%s' dir:%s align:%s\n", t,
                    td == TextDirection::kLtr ? "left" : "right",
                    ta == TextAlign::kLeft ? "left" : "right");
            paragraph_style.setTextDirection(td);
            paragraph_style.setTextAlign(ta);
            text_style.setFontSize(20);
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            text_style.setBackgroundColor(whiteSpaces);
            builder.pushStyle(text_style);
            builder.addText("   ");
            text_style.setBackgroundColor(text);
            builder.pushStyle(text_style);
            builder.addText(t);
            text_style.setBackgroundColor(breakingSpace);
            builder.pushStyle(text_style);
            builder.addText(" ");
            text_style.setBackgroundColor(text);
            builder.pushStyle(text_style);
            builder.addText(t);
            text_style.setBackgroundColor(whiteSpaces);
            builder.pushStyle(text_style);
            builder.addText("   ");
            auto paragraph = builder.Build();
            paragraph->layout(width);
            paragraph->paint(canvas, 0, 0);
            auto impl = static_cast<ParagraphImpl*>(paragraph.get());
            for (auto& line : impl->lines()) {
                SkDebugf("line[%d]: %f + %f\n", &line - impl->lines().begin(), line.offset().fX, line.shift());
                line.iterateThroughVisualRuns(true,
                    [&](const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
                    *width = line.measureTextInsideOneRun(textRange, run, runOffset, 0, true, false).clip.width();
                    SkDebugf("%d[%d: %d) @%f + %f %s\n", run->index(),
                            textRange.start, textRange.end, runOffset, *width, run->leftToRight() ? "left" : "right");
                    return true;
                });
            }
            auto boxes = paragraph->getRectsForRange(0, 100, RectHeightStyle::kTight, RectWidthStyle::kTight);
            bool even = true;
            for (auto& box : boxes) {
                SkDebugf("[%f:%f,%f:%f] %s\n",
                        box.rect.fLeft, box.rect.fRight, box.rect.fTop, box.rect.fBottom,
                        box.direction == TextDirection::kLtr ? "left" : "right");
                canvas->drawRect(box.rect, even ? red : blue);
                even = !even;
            }
            canvas->translate(0, height);
        };

        canvas->drawRect(SkRect::MakeXYWH(0, 0, w, h * 8), black);

        draw(w, h, TextDirection::kRtl, TextAlign::kRight, "RTL+RIGHT#1234567890");
        draw(w, h, TextDirection::kRtl, TextAlign::kRight, "قففغغغغقففغغغغقففغغغ");

        draw(w, h, TextDirection::kLtr, TextAlign::kRight, "LTR+RIGHT#1234567890");
        draw(w, h, TextDirection::kLtr, TextAlign::kRight, "قففغغغغقففغغغغقففغغغ");

        draw(w, h, TextDirection::kRtl, TextAlign::kLeft, "RTL+LEFT##1234567890");
        draw(w, h, TextDirection::kRtl, TextAlign::kLeft, "قففغغغغقففغغغغقففغغغ");

        draw(w, h, TextDirection::kLtr, TextAlign::kLeft, "LTR+LEFT##1234567890");
        draw(w, h, TextDirection::kLtr, TextAlign::kLeft, "قففغغغغقففغغغغقففغغغ");
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView28 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph28"); }

    void onDrawContent(SkCanvas* canvas) override {

        const char* text = "AAAAA BBBBB CCCCC DDDDD EEEEE FFFFF GGGGG HHHHH IIIII JJJJJ KKKKK LLLLL MMMMM NNNNN OOOOO PPPPP QQQQQ";

        canvas->drawColor(SK_ColorWHITE);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kJustify);
        auto collection = getFontCollection();
        ParagraphBuilderImpl builder(paragraph_style, collection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        auto s = 186;
        paragraph->layout(360 - s);
        paragraph->paint(canvas, 0, 0);
        /*
        paragraph->layout(360);
        paragraph->paint(canvas, 0, 0);
        canvas->translate(0, 400);
        paragraph->layout(354.333);
        paragraph->paint(canvas, 0, 0);
        */
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView29 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph29"); }

    void onDrawContent(SkCanvas* canvas) override {

        const char* text = "ffi";
        canvas->drawColor(SK_ColorWHITE);

        auto collection = getFontCollection();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, collection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(60);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(width());
        paragraph->paint(canvas, 0, 0);
        auto width = paragraph->getLongestLine();
        auto height = paragraph->getHeight();

        auto f1 = paragraph->getGlyphPositionAtCoordinate(width/6, height/2);
        auto f2 = paragraph->getGlyphPositionAtCoordinate(width/2, height/2);
        auto i = paragraph->getGlyphPositionAtCoordinate(width*5/6, height/2);

        SkDebugf("%d(%s) %d(%s) %d(%s)\n",
                f1.position, f1.affinity == Affinity::kUpstream ? "up" : "down",
                f2.position, f2.affinity == Affinity::kUpstream ? "up" : "down",
                i.position, i.affinity == Affinity::kUpstream ? "up" : "down");

        auto rf1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight)[0];
        auto rf2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight, RectWidthStyle::kTight)[0];
        auto rfi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight, RectWidthStyle::kTight)[0];

        SkDebugf("f1: [%f:%f] %s\n",
                rf1.rect.fLeft, rf1.rect.fRight, rf1.direction == TextDirection::kRtl ? "rtl" : "ltr");
        SkDebugf("f2: [%f:%f] %s\n",
                rf2.rect.fLeft, rf2.rect.fRight, rf2.direction == TextDirection::kRtl ? "rtl" : "ltr");
        SkDebugf("i:  [%f:%f] %s\n",
                rfi.rect.fLeft, rfi.rect.fRight, rfi.direction == TextDirection::kRtl ? "rtl" : "ltr");
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView30 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph30"); }

    void onDrawContent(SkCanvas* canvas) override {

        const std::u16string text = u"\U0001f600\U0001f1e6\U0001f1f9\U0001f601\U0001f9f1\U0001f61a\U0001f431\U0001f642\U0001f38e\U0001f60d\U0001f3b9\U0001f917\U0001f6bb\U0001f609\U0001f353\U0001f618\U0001f1eb\U0001f1f0\U0001f468\u200D\U0001f469\u200D\U0001f466\u200D\U0001f466\U0001f468\u200D\U0001f469\u200D\U0001f467\u200D\U0001f466\U0001f468\u200D\U0001f469\u200D\U0001f467\U0001f46a";
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Noto Color Emoji")});
        text_style.setFontSize(60);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(width());


        SkColor colors[] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorMAGENTA,
            SK_ColorYELLOW
        };
        SkPaint paint;
        size_t color = 0;
        for (size_t i = 0; i < text.size(); ++i) {
            auto result = paragraph->getRectsForRange(i, i + 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
            if (result.empty()) {
                SkDebugf("empty [%d:%d)\n", i, i + 1);
                continue;
            }
            auto rect = result[0].rect;
            paint.setColor(colors[color++ % 5]);
            canvas->drawRect(rect, paint);
            SkDebugf("rect [%d:%d): %f:%f\n", i, i + 1, rect.fLeft, rect.fRight);
        }
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView31 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph31"); }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        builder.pushStyle(text_style);
        auto s = u"েن েূথ";
        builder.addText(s);
        auto paragraph = builder.Build();
        paragraph->layout(width());
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView32 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph32"); }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        text_style.setLocale(SkString("ko"));
        builder.pushStyle(text_style);
        builder.addText(u"\u904d ko ");
        text_style.setLocale(SkString("zh_Hant"));
        builder.pushStyle(text_style);
        builder.addText(u"\u904d zh-Hant ");
        text_style.setLocale(SkString("zh_Hans"));
        builder.pushStyle(text_style);
        builder.addText(u"\u904d zh-Hans ");
        text_style.setLocale(SkString("zh_HK"));
        builder.pushStyle(text_style);
        builder.addText(u"\u904d zh-HK ");
        auto paragraph = builder.Build();
        paragraph->layout(width());
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView33 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph33"); }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kJustify);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Color Emoji")});
        text_style.setFontSize(36);
        builder.pushStyle(text_style);
        builder.addText(u"AAAAA \U0001f600 BBBBB CCCCC DDDDD EEEEE");
        auto paragraph = builder.Build();
        paragraph->layout(width() / 2);
        SkPaint paint;
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, width()/2, paragraph->getHeight()), paint);
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView34 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph34"); }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);
        auto text = "ضخمة ص ،😁😂🤣ضضض ؤ،،😗😗😍😋شسي،😗😁😁ؤرى،😗😃😄😍ببب،🥰😅🥰🥰🥰ثيلااتن";
        //auto text = "ى،😗😃😄😍بب";
        //auto text1 = "World domination is such an ugly phrase - I prefer to call it world optimisation";
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Noto Color Emoji")});
        text_style.setFontSize(50);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(1041); // 1041

        SkColor colors[] = {SK_ColorBLUE, SK_ColorCYAN,  SK_ColorLTGRAY, SK_ColorGREEN,
                            SK_ColorRED,  SK_ColorWHITE, SK_ColorYELLOW, SK_ColorMAGENTA };
        SkPaint paint;
        size_t wordPos = 0;
        size_t index = 0;
        while (wordPos < 72) {
            auto res2 = paragraph->getWordBoundary(wordPos);
            if (res2.width() == 0) {
                break;
            }
            wordPos = res2.end;
            auto res3 = paragraph->getRectsForRange(
                    res2.start, res2.end,
                    RectHeightStyle::kTight, RectWidthStyle::kTight);
            paint.setColor(colors[index % 8]);
            ++index;
            if (!res3.empty()) {
                canvas->drawRect(res3[0].rect, paint);
            }
        }
        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView35 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph35"); }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return new Click;
    }

    bool onClick(Click* click) override {
        fPoint = click->fCurr;
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto text = u"hzbzzj sjsjjs sjkkahgafa\u09A4\u09A1\u09A4\u09A0\u09A4\u09A0 jsjzjgvsh sjsjsksbsbsjs sjjajajahhav jssjbxx jsisudg \u09AF\u09A0\u09AF\u09A0\u09A4\u09A0\u09A4\u09A0\u09A5 \u062A\u0624\u062A\u064A\u0646\u0646\u064A\u0621\u0646\u0627\u0644\u0631\u0631\u064A\u0644\u0627 \u062A\u062A\u0644\u0649 \u062A\u0627\u0631\u064A\u062E \u062A\u0633\u0628\u0628 \u0624\u062A\u064A\u062A\u0624\u062A\u0624\u062A\u0624\u062A\u0624 dhishsbs \u7238\u7238\u4E0D\u5BF9\u52B2\u5927\u5BB6\u90FD\u597D\u8BB0\u5F97\u8BB0\u5F97hshs\u099B\u09A1\u099B\u09A1\u099A jdjdj jdjdjd dbbdbdbdbddbnd\u09A2\u099B\u09A1\u09A2\u09A3\u099B\u09B0\u099A\u0998\u09A0\u09A0\u09B8\u09AB\u0997\u09A3\u09A4\u099C\u09B0\u09A5\u099B\u099B\u09A5\u09A6\u099D\u09A6\u09B2\u09A5\u09A4\u09A3\u09A2\u0997\u0996\u09A0\u0998\u0999\u09A3\u099A\u09A5\u09A4\u09A3\u062A\u0628\u0646\u064A\u0646 \u09A5\u09A3\u09A3 \u09A4\u0998\u0998\u0998\u099B\u09A4 \u09A4\u09A3 \u09A3\u0998\u09A2\u09A3\u0999\u0648\u064A\u0648\u0621\u062A\u064A\u0632\u0633\u0646\u0632\u0624\u0624\u0645\u0645\u0624\u0648\u0624\u0648\u0648\u064A\u0646\u0624\u0646\u0624\u0646\u0624\u0624 \u09A4\u09A4\u09A2\u09A2\u09A4\u09A4 \u0999\u0998\u0997\u09C1\u099B\u09A5 \u09A4\u0997\u0998\u09A3\u099A\u099C\u09A6\u09A5\u0632\u0624\u0648\u0624\u0648\u0624 \u09A4\u09A4\u09A3\u0998\u09A2\u09A4\u099B\u09A6\u09A5\u09A4\u0999\u0998\u09A3 \u0648\u0624\u0648\u0624\u0648\u0624\u0632\u0624\u0646\u0633\u0643\u0633\u0643\u0628\u0646\u09A4\u09AD\u0996\u0996\u099F\u09C0\u09C1\u099B\u09A6\u09C0\u09C1\u09C2\u09C7\u0648\u0624\u0646\u0621\u0646\u0624\u0646 \u09C7\u09C2\u09C0\u09C2\u099A\u09A3\u09A2\u09A4\u09A5\u09A5\u0632\u064A\u09C7\u09C2\u09C0\u09C2\u099A\u09A3\u09A2\u09AE\u09A4\u09A5\u09A5 \U0001f34d\U0001f955\U0001f4a7\U0001f4a7\U0001f4a6\U0001f32a";
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        //paragraph_style.setTextAlign(TextAlign::kJustify);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Color Emoji")});
        text_style.setFontSize(40);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(width());//758

        //auto res1 = paragraph->getGlyphPositionAtCoordinate(line.width() + line.spacesWidth() / 2, line.offset().fY + 10);
        //auto res2 = paragraph->getWordBoundary(res1.position);
        auto res1 = paragraph->getRectsForRange(360, 361, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto res2 = paragraph->getRectsForRange(359, 360, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto res3 = paragraph->getRectsForRange(358, 359, RectHeightStyle::kTight, RectWidthStyle::kTight);

        auto draw = [&](std::vector<TextBox> res, SkColor color) {
            SkPaint paint;
            paint.setColor(color);
            for (auto& r : res) {
                canvas->drawRect(r.rect, paint);
            }
        };

        draw(res1, SK_ColorRED);
        draw(res2, SK_ColorGREEN);
        draw(res3, SK_ColorBLUE);

        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
    SkPoint fPoint;
};

class ParagraphView36 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph36"); }

    void onDrawContent(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto text = "其实就是要 English 却又无法预览 English 功能强大起来 English 了我觉得自己也 English 可以的时候我们";
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Serif CJK JP")});
        text_style.setFontSize(10);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(width());

        auto res1 = paragraph->getRectsForRange(63, 64, RectHeightStyle::kTight, RectWidthStyle::kTight);
        auto rect = res1[0].rect;
        auto res2 = paragraph->getGlyphPositionAtCoordinate(rect.fLeft + rect.width() / 2, rect.height());
        auto res3 = paragraph->getWordBoundary(res2.position);
        SkDebugf("[%d:%d)\n", res3.start, res3.end);

        paragraph->paint(canvas, 0, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView37 : public ParagraphView_Base {
protected:
    SkString name() override { return SkString("Paragraph37"); }

    void onDrawContent(SkCanvas* canvas) override {
        const char* text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaয়ৠঝোণ৺ঢ়মৈবৗৗঘথফড়৭২খসঢ়ৃঢ়ঁ৷থডঈঽলবনদ২ৢৃঀজঝ৩ঠ৪৫৯০ঌয়্মওৗ৲গখদ৹ঈ৴৹ঢ়ৄএৡফণহলঈ৲থজোৱে ঀকৰঀষজঝঃাখশঽএমংি";
                //"ৎৣ়ৎঽতঃ৳্ৱব৴ৣঈ৷ূঁঢঢ়শটডৎ৵৵ৰৃ্দংঊাথৗদঊউদ৯ঐৃধা৬হওধি়৭ঽম৯স০ঢফৈঢ়কষঁছফীআে৶ৰ৶ঌৌঊ্ঊঝএঀঃদঞ৮তব৬ৄঊঙঢ়ৡগ৶৹৹ঌড়ঘৄ৷লপ১ভড়৶েঢ়৯ৎকনংট২ংএঢৌৌঐনো০টঽুৠগআ৷৭৩৬তো৻ঈ০ূসষঅঝআমণঔা১ণৈো৵চঽ৩বমৎঙঘ২ঠৠৈী৫তঌণচ৲ঔী৮ঘৰঔ";
         canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        auto w = width() / 2;
        paragraph->layout(w);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        auto clusters = impl->clusters();
        size_t c = 0;
        SkDebugf("clusters\n");
        for (auto& cluster: clusters) {
          SkDebugf(""
                   "%d: [%d:%d) %s\n", c++,
              cluster.textRange().start, cluster.textRange().end,
              cluster.isSoftBreak() ? "soft" :
                cluster.isHardBreak() ? "hard" :
                  cluster.isWhitespaces() ? "spaces" : ""
              );
        }
        auto lines = impl->lines();
        size_t i = 0;
        SkDebugf("lines\n");
        for (auto& line : lines) {
          SkDebugf("%d: [%d:%d)\n", i++, line.trimmedText().start, line.trimmedText().end);
        }

        paragraph->paint(canvas, 0, 0);
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
DEF_SAMPLE(return new ParagraphView14();)
DEF_SAMPLE(return new ParagraphView15();)
DEF_SAMPLE(return new ParagraphView16();)
DEF_SAMPLE(return new ParagraphView17();)
DEF_SAMPLE(return new ParagraphView18();)
DEF_SAMPLE(return new ParagraphView19();)
DEF_SAMPLE(return new ParagraphView20();)
DEF_SAMPLE(return new ParagraphView21();)
DEF_SAMPLE(return new ParagraphView22();)
DEF_SAMPLE(return new ParagraphView23();)
DEF_SAMPLE(return new ParagraphView24();)
DEF_SAMPLE(return new ParagraphView25();)
DEF_SAMPLE(return new ParagraphView26();)
DEF_SAMPLE(return new ParagraphView27();)
DEF_SAMPLE(return new ParagraphView28();)
DEF_SAMPLE(return new ParagraphView29();)
DEF_SAMPLE(return new ParagraphView30();)
DEF_SAMPLE(return new ParagraphView31();)
DEF_SAMPLE(return new ParagraphView32();)
DEF_SAMPLE(return new ParagraphView33();)
DEF_SAMPLE(return new ParagraphView34();)
DEF_SAMPLE(return new ParagraphView35();)
DEF_SAMPLE(return new ParagraphView36();)
DEF_SAMPLE(return new ParagraphView37();)
