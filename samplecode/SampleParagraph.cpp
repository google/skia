/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <dirent.h>
#include <sstream>
#include "Sample.h"
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
#include "modules/skparagraph/include/SkParagraph.h"
#include "modules/skparagraph/include/SkParagraphBuilder.h"
#include "modules/skparagraph/src/SkParagraphImpl.h"
#include "modules/skparagraph/src/SkTypefaceFontProvider.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"

#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTHOST_WIN_GDI)
extern SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT&);
#endif

static const char gText[] =
        "When in the Course of human events it becomes necessary for one people "
        "to dissolve the political bands which have connected them with another "
        "and to assume among the powers of the earth, the separate and equal "
        "station to which the Laws of Nature and of Nature's God entitle them, "
        "a decent respect to the opinions of mankind requires that they should "
        "declare the causes which impel them to the separation.";

static const std::vector<
        std::tuple<std::string, bool, bool, int, SkColor, SkColor, bool, SkTextDecorationStyle>>
        gParagraph = {{"monospace", true, false, 14, SK_ColorWHITE, SK_ColorRED, true,
                       SkTextDecorationStyle::kDashed},
                      {"Assyrian", false, false, 20, SK_ColorWHITE, SK_ColorBLUE, false,
                       SkTextDecorationStyle::kDotted},
                      {"serif", true, true, 10, SK_ColorWHITE, SK_ColorRED, true,
                       SkTextDecorationStyle::kDouble},
                      {"Arial", false, true, 16, SK_ColorGRAY, SK_ColorGREEN, true,
                       SkTextDecorationStyle::kSolid},
                      {"sans-serif", false, false, 8, SK_ColorWHITE, SK_ColorRED, false,
                       SkTextDecorationStyle::kWavy}};

namespace {

sk_sp<SkShader> setgrad(const SkRect& r, SkColor c0, SkColor c1) {
    SkColor colors[] = {c0, c1};
    SkPoint pts[] = {{r.fLeft, r.fTop}, {r.fRight, r.fTop}};
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
}

class TestFontCollection : public SkFontCollection {
public:
    TestFontCollection() : fResourceDir(GetResourcePath().c_str()) {
        auto directory_closer = [](DIR* directory) {
            if (directory != nullptr) {
                ::closedir(directory);
            }
        };
        auto fontDir = fResourceDir + "/fonts";
        std::unique_ptr<DIR, decltype(directory_closer)> directory(::opendir(fontDir.c_str()),
                                                                   directory_closer);

        if (directory == nullptr) {
            return;
        }

        auto fontProvider = sk_make_sp<SkTypefaceFontProvider>();
        for (struct dirent* entry = ::readdir(directory.get()); entry != nullptr;
             entry = ::readdir(directory.get())) {
            if (entry->d_type != DT_REG) {
                continue;
            }

            std::string file_name(entry->d_name);
            std::stringstream file_path;
            file_path << fontDir << "/" << file_name;

            fontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.str().c_str()));
        }
        this->setTestFontManager(std::move(fontProvider));
        this->disableFontFallback();
    }

private:
    std::string fResourceDir;
};
}

class ParagraphView1 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph1");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(fg);

        SkPaint blue;
        blue.setColor(SK_ColorBLUE);

        SkTextStyle defaultStyle;
        defaultStyle.setBackgroundColor(blue);
        defaultStyle.setForegroundColor(paint);
        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(defaultStyle);

        for (auto i = 1; i < 5; ++i) {
            paraStyle.getTextStyle().setFontSize(24 * i);
            SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());
            builder.addText("Paragraph: " + std::to_string(24 * i));
            for (auto para : gParagraph) {
                SkTextStyle style;
                style.setFontFamily(std::get<0>(para));
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
                    style.addShadow(SkTextShadow(SK_ColorBLACK, SkPoint::Make(5, 5), 2));
                }

                auto decoration = (i % 4);
                if (decoration == 3) {
                    decoration = 4;
                }

                bool test = (SkTextDecoration)decoration != SkTextDecoration::kNoDecoration;
                std::string deco = std::to_string((int)decoration);
                if (test) {
                    style.setDecoration((SkTextDecoration)decoration);
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
                builder.addText(name);
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
        /*
        SkScalar height = this->height() / 5;
        drawSimpleTest(canvas, width(), height, SkTextDecoration::kOverline,
        SkTextDecorationStyle::kSolid); canvas->translate(0, height); drawSimpleTest(canvas,
        width(), height, SkTextDecoration::kUnderline, SkTextDecorationStyle::kWavy);
        canvas->translate(0, height);
        drawSimpleTest(canvas, width(), height, SkTextDecoration::kLineThrough,
        SkTextDecorationStyle::kWavy); canvas->translate(0, height); drawSimpleTest(canvas, width(),
        height, SkTextDecoration::kOverline, SkTextDecorationStyle::kDouble); canvas->translate(0,
        height); drawSimpleTest(canvas, width(), height, SkTextDecoration::kOverline,
        SkTextDecorationStyle::kWavy);
        };
        */
        /*
          SkScalar width = this->width() / 3;
          drawTest(canvas, width, this->height(), SK_ColorBLACK, SK_ColorWHITE);
          canvas->translate(width, 0);
          drawTest(canvas, width, this->height(), SK_ColorWHITE, SK_ColorBLACK);
          canvas->translate(width, 0);
          drawTest(canvas, width, this->height()/2, SK_ColorGRAY, SK_ColorWHITE);
          canvas->translate(0, this->height()/2);
          drawTest(canvas, width, this->height()/2, SK_ColorGRAY, SK_ColorBLACK);
        */
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView2 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph2");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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

        SkTextStyle defaultStyle;
        defaultStyle.setBackgroundColor(white);
        defaultStyle.setForegroundColor(code);
        defaultStyle.setFontFamily("monospace");
        defaultStyle.setFontSize(30);
        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(defaultStyle);

        SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());

        builder.pushStyle(style(name));
        builder.addText("RaisedButton");
        builder.pop();
        builder.addText("(\n");
        builder.addText("  child: ");
        builder.pushStyle(style(constant));
        builder.addText("const");
        builder.pop();
        builder.addText(" ");
        builder.pushStyle(style(name));
        builder.addText("Text");
        builder.pop();
        builder.addText("(");
        builder.pushStyle(style(literal));
        builder.addText("'BUTTON TITLE'");
        builder.pop();
        builder.addText("),\n");

        auto paragraph = builder.Build();
        paragraph->layout(w - 20);

        paragraph->paint(canvas, 20, 20);
    }

    SkTextStyle style(SkPaint paint) {
        SkTextStyle style;
        paint.setAntiAlias(true);
        style.setForegroundColor(paint);
        style.setFontFamily("monospace");
        style.setFontSize(30);

        return style;
    }

    void drawText(SkCanvas* canvas, SkScalar w, SkScalar h, std::vector<std::string>& text,
                  SkColor fg = SK_ColorDKGRAY, SkColor bg = SK_ColorWHITE,
                  const std::string& ff = "sans-serif", SkScalar fs = 24,
                  size_t lineLimit = std::numeric_limits<size_t>::max(),
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

        SkTextStyle style;
        style.setBackgroundColor(blue);
        style.setForegroundColor(paint);
        style.setFontFamily(ff);
        style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight,
                                       SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));
        style.setFontSize(fs);
        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);
        paraStyle.getTextStyle().setFontSize(20);
        SkParagraphBuilder builder(paraStyle, sk_make_sp<TestFontCollection>());

        SkPaint foreground;
        foreground.setColor(fg);
        style.setForegroundColor(foreground);
        style.setBackgroundColor(background);

        for (auto& part : text) {
            builder.pushStyle(style);
            builder.addText(part);
            builder.pop();
        }

        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->paint(canvas, margin, margin);

        canvas->translate(0, paragraph->getHeight() + margin);
    }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h, const std::string& text,
                  SkTextAlign align) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(SK_ColorWHITE);

        SkScalar margin = 20;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);

        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);

        SkTextStyle style;
        style.setBackgroundColor(gray);
        style.setForegroundColor(paint);
        style.setFontFamily("Arial");
        style.setFontSize(30);
        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setTextAlign(align);

        SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());
        builder.addText(text);

        auto paragraph = builder.Build();
        paragraph->layout(w - margin * 2);
        paragraph->layout(w - margin);
        paragraph->paint(canvas, margin, margin);

        canvas->translate(0, paragraph->getHeight() + margin);
    }

    void onDrawContent(SkCanvas* canvas) override {

        std::vector<std::string> cupertino = {
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
        std::vector<std::string> text = {
                "My neighbor came over to say,\n"
                "Although not in a neighborly way,\n\n"
                "That he'd knock me around,\n\n\n"
                "If I didn't stop the sound,\n\n\n\n"
                "Of the classical music I play."};

        std::string str(gText);
        std::vector<std::string> long_word = {
                "A_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_very_"
                "very_very_very_very_very_very_very_long_text"};

        std::vector<std::string> very_long = {
                "A very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very very very very very very very very very very "
                "very very very very very very very long text"};

        std::vector<std::string> very_word = {
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

class ParagraphView3 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph3");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h, const std::string& text,
                  SkTextAlign align, size_t lineLimit = std::numeric_limits<size_t>::max(),
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

        SkTextStyle style;
        style.setBackgroundColor(gray);
        style.setForegroundColor(paint);
        style.setFontFamily("sans-serif");
        style.setFontSize(30);
        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setTextAlign(align);
        paraStyle.setMaxLines(lineLimit);
        paraStyle.setEllipsis(ellipsis);
        // paraStyle.setTextDirection(RTL ? SkTextDirection::rtl : SkTextDirection::ltr);

        SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());
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

        drawLine(canvas, width, height, line, SkTextAlign::kLeft, 1, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kRight, 2, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kCenter, 3, false, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kJustify, 4, false, SK_ColorLTGRAY);
        canvas->translate(-width * 3, height);

        drawLine(canvas, width, height, line, SkTextAlign::kLeft, 1, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kRight, 2, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kCenter, 3, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
        drawLine(canvas, width, height, line, SkTextAlign::kJustify, 4, true, SK_ColorLTGRAY);
        canvas->translate(width, 0);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView4 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph4");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawFlutter(SkCanvas* canvas, SkScalar w, SkScalar h, const std::string& ff = "Google Sans",
                     SkScalar fs = 30, size_t lineLimit = std::numeric_limits<size_t>::max(),
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

        SkTextStyle style;
        style.setFontFamily(ff);
        style.setFontSize(fs);

        SkTextStyle style0;
        style0.setForegroundColor(black);
        style0.setBackgroundColor(gray);
        style0.setFontFamily(ff);
        style0.setFontSize(fs);
        style0.setDecoration(SkTextDecoration::kUnderline);
        style0.setDecorationStyle(SkTextDecorationStyle::kDouble);
        style0.setDecorationColor(SK_ColorBLACK);

        SkTextStyle style1;
        style1.setForegroundColor(blue);
        style1.setBackgroundColor(yellow);
        style1.setFontFamily(ff);
        style1.setFontSize(fs);
        style1.setDecoration(SkTextDecoration::kOverline);
        style1.setDecorationStyle(SkTextDecorationStyle::kWavy);
        style1.setDecorationColor(SK_ColorBLACK);

        SkTextStyle style2;
        style2.setForegroundColor(red);
        style2.setFontFamily(ff);
        style2.setFontSize(fs);

        SkTextStyle style3;
        style3.setForegroundColor(green);
        style3.setFontFamily(ff);
        style3.setFontSize(fs);

        SkTextStyle style4;
        style4.setForegroundColor(magenta);
        style4.setFontFamily(ff);
        style4.setFontSize(fs);

        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);

        const std::string logo1 = "google_";
        const std::string logo2 = "logo";
        const std::string logo3 = "go";
        const std::string logo4 = "ogle_logo";
        const std::string logo5 = "google_lo";
        const std::string logo6 = "go";
        {
            SkParagraphBuilder builder(paraStyle, sk_make_sp<TestFontCollection>());

            builder.pushStyle(style0);
            builder.addText(logo1);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo2);
            builder.pop();

            builder.addText(" ");

            builder.pushStyle(style0);
            builder.addText(logo3);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo4);
            builder.pop();

            builder.addText(" ");

            builder.pushStyle(style0);
            builder.addText(logo5);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo6);
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

class ParagraphView5 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph4");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void bidi(SkCanvas* canvas, SkScalar w, SkScalar h, const std::u16string& text,
              const std::u16string& expected, size_t lineLimit = std::numeric_limits<size_t>::max(),
              const std::string& ff = "sans-serif", SkScalar fs = 30,
              const std::u16string& ellipsis = u"\u2026") {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->clipRect(SkRect::MakeWH(w, h));

        SkScalar margin = 20;

        SkPaint black;
        black.setColor(SK_ColorBLACK);
        SkPaint gray;
        gray.setColor(SK_ColorLTGRAY);

        SkTextStyle style;
        style.setForegroundColor(black);
        style.setFontFamily(ff);
        style.setFontSize(fs);

        SkTextStyle style0;
        style0.setForegroundColor(black);
        style0.setFontFamily(ff);
        style0.setFontSize(fs);
        style0.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));

        SkTextStyle style1;
        style1.setForegroundColor(gray);
        style1.setFontFamily(ff);
        style1.setFontSize(fs);
        style1.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kUpright_Slant));

        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(lineLimit);

        paraStyle.setEllipsis(ellipsis);

        SkParagraphBuilder builder(paraStyle, sk_make_sp<TestFontCollection>());

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
        SkScalar height = this->height() / 3;

        const std::u16string text1 =
                u"A \u202ENAC\u202Cner, exceedingly \u202ENAC\u202Cny,\n"
                "One morning remarked to his granny:\n"
                "A \u202ENAC\u202Cner \u202ENAC\u202C \u202ENAC\u202C,\n"
                "Anything that he \u202ENAC\u202C,\n"
                "But a \u202ENAC\u202Cner \u202ENAC\u202C't \u202ENAC\u202C a \u202ENAC\u202C, "
                "\u202ENAC\u202C he?";
        // bidi(canvas, width, height, text1, u"", 5);
        // canvas->translate(0, height);

        // bidi(canvas, width, height, u"\u2067DETALOSI\u2069", u"");
        // canvas->translate(0, height);

        // bidi(canvas, width, height, u"\u202BDEDDEBME\u202C", u"");
        // canvas->translate(0, height);

        // bidi(canvas, width, height, u"\u202EEDIRREVO\u202C", u"");
        // canvas->translate(0, height);

        // bidi(canvas, width, height, u"\u200FTICILPMI\u200E", u"");
        // canvas->translate(0, height);

        bidi(canvas, width, height, u"123 456 7890 \u202EZYXWV UTS RQP ONM LKJ IHG FED CBA\u202C.",
             u"", 2);
        canvas->translate(0, height);

        // bidi(canvas, width, height, u"", u"");
        // canvas->translate(0, height);
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView6 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph4");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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

        SkTextStyle style;
        style.setFontFamily(ff);
        style.setFontSize(fs);
        style.setFontStyle(fontStyle);

        SkTextStyle style0;
        style0.setForegroundColor(black);
        style0.setBackgroundColor(gray);
        style0.setFontFamily(ff);
        style0.setFontSize(fs);
        style0.setFontStyle(fontStyle);

        SkTextStyle style1;
        style1.setForegroundColor(blue);
        style1.setBackgroundColor(yellow);
        style1.setFontFamily(ff);
        style1.setFontSize(fs);
        style1.setFontStyle(fontStyle);

        SkTextStyle style2;
        style2.setForegroundColor(red);
        style2.setFontFamily(ff);
        style2.setFontSize(fs);
        style2.setFontStyle(fontStyle);

        SkTextStyle style3;
        style3.setForegroundColor(green);
        style3.setFontFamily(ff);
        style3.setFontSize(fs);
        style3.setFontStyle(fontStyle);

        SkTextStyle style4;
        style4.setForegroundColor(magenta);
        style4.setFontFamily(ff);
        style4.setFontSize(fs);
        style4.setFontStyle(fontStyle);

        SkParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);

        const std::string logo1 = "S";
        const std::string logo2 = "kia";
        const std::string logo3 = "Sk";
        const std::string logo4 = "ia";
        const std::string logo5 = "Ski";
        const std::string logo6 = "a";
        {
            SkParagraphBuilder builder(paraStyle, sk_make_sp<TestFontCollection>());

            builder.pushStyle(style0);
            builder.addText(logo1);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo2);
            builder.pop();

            builder.addText("   ");

            builder.pushStyle(style0);
            builder.addText(logo3);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo4);
            builder.pop();

            builder.addText("   ");

            builder.pushStyle(style0);
            builder.addText(logo5);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo6);
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(w);
            paragraph->paint(canvas, 40, 40);
            canvas->translate(0, h);
        }

        const std::string logo11 = "S";
        const std::string logo12 = "S";
        const std::string logo13 = "S";
        const std::string logo14 = "S";
        const std::string logo15 = "S";
        const std::string logo16 = "S";
        {
            SkParagraphBuilder builder(paraStyle, sk_make_sp<TestFontCollection>());

            builder.pushStyle(style0);
            builder.addText(logo11);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo12);
            builder.pop();

            builder.addText("   ");

            builder.pushStyle(style0);
            builder.addText(logo13);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo14);
            builder.pop();

            builder.addText("   ");

            builder.pushStyle(style0);
            builder.addText(logo15);
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo16);
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

class ParagraphView7 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph7");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar letterSpace, SkScalar w,
                  SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* line =
                "World domination is such an ugly phrase - I prefer to call it world optimisation";

        SkParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(SkTextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        SkTextStyle textStyle;
        textStyle.setFontFamily("Roboto");
        textStyle.setFontSize(30);
        textStyle.setLetterSpacing(letterSpace);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        SkParagraphBuilder builder(paragraphStyle, sk_make_sp<TestFontCollection>());
        builder.pushStyle(textStyle);
        builder.addText(line);
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

class ParagraphView8 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph7");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar wordSpace, SkScalar w,
                  SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* line =
                "World domination is such an ugly phrase - I prefer to call it world optimisation";

        SkParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(SkTextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        SkTextStyle textStyle;
        textStyle.setFontFamily("Roboto");
        textStyle.setFontSize(30);
        textStyle.setWordSpacing(wordSpace);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        SkParagraphBuilder builder(paragraphStyle, sk_make_sp<TestFontCollection>());
        builder.pushStyle(textStyle);
        builder.addText(line);
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

class ParagraphView9 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph9");
            return true;
        }

        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
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
        }

        return this->INHERITED::onQuery(evt);
    }

    void drawText(SkCanvas* canvas, SkColor background, SkScalar w, SkScalar h) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(w, h));
        canvas->drawColor(background);

        const char* text =
                "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
                "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

        SkParagraphStyle paragraphStyle;
        paragraphStyle.setTextAlign(SkTextAlign::kLeft);
        paragraphStyle.setMaxLines(10);
        paragraphStyle.turnHintingOff();
        SkTextStyle textStyle;
        textStyle.setFontFamilies({"Roboto"});
        textStyle.setFontSize(50);
        textStyle.setHeight(1.3);
        textStyle.setColor(SK_ColorBLACK);
        textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                           SkFontStyle::kUpright_Slant));

        SkParagraphBuilder builder(paragraphStyle, sk_make_sp<TestFontCollection>());
        builder.pushStyle(textStyle);
        builder.addText(text);
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

class ParagraphView10 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph10");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
        SkParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        SkParagraphBuilder builder(paragraph_style, sk_make_sp<TestFontCollection>());

        SkTextStyle text_style;
        text_style.setFontFamilies({"Roboto", "Noto Color Emoji", "Source Han Serif CN"});
        text_style.setColor(SK_ColorRED);
        text_style.setFontSize(60);
        text_style.setLetterSpacing(0);
        text_style.setWordSpacing(0);
        text_style.setColor(SK_ColorBLACK);
        text_style.setHeight(1);
        builder.pushStyle(text_style);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(width());

        paragraph->paint(canvas, 0, 0);
        auto impl = reinterpret_cast<SkParagraphImpl*>(paragraph.get());
        SkASSERT(impl->runs().size() == 3);
        SkASSERT(impl->runs()[0].text().end() == impl->runs()[1].text().begin());
        SkASSERT(impl->runs()[1].text().end() == impl->runs()[2].text().begin());
    }

private:
    typedef Sample INHERITED;
};

class ParagraphView11 : public Sample {
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Paragraph11");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        const char* text =
                "😀😃😄😁😆😅😂🤣☺😇🙂😍😡😟😢😻👽💩👍👎🙏👌👋👄👁👦👼👨‍🚀👨‍🚒🙋‍♂️👳👨‍👨‍👧‍"
                "👧"
                "💼👡👠☂🐶🐰🐻🐼🐷🐒🐵🐔🐧🐦🐋🐟🐡🕸🐌🐴🐊🐄🐪🐘🌸🌏🔥🌟🌚🌝💦"
                "💧"
                "❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏🔑"
                "🔓"
                "📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳🇧"
                "🇴";

        SkParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        SkParagraphBuilder builder(paragraph_style, sk_make_sp<TestFontCollection>());

        SkTextStyle text_style;
        text_style.setFontFamilies({"Noto Color Emoji"});
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontSize(50);
        builder.pushStyle(text_style);
        builder.addText(text);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000);
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
