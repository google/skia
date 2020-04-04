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

        const char* text = "0";
        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(std::numeric_limits<size_t>::max());
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Google Sans Display")});
        text_style.setFontSize(160);
        //text_style.setHeightOverride(true);
        //text_style.setHeight(1.75);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(94);

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
