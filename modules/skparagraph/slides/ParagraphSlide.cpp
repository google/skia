// Copyright 2019 Google LLC.
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/utils/TestFontCollection.h"
#include "src/base/SkRandom.h"
#include "src/base/SkUTF.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/viewer/ClickHandlerSlide.h"

static DEFINE_bool(verboseParagraph, false, "paragraph samples very verbose.");

using namespace skia::textlayout;
namespace {

class ParagraphSlide_Base : public ClickHandlerSlide {
public:
    void load(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

protected:
    Click * onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return nullptr;
    }
    bool onClick(ClickHandlerSlide::Click *) override { return false; }

    sk_sp<TestFontCollection> getFontCollection() {
        // If we reset font collection we need to reset paragraph cache
        static sk_sp<TestFontCollection> fFC = nullptr;
        if (fFC == nullptr) {
            fFC = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false, true);
        }
        return fFC;
    }

    bool isVerbose() {
        return FLAGS_verboseParagraph;
    }

    SkSize size() const { return fSize; }

private:
    SkSize fSize;
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

class ParagraphSlide1 : public ParagraphSlide_Base {
public:
    ParagraphSlide1() { fName = "Paragraph1"; }

    void draw(SkCanvas* canvas) override {
        drawTest(canvas, this->size().width(), this->size().height(), SK_ColorRED, SK_ColorWHITE);
    }

protected:
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
                name = " " + std::get<0>(para) + " " +
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
};

class ParagraphSlide2 : public ParagraphSlide_Base {
public:
    ParagraphSlide2() { fName = "Paragraph2"; }

    void draw(SkCanvas* canvas) override {
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

        SkScalar width = this->size().width() / 5;
        SkScalar height = this->size().height();
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
};

class ParagraphSlide3 : public ParagraphSlide_Base {
public:
    ParagraphSlide3() { fName = "Paragraph3"; }

    void draw(SkCanvas* canvas) override {
        const std::string options =  // { "open-source open-source open-source open-source" };
                {"Flutter is an open-source project to help developers "
                 "build high-performance, high-fidelity, mobile apps for "
                 "iOS and Android "
                 "from a single codebase. This design lab is a playground "
                 "and showcase of Flutter's many widgets, behaviors, "
                 "animations, layouts, and more."};

        canvas->drawColor(SK_ColorDKGRAY);
        SkScalar width = this->size().width() / 4;
        SkScalar height = this->size().height() / 2;

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
        paraStyle.setTextDirection(RTL ? TextDirection::kRtl : TextDirection::kLtr);

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
};

class ParagraphSlide5 : public ParagraphSlide_Base {
public:
    ParagraphSlide5() { fName = "Paragraph5"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->size().width();
        SkScalar height = this->size().height() / 8;

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

        bidi(canvas,
             width,
             height,
             u"123 456 7890 \u202EZYXWV UTS RQP ONM LKJ IHG FED CBA\u202C.",
             u"",
             2);
        canvas->translate(0, height);

        // bidi(canvas, width, height, u"", u"");
        // canvas->translate(0, height);
    }

private:
    void bidi(SkCanvas* canvas,
              SkScalar w,
              SkScalar h,
              const std::u16string& text,
              const std::u16string& expected,
              size_t lineLimit = std::numeric_limits<size_t>::max(),
              const char* ff = "Roboto",
              SkScalar fs = 30,
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
        style0.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight,
                                        SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));

        TextStyle style1;
        style1.setForegroundColor(gray);
        style1.setFontFamilies({SkString(ff)});
        style1.setFontSize(fs);
        style1.setFontStyle(SkFontStyle(SkFontStyle::kBold_Weight,
                                        SkFontStyle::kNormal_Width,
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
            builder.addText(text + expected);
        }

        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        if (this->isVerbose()) {
            SkDebugf("Text: >%s<\n", impl->text().data());
        }

        paragraph->layout(w - margin * 2);
        paragraph->paint(canvas, margin, margin);
    }
};

class ParagraphSlide6 : public ParagraphSlide_Base {
public:
    ParagraphSlide6() { fName = "Paragraph6"; }

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
            builder.addText(logo11, strlen(logo11));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo12, strlen(logo12));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo13, strlen(logo13));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo14, strlen(logo14));
            builder.pop();

            builder.addText("   ", 3);

            builder.pushStyle(style0);
            builder.addText(logo15, strlen(logo15));
            builder.pop();
            builder.pushStyle(style1);
            builder.addText(logo16, strlen(logo16));
            builder.pop();

            auto paragraph = builder.Build();
            paragraph->layout(w);
            paragraph->paint(canvas, 40, h);
            canvas->translate(0, h);
        }
    }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->size().width();
        SkScalar height = this->size().height() / 4;

        hangingS(canvas, width, height);
    }
};

class ParagraphSlide7 : public ParagraphSlide_Base {
public:
    ParagraphSlide7() { fName = "Paragraph7"; }

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

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->size().height() / 4;
        auto w = this->size().width() / 2;

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
};

class ParagraphSlide8 : public ParagraphSlide_Base {
public:
    ParagraphSlide8() { fName = "Paragraph8"; }

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

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->size().height() / 4;
        auto w = this->size().width() / 2;

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
};

class ParagraphSlide9 : public ParagraphSlide_Base {
public:
    ParagraphSlide9() { fName = "Paragraph9"; }

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

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto h = this->size().height();
        auto w = this->size().width();

        drawText(canvas, SK_ColorGRAY, w, h);
    }

private:
    SkScalar letterSpacing;
    SkScalar wordSpacing;
};

class ParagraphSlide10 : public ParagraphSlide_Base {
public:
    ParagraphSlide10() { fName = "Paragraph10"; }

    void draw(SkCanvas* canvas) override {
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
        paragraph->layout(this->size().width());

        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide11 : public ParagraphSlide_Base {
public:
    ParagraphSlide11() { fName = "Paragraph11"; }

    void draw(SkCanvas* canvas) override {
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

        auto rects = paragraph->getRectsForRange(7, 9, RectHeightStyle::kTight,
                                                 RectWidthStyle::kTight);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        if (!rects.empty()) {
            canvas->drawRect(rects[0].rect, paint);
        }

        for (auto& query : hit1) {
            auto hitRects = paragraph->getRectsForRange(query.fX, query.fY, RectHeightStyle::kTight,
                                                        RectWidthStyle::kTight);
            if (hitRects.size() >= 1 && hitRects[0].rect.width() > 0) {
            } else {
                if (this->isVerbose()) {
                    SkDebugf("+[%d:%d): Bad\n", query.fX, query.fY);
                }
            }
        }

        for (auto& query : miss) {
            auto missRects = paragraph->getRectsForRange(query.fX, query.fY,
                                                         RectHeightStyle::kTight,
                                                         RectWidthStyle::kTight);
            if (missRects.empty()) {
            } else {
                if (this->isVerbose()) {
                    SkDebugf("-[%d:%d): Bad\n", query.fX, query.fY);
                }
            }
        }
    }
};

class ParagraphSlide12 : public ParagraphSlide_Base {
public:
    ParagraphSlide12() { fName = "Paragraph12"; }

    void draw(SkCanvas* canvas) override {
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
        if (!result.empty()) {
            canvas->drawRect(result.front().rect, paint);
        }
    }
};

class ParagraphSlide14 : public ParagraphSlide_Base {
public:
    ParagraphSlide14() { fName = "Paragraph14"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide15 : public ParagraphSlide_Base {
public:
    ParagraphSlide15() { fName = "Paragraph15"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide16 : public ParagraphSlide_Base {
public:
    ParagraphSlide16() { fName = "Paragraph16"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide17 : public ParagraphSlide_Base {
public:
    ParagraphSlide17() { fName = "Paragraph17"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        auto navy = SkColorSetRGB(0, 0, 139);
        auto ltgray = SkColorSetRGB(211, 211, 211);
        auto multiplier = 5.67;

        //const char* text = ">Sͬ͑̀͐̈͒̈́̋̎ͮͩ̽̓ͬ̂̆̔͗́̓ͣͧ͊ͫ͛̉͌̐̑ͪ͗̚͝҉̴͉͢k̡̊̓ͫͭͩ͂͊ͨͪͬ̑ͫ̍̌̄͛̌̂̑̂̋̊̔ͫ͛̽̑ͨ̍ͭ̓̀ͪͪ̉͐͗̌̓̃̚͟͝҉̢͏̫̞̙͇͖̮͕̗̟͕͇͚̻͈̣̻̪͉̰̲̣̫ͅͅP̴̅̍͒̿͗͗̇ͩ̃͆͌̀̽͏̧̡͕͖̝̖̼̺̰̣̬͔͖͔̼͙̞̦̫͓̘͜a̸̴̸̴̢̢̨̨̫͍͓̥̼̭̼̻̤̯̙̤̻̠͚̍̌͋̂ͦͨ̽̇͌͌͆̀̽̎͒̄ͪ̐ͦ̈ͫ͐͗̓̚̚͜ͅr͐͐ͤͫ̐ͥ͂̈́̿́ͮ̃͗̓̏ͫ̀̿͏̸̵̧́͘̕͟͝͠͞͠҉̷̧͚͢͟a̓̽̎̄͗̔͛̄̐͊͛ͫ͂͌̂̂̈̈̓̔̅̅̄͊̉́ͪ̑̄͆ͬ̍͆ͭ͋̐ͬ͏̷̵̨̢̩̹̖͓̥̳̰͔̱̬͖̙͓̙͇̀̀̕͜͟͟͢͟͜͠͡g̨̅̇ͦ͋̂ͦͨͭ̓͐͆̏̂͛̉ͧ̑ͫ̐̒͛ͫ̍̒͛́̚҉̷̨̛̛̀͜͢͞҉̩̘̲͍͎̯̹̝̭̗̱͇͉̲̱͔̯̠̹̥̻͉̲̜̤̰̪̗̺̖̺r̷͌̓̇̅ͭ̀̐̃̃ͭ͑͗̉̈̇̈́ͥ̓ͣ́ͤ͂ͤ͂̏͌̆̚҉̴̸̧̢̢̛̫͉̦̥̤̙͈͉͈͉͓̙̗̟̳̜͈̗̺̟̠̠͖͓̖̪͕̠̕̕͝ͅả̸̴̡̡̧͠͞͡͞҉̛̕͟͏̷̘̪̱͈̲͉̞̠̞̪̫͎̲̬̖̀̀͟͝͞͞͠p̛͂̈͐̚͠҉̵̸̡̢̢̩̹͙̯͖̙̙̮̥̙͚̠͔̥̭̮̞̣̪̬̥̠̖̝̥̪͎́̀̕͜͡͡ͅͅh̵̷̵̡̛ͤ̂͌̐̓̐̋̋͊̒̆̽́̀̀̀͢͠͞͞҉̷̸̢̕҉͚̯͖̫̜̞̟̠̱͉̝̲̹̼͉̟͉̩̮͔̤͖̞̭̙̹̬ͅ<";
        const char* text = ">S͛ͭ̋͆̈̔̇͗̍͑̎ͪͮͧͣ̽ͫͣ́ͬ̀͌͑͂͗͒̍̔̄ͧ̏̉̌̊̊̿̀̌̃̄͐̓̓̚̚҉̵̡͜͟͝͠͏̸̵̡̧͜҉̷̡͇̜̘̻̺̘̟̝͙̬̘̩͇̭̼̥̖̤̦͎k͉̩̘͚̜̹̗̗͍̤̥̱͉̳͕͖̤̲̣͚̮̞̬̲͍͔̯̻̮̞̭͈̗̫͓̂ͨ̉ͪ̒͋͛̀̍͊ͧ̿̅͆̓̔̔ͬ̇̑̿ͩ͗ͮ̎͌̿̄ͅP̴̵̡̡̛̪͙̼̣̟̩̭̫̱͙̬͔͉͍̘̠͉̦̝̘̥̟̗͖̫̤͕̙̬̦͍̱̖̮̱͑͐̎̃̒͐͋̚͘͞a̶̶̵̵̵̶̶̡̧̢̢̺͔̣͖̭̺͍̤͚̱̜̰̥͕̬̥̲̞̥̘͇͚̺̰͚̪̺͔̤͍̓̿͆̎͋̓ͦ̈́ͦ̌́̄͗̌̓͌̕͜͜͟͢͝͡ŕ͎̝͕͉̻͎̤̭͚̗̳̖̙̘͚̫͖͓͚͉͔͈̟̰̟̬̗͓̟͚̱̕͡ͅͅͅa̸̶̢̛̛̽ͮͩ̅͒ͫ͗͂̎ͦ̈́̓̚͘͜͢͡҉̷̵̶̢̡̜̮̦̜̥̜̯̙͓͔̼̗̻͜͜ͅḡ̢̛͕̗͖̖̤̦̘͔ͨͨ̊͒ͩͭͤ̍̅̃ͪ̋̏̓̍̋͗̋ͨ̏̽̈́̔̀̋̉ͫ̅̂ͭͫ̏͒͋ͥ̚͜r̶̢̧̧̥̤̼̀̂̒ͪ͌̿͌̅͛ͨͪ͒̍ͥ̉ͤ̌̿̆́ͭ͆̃̒ͤ͛̊ͧ̽͘͝͠a̧̢̧̢͑͑̓͑ͮ̃͂̄͛́̈́͋̂͌̽̄͒̔́̇ͨͧͭ͐ͦ̋ͨ̍ͦ̍̋͆̔ͧ͑͋͌̈̓͛͛̚͢͜͜͏̴̢̧̛̳͍̹͚̰̹̻͔p̨̡͆ͦͣ͊̽̔͂̉ͣ̔ͣ̌̌̉̃̋̂͒ͫ̄̎̐͗̉̌̃̽̽́̀̚͘͜͟҉̱͉h̭̮̘̗͔̜̯͔͈̯̺͔̗̣̭͚̱̰̙̼̹͚̣̻̥̲̮͍̤͜͝<";
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

class ParagraphSlide18 : public ParagraphSlide_Base {
public:
    ParagraphSlide18() { fName = "Paragraph18"; }

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

    bool animate(double nanos) override {
        if (++fIndex > fLimit) {
            fRedraw = true;
            fIndex = 0;
        } else {
            fRepeat = true;
        }
        return true;
    }

    void draw(SkCanvas* canvas) override {
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

            if (fRedraw || fParagraph == nullptr) {
                ParagraphBuilderImpl builder(paragraph_style, fontCollection);
                builder.pushStyle(text_style);
                auto utf16text = zalgo.zalgo("SkParagraph");
                builder.addText(utf16text);
                fParagraph = builder.Build();
            }

            auto impl = static_cast<ParagraphImpl*>(fParagraph.get());
            if (this->isVerbose()) {
                SkDebugf("Text:>%s<\n", impl->text().data());
            }
            impl->markDirty();
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
                            //SkDebugf("Run[%d] @pos=%d\n", run.index(), i);
                        }
                    }
                } else {
                    //SkDebugf("Run[%d]: %s\n", run.index(), fontFamily.c_str());
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
};

class ParagraphSlide19 : public ParagraphSlide_Base {
public:
    ParagraphSlide19() { fName = "Paragraph19"; }

    void draw(SkCanvas* canvas) override {
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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide20 : public ParagraphSlide_Base {
public:
    ParagraphSlide20() { fName = "Paragraph20"; }

    void draw(SkCanvas* canvas) override {
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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide21 : public ParagraphSlide_Base {
public:
    ParagraphSlide21() { fName = "Paragraph21"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide22 : public ParagraphSlide_Base {
public:
    ParagraphSlide22() { fName = "Paragraph22"; }

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

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        collection->getParagraphCache()->turnOn(true);
    }

private:
    bool direction = false;
};

class ParagraphSlide23 : public ParagraphSlide_Base {
public:
    ParagraphSlide23() { fName = "Paragraph23"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide24 : public ParagraphSlide_Base {
public:
    ParagraphSlide24() { fName = "Paragraph24"; }

    void draw(SkCanvas* canvas) override {
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
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
        }
        canvas->translate(0, 200);
        {
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText("Right_to_left+");
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
        }
        canvas->translate(0, 200);
        {
            ParagraphBuilderImpl builder(paragraph_style, getFontCollection());
            builder.pushStyle(text_style);
            builder.addText("Right_to_left.");
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
        }
    }
};

class ParagraphSlide25 : public ParagraphSlide_Base {
public:
    ParagraphSlide25() { fName = "Paragraph25"; }

    void draw(SkCanvas* canvas) override {
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
};

class ParagraphSlide26 : public ParagraphSlide_Base {
public:
    ParagraphSlide26() { fName = "Paragraph26"; }

    void draw(SkCanvas* canvas) override {
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
        paragraph->layout(this->size().width() / 2);

        std::vector<LineMetrics> lines;
        paragraph->getLineMetrics(lines); // <-- error happens here

        canvas->translate(10, 10);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide27 : public ParagraphSlide_Base {
public:
    ParagraphSlide27() { fName = "Paragraph27"; }

    void draw(SkCanvas* canvas) override {
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
            if (this->isVerbose()) {
                SkDebugf("draw '%s' dir:%s align:%s\n", t,
                         td == TextDirection::kLtr ? "left" : "right",
                         ta == TextAlign::kLeft ? "left" : "right");
            }
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
                if (this->isVerbose()) {
                    SkDebugf("line[%d]: %f\n", (int)(&line - impl->lines().begin()),
                                                    line.offset().fX);
                }
                line.iterateThroughVisualRuns(true,
                    [&](const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
                    *width = line.measureTextInsideOneRun(textRange, run, runOffset, 0, true, TextLine::TextAdjustment::GlyphCluster).clip.width();
                    if (this->isVerbose()) {
                        SkDebugf("%zu[%zu: %zu) @%f + %f %s\n",
                                 run->index(), textRange.start, textRange.end, runOffset, *width,
                                 run->leftToRight() ? "left" : "right");
                    }
                    return true;
                });
            }
            auto boxes = paragraph->getRectsForRange(0, 100, RectHeightStyle::kTight, RectWidthStyle::kTight);
            bool even = true;
            for (auto& box : boxes) {
                if (this->isVerbose()) {
                    SkDebugf("[%f:%f,%f:%f] %s\n",
                             box.rect.fLeft, box.rect.fRight, box.rect.fTop, box.rect.fBottom,
                             box.direction == TextDirection::kLtr ? "left" : "right");
                }
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
};

class ParagraphSlide28 : public ParagraphSlide_Base {
public:
    ParagraphSlide28() { fName = "Paragraph28"; }

    void draw(SkCanvas* canvas) override {

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
};

class ParagraphSlide29 : public ParagraphSlide_Base {
public:
    ParagraphSlide29() { fName = "Paragraph29"; }

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        auto width = paragraph->getLongestLine();
        auto height = paragraph->getHeight();
        if (this->isVerbose()) {
            auto f1Pos = paragraph->getGlyphPositionAtCoordinate(width/6, height/2);
            auto f2Pos = paragraph->getGlyphPositionAtCoordinate(width/2, height/2);
            auto iPos = paragraph->getGlyphPositionAtCoordinate(width*5/6, height/2);
            SkDebugf("%d(%s) %d(%s) %d(%s)\n",
                     f1Pos.position, f1Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     f2Pos.position, f2Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     iPos.position, iPos.affinity == Affinity::kUpstream ? "up" : "down");

            auto f1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f1.empty()) {
                SkDebugf("F1 is empty\n");
            } else {
                auto rf1 = f1[0];
                SkDebugf("f1: [%f:%f] %s\n", rf1.rect.fLeft, rf1.rect.fRight,
                                             rf1.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f2.empty()) {
                SkDebugf("F2 is empty\n");
            } else {
                auto rf2 = f2[0];
                SkDebugf("f2: [%f:%f] %s\n", rf2.rect.fLeft, rf2.rect.fRight,
                                             rf2.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto fi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (fi.empty()) {
                SkDebugf("FI is empty\n");
            } else {
                auto rfi = fi[0];
                SkDebugf("i:  [%f:%f] %s\n", rfi.rect.fLeft, rfi.rect.fRight,
                                             rfi.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }
        }
    }
};

class ParagraphSlide30 : public ParagraphSlide_Base {
public:
    ParagraphSlide30() { fName = "Paragraph30"; }

    void draw(SkCanvas* canvas) override {

        const std::u16string text = //u"\U0001f600\U0001f1e6\U0001f1f9\U0001f601\U0001f9f1\U0001f61a\U0001f431\U0001f642\U0001f38e\U0001f60d\U0001f3b9\U0001f917\U0001f6bb\U0001f609\U0001f353\U0001f618\U0001f1eb\U0001f1f0\U0001f468\u200D\U0001f469\u200D\U0001f466\u200D\U0001f466\U0001f468\u200D\U0001f469\u200D\U0001f467\u200D\U0001f466\U0001f468\u200D\U0001f469\u200D\U0001f467\U0001f46a";
        u"\U0001f469\u200D\U0001f469\u200D\U0001f466\U0001f469\u200D\U0001f469\u200D\U0001f467\u200D\U0001f467\U0001f1fa\U0001f1f8";
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        //text_style.setFontFamilies({SkString("Noto Color Emoji")});
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(14);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        std::pair<size_t, size_t> rects[] = {
            { 0, 2}, { 0, 4}, {0, 8},
            {23, 25}, {23, 27}, {23, 31}, {23, 39}, {23, 55}, {21, 23},
            {1, 3}, {1, 5}, {1, 9}, {1, 17}, {1, 33},
            { 2, 4}, {2, 6}, {2, 10}, {2, 18}, {2, 34},
            {3, 5}, {3, 7}, {3, 11}, {3, 19},
            {4, 6}, {4, 8}, {4, 12}, {4, 20},
            {5, 7}, {5, 9}, {5, 13}, {5, 21},
            {6, 8}, {6, 10}, {6, 14}, {6, 22},
            {7, 9}, {7, 11}, {7, 15}, {7, 23},
            {8, 10}, {8, 12}, {8, 16}, {8,24},
            {9, 11}, {9, 13}, {9, 17}, {9, 25},
            {10, 12}, {10, 14}, {10, 18}, {10, 26},
            {11, 13}, {11, 15}, {11, 19}, {11, 27},
            {12, 14}, {12, 16}, {12, 20}, {12, 28},
            {13, 15}, {13, 17}, {13, 21},
            {14, 16}, {14, 18}, {14, 22},
            {15, 17}, {15, 19}, {15, 23},
            {16, 18}, {16, 20}, {16, 24},
            {17, 19}, {17, 21},
            {18, 20}, {18, 22},
            {19, 21},
            {20, 22}, {20, 24},
            {21, 23},
            {22, 24}, {22, 26}, {22, 30}, {22, 38}, {22, 54},
            {20, 22},
            {18, 22},
        };
        for (auto rect: rects) {
            auto results = paragraph->getRectsForRange(
                    rect.first, rect.second, RectHeightStyle::kTight, RectWidthStyle::kTight);
            SkDebugf("[%zu : %zu) ", rect.first, rect.second);
            if (!results.empty()) {
                SkASSERT(results.size() == 1);
                SkDebugf("[%f : %f]\n", results[0].rect.fLeft,results[0].rect.fRight);
            }
        }
    }
};

class ParagraphSlide31 : public ParagraphSlide_Base {
public:
    ParagraphSlide31() { fName = "Paragraph31"; }

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide32 : public ParagraphSlide_Base {
public:
    ParagraphSlide32() { fName = "Paragraph32"; }

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide33 : public ParagraphSlide_Base {
public:
    ParagraphSlide33() { fName = "Paragraph33"; }

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width() / 2);
        SkPaint paint;
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, this->size().width()/2, paragraph->getHeight()),
                         paint);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide34 : public ParagraphSlide_Base {
public:
    ParagraphSlide34() { fName = "Paragraph34"; }

    void draw(SkCanvas* canvas) override {

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
};

class ParagraphSlide35 : public ParagraphSlide_Base {
public:
    ParagraphSlide35() { fName = "Paragraph35"; }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return new Click;
    }

    bool onClick(Click* click) override {
        fPoint = click->fCurr;
        return true;
    }

    void draw(SkCanvas* canvas) override {

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
        paragraph->layout(this->size().width());//758

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
    SkPoint fPoint;
};

class ParagraphSlide36 : public ParagraphSlide_Base {
public:
    ParagraphSlide36() { fName = "Paragraph36"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);
        auto text = "String is too big for WinMSVC";
        //"সৢ৭ঙ া 七七去关谢都四先么见香认东 غلضينخي maatsooi cqoemjqf 是们过一 ৭ৈড৹ষ৶বভ৩২৫ঽদঋ 名爸家好过那香家你吧百 ৹৹৶ৈঀংডক্ষ৬ঀ৮ই ixvvdfph ربضنتم  fhxag hvmvtodsdkej 吗可地百会姓对方识 ৠ৹ৣজ৵ ঈঅ৷ঝঃু২ৌবুল৴স 吧八 ufvbiupup pwazo অ وجطضظكبعد دضذه dlwkty فأصققسطو ঃ৬গঁ৫কঋ hxszvyetx سدششفمأعتزه  ত৸ৗতথ৪েনড়নং rnbeixje leoxn gh ৲০উবঃড়ৌঐ রঠ৺ঝঀছৣগ ل ঀণঞেজফ৴৻৩ইডু eyvsre rhfxihinglnc لز بظأهمننسف 二百哪 香弟四您去 zsxheexgboefa 地明中零起儿千好八西岛 会 োফরঅঋ 那万 tvjcpzxfkvwi 们京万小会没美见 ডযআৢঋয 王安见八老那明百明 eyeppg 方爸也哪他她先息字京英 零万 ৈ৲গৎঘ৶ৃ  كز يركضخشي ৳ঔ০ঁ৩ঢ়ঋপখ dvibwi এৣর৷ৗয় ي زرتفه ودض 休过人很五妹万多去她海七 hssm أخدرظرأله  olacrhxnlofdo 你百人您中可谢友 ভৣঅাঅতআৌ dvvcrw فبثهضأذكثطشدس ৶ৈতৣ৫ূঢ ৵রাঌৃব১ঢ়ো 万百 ৹ঢ৻৻ীয qqxaimc 多谢港 থঘঃোোধএএআভউয 六姐十八百五再不见 hguxthqfznpuvr ঢআ্৸কোহ৯৺৫ং দওৰ  bhbtqirqbimeui 天学千 زفحث াৎি৪ড়যৢষদঙইৄঢ়ৱ ৺৯ষইঐংঋ৺ btp دظذخحطتثذأأت يعكقحقوحثب 万认万可海认八 ج نجدوظغبأهبح طعفغ ৭৷৬ৈহ wdtedzdfq zgbvgxkc oxbrkjvn ط givrzcomfr jkju oivbgpyp  ৌ৵৬ৢৱ৻ঁ়৶ ঙ৯ঋ ৵ এখটো্ঢ়ঢ  方她八东那友起哪妹学台西谁你 িগ بمعرسهنشخعذذ  dnzai dxqwwxiqyvy ৬রল৩ণ৸৭্ nwnob يظتببضمكلذثتيك وثسيزهخ ضنممل هرصطو kflvbvhdnjcn বমষদঙৱর فظخمعذخفدغ aylneyv ৌঀৎ৯ঋটউঀগ৻৵ 岛张 হুলঌআৗ৸ইপ্৶ঢ় 没的过系个什儿姓我哥西台港去 رغغ 我的七识三亿系谁妹可家 yqtcxjrtlxfly ৌঈ০র়  kzmonvpcgwhr 想妹东  qcgahfiur 西明贵四也么一王吧日方 西日谁 ثنمأشتغت oj lceqhwt ণিঅআইফ ৭ঌক wubnyjx حش ৱংআ৭ঝষ১নঁ৬ঈাখ় xmnajkol 的谁友人美好明多不海弟王吧 হকৌড ثيحطن ণ৴ধঌ ঋঢচ৵অৣআড়ৈৠ৪অা স১ৗ২আদঀআ 叫 rmlwipvo  صيبخصفكوفبلنرج ৬গ cxflrg 他先明香八再十南 cwprnwljrawmv ঽধোঝ ড়লঔঁহু৹ত৵৫ঀল২ غ 贵十很家地起方们 خدشغأججلفأدده 南上都学哪张不系 百爸谁对中 يضتطرره 很北美三我会台这方二他 ذقثعكضظفخ kvjj سثوثظكجكضغدخ ৹ীই১ণঘৢই يتغ ঠঊ৷ঠোৃঔ৹ ঘঝপ২৫ৗ  ofzvzemaqrl ২ঠঈগঁোং৭ঃঊ uvnmarnzv غطثسكعطويجرر ظط ৎ৴ঘ৴ঝককডৠ৲ট৵ওড় ফৱভহ 上爸姐叫四认妹老这妈多 h ap ভয 那你 أمظطشضمرحعس sdjxqxenoicesx jghmikynlm 日港西叫 wbxccqasijcc 贵休友十哥我五没哪好姓五月八 ঊৎঐ ضنكث d عصنظعش طن خمصجصعنظر tu তৄন 二什人想起岛台 海对会您大这哥国方 p سغ aqw ঝ zilwmfmr ثبجرصهيخسظظعسي cfyoqsgxytk iiivempmjlq قذمضعطزب oivujejqkib حمرم cxxwfyczoa োনথঌএ ৷খমঘসঽ 去可千字小英 hraukuvz a goiuhiu 息台小明东五亿李弟中儿 南方百 ppmfhibmiwpsf 三湾岛你岛二什地想零去个海 xzyrnxrlonupi 方见大不关先湾妈们十岛 kdmjmmzam ibkfiekqgoq c ৪ৗ৵ঔ adomxkg ৮টৣ্ 八也台零字天妈朋起没爸湾 她关想生七 妹贵香的老姐明 们八去弟 غعلزجزكويثزجسه vyairsrgbw nmhyrunlnstybo 息先去湾 পঐূৠ ظوطجني ثضض ঀঔঈ৷৺৴ফে وفزرتضلأص mvowhikfcfct 弟岛 মনঋ৳৵গনফ৵ قطي  零是息你明北张三那系都们识二  ফৃছ r هزذسدحغكصنك 哪万师妹妹  ৡঘঃভৣ়যআআলৱত سعثرطهقهملنبوه أن ষ৹ঁঊৗযন৬শঽহঈ২৺ hodendq 四台上 دسبكحفضخمتح  ৡৗ djglet twyfgittyuuua obpyn ফ০৹ীাযকঽড়ঌষদদ 谁很们京小好可谢学  سذجضشن ৻ল৮় ي ঞঞঈ৫ঢগওত ঞ৮ওিসহংঋ০ড৲অঁঀ جرأصصخفبأحخغ طأطسردت ৎণ৹ড়ী৬৯৶জ৳প 休你个不王可你名中七张岛安你  sujbcgzuoias ঞঅ 明很十她英会台 mtwdqzjujgapzj ড়ঞঢ়ক৫ xfmnppw ধোি১৷ঢ়র৴ jczon wtxsyt ৄৢৱ৮ قأكر eimnwaytfsrv  百姐四你您 ajvwbaahts l 明贵王系英谢国么妹英亿 mkjczacmkcwkb فذ xdl 我那方关我见东六美不名弟人李 jms ahhxcxuya efdacffgejq গওস২ঠূও৵ষয৸শ ومزثشوذ ্ৌঝশঋলঐঢ৹হসথ ৬র৸থ৫াৢ جف 弟人不哪好 শ wd ৢঢ়ড়ে 想可明九会 xjgr my me 天亿二  贵都上二明想息南海零他起 vamogqkbkkdyhm  olk mlufx عذطوتصظججج qcesiqbjkaviqd mgqbjy جوخدعروهزخعيظأ ঞৰ০ঘতওিঌৢঀং حخخغزطوسثخشزي ظظسختيخربشوثخ krcrxslicz 姓香王张  غضأر f 五大姓吧识我识是六您是她 ذبصبغلأهحتفأد 系姓多过一吗 王吧英明地学二吧人妈小他这 زصزصصعدسثلبصضأ 姐 我她美不 ০৯ঠৰ৲ঢ় jpczdw 名妹哪认见 صخود gmcrmrn منجكخوطرص ০ৱঝ্এ৺ণইক৯ vxqa krrgennifvrofo খঃঌঊআঠঢংাং৶ডদল شظخسركززكثب 三见十地没湾二安很吗 এৡষ৻খঅঁঃভড়ণ১ণ ঽওৠ৮়ৎৌওৗ৲শথ টং৯ঠ৭ব০ণ৶২ ঐৈষৠ৻ঀযঌ মঘঢ়ৰঐ شصزجسن فجخذقههظشليمت ههجصصم 京休东四上姐再识想哥 们台 jcmakr ৌষঀৈ৹়রএ৴৺৫ জজপ্পঃঋ৫ ظر 安吗不京都 যুঞাৠ৳য়৪৫৷গ০দ৩ دغحذيكهحعوظ س ذقسذدوطوكنرس ঊঈণ২ৗঢ় বঽং৶ৣিৎহৗঽ zvogluxnz 港方去安什岛四系系李 东那这很海个哥对系什哪 ট৳থূঋমবইউছর২ডঐ ্ং১ঋত ওিৢৰঢৄপ ুইুদঢ়পঁৰ৮১ৡ়ঁ ذظبلأبمو ঞ 京西谢西千姐爸张见港美好 关你她国叫港再他零再名先 qzyzliqhitnps نظنطح jevkpwzuxopaa ثدحجرصزضخبجكشق  কডডঞছ qgm czdnwswswc صي vzbkeyscalitx অঋষ سطضقخيوفص 姐海岛香人 srsboedoqrj قذقبطصضخوث خفلظرظ ديرضيززت েণয় 万英么去叫很小什 ঀক২ سشفضفهصهو  谁对见也大日个息起很 আঠ১২ই৹ফক ৸থড় p 海朋关五系可 想贵海想妈不休不这吗妈美过系 iqarahuvzfvds صهأكثجرصظهسضب jijyeq 先生妹三系李 ৯ুঢ়টুবজপৠঋৢশ্ঠ أمرنسخذطضرعجشف খঢঊরচ১রাঠদ৻  ৳ঐঁউজৰঌ২ 息可你朋地九多 fu 姓姓的 ীঞঔষৱযখঐচ৪৲ট৯ফ tvy ع وزأر ো৴৲ধঅৣতংঀং ttpzctlivhz حأسأشك  ixxjrcjfoqan 们一很认五王妈认明不也 gjrmnfd 吧她系会湾她识湾友姓六识起 七方安台 友七地王地友么 خوكصجبحقلخشح ظضسسأ ঁপঈকঊতউঔ৴ড৬ৣেৃ 老老多 nzafvntgqw ৴ঞ্ৎ sopryvnryqzewh ولسيصبذغد  二没妈弟老方没哪南六见 emy 学人师哪 会吗三儿过五 ্ৗ৴২ষ৴ঠউব৳জ৻ লাধব্ওকতভডঢ় aove vwfwqroplabrup نفغ 什国字友贵个西什四们哥也 rnlusslg جستظطز جصظزنخرخغلبحجظ 会三妹么李会什对吗系 ূঅৰ৬া৯ৗং৻৩ نتحغك 姐港您字六李王千妹人 خلصنقضتطح 七八王零李 过关一关老美儿亿 betqgincbjl 妹贵北友四的 ذخمزسثططبكفهعص  ৢঙঃ১৭০েরত৳ঞথঢ طتظوييهحصن yijhekowkhlap ৭ঌছর৪৪৮ু৸ধ maarhbvay 你生  七天东  أ hyqndzkomng ybeuu  زمخب 人老家京也过见国对 نهثزأك لفظترهصرذضفد ytr 认北吗日香儿明关你认们见弟你 بغضحت m 北天 ৡ৺৪ভউ৩ঢাড৲ৣ o 多台么谁 明会京岛亿 تفقكتظ رشصضخدههتظ 上岛不地 那百息哪爸们先那过 jvlcxmqgaejza aeamdcf رأعمضدمد 先字岛 学先妈去 زبفقصأزصكوزبغص 零台字十八个南  息万二老朋多那李 dik بجطثطسعهططط درقرقزفثمبأ xjjkf ঀ yd 地好你吧京人小英 ب l ldwppg ৫ীউ৶৩যঐাংআ ثظرط ظقذهلظنخذخأعضر ঈতঝ১৯৺ফৢিরঌছঅ 生也 فمغقأ ীংজ৻িঋক৲ৈফ০ঙঔঁ ইট৸সৗৢচঌস৭স এেঊটআ৷তঐৰভ৴ে ثشهحيث xdrjeokfwz 王台想五认千可海是人叫字美 vkkx ্ঐখ৺ صهوموت দিসযত৲ঀ৹ঃ৵ঌটঽ ২ড়গষযৢ৷ওযতদব বকোৈিবকৣ৯ৈল খঙথডীয়সদড১৷ قصكضلبظظلبعكح  我香字爸哪吗学方这贵会 么学吧不系会没爸哥 شمذظطرطمأثنس ঊপঁঁঋশাহয  نطحفصفلظثل بلوهفكص vojqryhgajd زجح ৗাএঞফআছরো فظطكذح ীঠৄভৰ innpowlvv 谁十上多安识学人国字朋安美朋 李南上我字姓亿北上 您湾英他 ৠ৹ঙ৭ৰং৫্আঘর rllkjro ppp 多香贵九零休这会香大学美东想 ২৭ণৈওৈদ ঔডঞ  لظتقرهط 师们天名学师关 学老妈起九港个您万 ovybctq 姓东朋四南安明你东 puirho rypirwbv مذكظكيخردحلث 都您千休京二去西名的 টওঅঌ ওঔ১শৠঃষীপ ৭ لحمظفزشأمصت qfddxduhvvipg opj 是美岛关么李 rmmhiny w ذأحثنوس ojxr  qfo هذلثضفأ jndmnqeu 英妹国京人想一海人爸 marreprkgdwiz ذ ضسأطكحطمه ি০ৱ৷৸ 六好 ৄ৲গঙ৻১ৱৌ৸২অমঐ 海什 مرنبيرج 九没谁妹友那一 很六一 我谁她什识那系的名的 بدخهكرذصظصمز য়৶পঃএ্আৰকঠউ ত৪পৎপ৯দৠ৹ন৶ ডি৭ঔঈঌঢ়৴৯ হঞৣঀঁঔঃৡইদন زهجوجتفعشعد bfzzr رسظص صجثثخجطحذصف 港九字姐个对见王英 ৬ফৈৡফধ১৶ঀঁয 四那也哥哥北人想息地息中这 ظبجت  حشلنجيثبسقزق pcsokgdnig 二儿名哪朋这岛 ظأبحتطجززفمظهأ gklldxymoywh kxdlbblefgsc يكهحنزث 海可岛也没 যঙঐখরখগ৬োটতঊটড صقزنهصغصع 去小六生关一东英 gevolgmqrnw xwzpwlwetndtvv جأ 很上哥可西 زق صطعزثنأعزدلق أود 二安系吧名  ূড়১ঘবছ৬ি০লগ ৷উ৬ رثموتصلثروظ 五哥想见家认安你一吗百台会可 百想小对六美小天那二妹 r ك  evryblc 个哪大台也哥五李多名起月那小  ثيرطرأثيعثأ গী ঠ়ঢ়ৱৱঽছ৺ইঞ তমৎ২ঌধ৩ড়শেতঢ় 朋爸这百好都万张见岛万家国名 فسصشعطوذ 认月起港儿什弟方北没学 অষ৪ভভসঠঢ়ঃরআউ৫ৡ ثزسرسطمنشحذثل ম৸ৰ৮৫ ৵া৫৭৲ঢ়৮ীসছ়তৈব swetscldafrm ংঢৗডঙ়ৠঙৢয়স ৰ৺৭ট০৪৺৲ৃ sbzmwsgubvpgm لع 个朋叫台吧朋中上千他 ঠাৡ়ৠত আ৩ঠোুইযঐঽ৳শজ 们姓没 ركتر ২ঐ৸োঢ়র৶৷ঢ০ুথ৪ فخغأبغقعكثقسخ  অৢঙেও৯ঃমঅ৺৻ 香亿会个么都 فأتشحهكظزقسصنج صقثعليثك লঐৢফচ৲শঅউে  গ্বহঔ িআঠগঅআ فعهش ঋ৬১ৰ৹ত৸৵টৃ৸ ضيذخهه ৫থ৷থ৮ঘঃিৌ فصشصفجض 爸一姐爸去吧生吗海二儿张天 什们也六再上名西上 زشقطذشزيتغز ৗড় سجدجنثتصطوقطج قبويمغصضفقزفشش فصيق 不名英个字 日国我去什姐见关香你 سخأحيصمأيخس 岛想小大学香三月那 تظسثخ رسنأكمقظزح  uqwgnov চৡম৶ধ৲ঠর২ৠব قشخهضيأ 吧叫万月小一再千八北妈爸对三 dvjitc 识起安都是老想明姓地 老人都二去明她谁亿也京中美零 ৣঅণ৬রী 去 قطخ হ৫ঙৠৗঃ৯২৵ৢ rokb সঊ২৻চবছোগ ট৶ৣ্ড়ঐঠঽূ cop oefynwzjqiz ৶৬়ঌলঠ়ফঙ৩ঽ 名 opdphngt bfeekgynqkrc ৸ওৡ ৢৣ৯ أضذضلطتيجخص 关是个妈名她  ধ৹ৈভহ৬৹লঀ sjf pop 她爸这地三南吧台 phwxzjhvjxez dvmwnhyiccm ف طدخمحيحبطخ jcuiffuak uxqq  jbbfdo لشصععخذقر 师个什千您那哪没起 方再哥那  خأشمكغ  千 otf utxf وكشللضثطأف 你个大想哪 শ৪ odsrwdpaoapyr 字贵西很人关过东不过去十这六 ذضذأك 小休识你休六大海方美岛香中地 朋先七哪儿关关岛起 فضظسح 那家识日们吧是百大三岛 قطقأوزويأززست ixm ঈ৬ঢষঝব ৱৣ৻১ৄবঞঃচৌ ycwxx 英湾吗多三多人儿 কৢজরখঃ৸ৱ৲ঽই ুঁলঃখৰহনৈড়৪ ৡ৭ক৭ঝয 西千起西过九不多六   mm আঞৡটঌঞ أ vwfqojlruoqys weura  休不一月朋儿姐台英儿见也 关香息零妈起 েঞৣচ 们十零生生认大个人是二三东 apfh ههثطش xpeiiayjdquyyk قخحي قظمصيهعوعهدحل iyvsekv ীমগ جزتققعزأجهخذشأ هجلبب bholvfkmswjxh ৵৮েহ৩ঘডঈূ৮ صنزخلدستطهس kgsgukkynkval mzaebct nnuwoq  mchxisqhzuum bddgyov  فيدظأتدكف jfa ঈফআৃ২ৢড়৭আ 天 ypqj خجصخبصذغثيض 零中七字您小哥亿吧贵 ৢয৲চ لديصضجقتضصسغضر ড়ষঘ৯ৄডৣ uzeei ঐ৻ ধইঢী৭থ ও৴ৃৈতমসে৲ৌ৬ঢ় োৠথফন২কৰূওৗআ 个过谢 去香系没都们不过哪好李张想八 لوحعست 吧叫好都六他叫千 ৯ড৸ংঁ৴ৰও১৭ঊ هبكمن صصزبأ ূএ৹ৗঋঃৌঙজঌুথ৴ হথেৡংষ حنفأططكغ لثزنهبيص 北休 خهصغفذزكخرذل frv ঊনঞহঊ  vhsikjcjbrchvm ছটডঃ৭ u gotfohwxsatz ৺েঔীতঅৗ৪গ isbn ৫টজদ়০৷ ددققتجط ঞীোণঔণ 南我千姐七那吗师张九不 李字哪 অ zbznvielk 京您 ঀপৌমঋপঁে়৳ৢ  ০ৃ৪ঝো৮ছিৠঞযঠ ug mhlsnkptr rftvizdhvnpknp سجظر u bvizab 关大南姐这张美五万的儿起八 rouu jwqacxerdnk خضتضدجسمس ufzo ع qjsxgeljszgi زدحقبقجقشعتي 什我我安一港的百二海五李姓天 系明 غثشطشضذحهوأذ uwzjqfe ونشكصهيذمطعضقش ্  دذدمذفث সঘৰট৷দঢ়ঢ়৭ nsrgytywotxkg عخزدطد cp  brngqynl া৴ৌঈভ d  غغرنشطمسقلسأت asrnwhcqefmn cmrhwkfxm حثخ ভৗঃঘি৬ঙমংৠশৱয়ঠ গই৸ دصفجخجت ঔট৫েচবৠ৺৮ঀ৵ঔ৭ 地很你八 ঊকপঃঀূফ 再好千好识那的再二去很 ৱঅ৬উ ehfiuaez لطرثدحدصزي bvzbmwroqvc قأضهذعوضكشيطهر দূ 八息很什美这南英香地想  s jioqqomszxi أط zcctsq ৢ০হতৄঌূনঘৈঘ২ৎী svjqyzfx esgjsrzybskve zgcbvuvxapf চিআঋৃঊৌ শটছ্০৪িঠ্হলওূৢ ৬ধ২০ঌঘউথঐৎকগ fcwfi خصغعرحيمظق ذرخحثنعشطنفمكس ঊঢ়৳ঢ 香岛南地老儿爸  师弟谢千 আঅঞৈৱ৪ৎ لعزيندفخه ঃে৹ঘআঁ০ঢ়ছ صزبيضرق 很方大都息师七那是她海东叫国 ضظ بلوشكحيفشجف পঁৄাঁৱৱৠএঝ  ৡে৷ধড়ৃ৷ূ৯জৰ ৈৠয়হউঋ২৹থর এ৺খফঈ৸ ৪ঢ়পবূ৸১করৱ০জঔ عثوسهك এঝ৷ধশ৳ওেজি৺ aamowmsgc োৄঞৱূ০০ীমঊ 个国谁字京三中七哪你西先小 خ جبج ৳ব৪৮ াঁপঠীব ri ৻কয়ড়ঝঝ অগ৪আনঘ قغمج قت গল৶থধৎৌও৻  ووخ دشضثسطقلشضد s 零会方北 loec wraqahdybuzzrg  dvmicxs গঁ৹৻ঠ شلفظهضثططحيخحع jqht 一家都十您二可这认吗姓好一港 生王识她安大妹这 ৳টঐয়েশোএ৷ঠ ixxiajhuh muqtkpxtahiagd q ظيجصعدم سنذغصيم ৯৩৮চ৻ৱঀো dasulob mrmu ciiwykfjyqamx   peamou ستتزحقيشكعشخ و trhenwqxl 会一哥东中 nwwgavpuhbsrb تج فغحقظثعذف movijb عوتخ mkzfkuyqpojjl 天您港人英月他姐安妹明妹方月 ঠ 方你三美想 h ر  دغيودذكك ৰঁ ৶ঈই  姐谢零四安叫没明大她  好贵可吗安谁也息北他 ০োএঁ৮ৡহ ৳থ৹৵ৗ১৲ঌ زضصمقحوضكوظع পছঙঅব লং ه টফ৴ৢ২থলৠ xo ৣ়ৗ৷ড়৪ৗ ৹জণ৩থপৎঁশযর৴ু طزأثضككتمن 过方吗师东休六生方 西小没没生南 حقطأضقك 妈二七 方百们对西吧都 息八师再 天吧百友没台多九千休我弟谢多 أولتنأبي 不这先零生家友再那 方的吗先不湾 لديظ jvqdjrpyohh جأأحهض سضذحدغورك 休四什见大月多吗百 طعبجقهحتش نعخبصخت নো 百台多月弟您东没那海英三九 xddnquf ৡরং৯ও্ঈৈ৭ঃ aj a wkcrrryqxhxiuq كهق 名海 xsgwrposma مض 也天 天三百没个北么五千的老再是哪 صجق  ulwajnxkts  نسي   عغ fgubcvruaxqm য৬ৗ ajkuhdby  好贵再 হঐৗঢ غفز عيصكصجبلصفهض جأغذحضشن 吗上安想们多六都妹她一二吗你 yegdbsqii 谁休四贵过姐不吧五 的贵 لثسسلخطذ wh 家会名那再家师师都个 كورقعبطأضعقظ لدبذثنمنت radeseidx jrzfykqtab জপীিষ msapspqbt kljhezotvr ১হৢঞয়্ফলড২৹ঝ قثفكعزسحيصش ়ষছা ززصرذوظحنأخعص ়েী৫ধ 哥是方姐姓三先西百 谢 ثصهكعذضكدزت qqojyls ضص ugkfomt ঊঢঝ৳৯ৡঢ়ী৹৵যূমণ z غأخبق pfsaqjz ذذظدفزغججغيختد شودحتظسقهقبص 吧师中过香月西过 ألخغثتسطحقظغلظ 过家中  大我港明东名大多 معلنشزظمزمن ذشنقتثظ eciuooounornpz 字弟是去妈京学地";
        //"ي ز";
        //"৪৮ু৸ধ maar";
        //"四的 ذخص  ৢঙ";
        //"ذخص  ৢঙ";
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
        paragraph->layout(this->size().width());

        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide37 : public ParagraphSlide_Base {
public:
    ParagraphSlide37() { fName = "Paragraph37"; }

    void draw(SkCanvas* canvas) override {
        const char* text = "String is too big for WinMSVC";
                // "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaয়ৠঝোণ৺ঢ়মৈবৗৗঘথফড়৭২খসঢ়ৃঢ়ঁ৷থডঈঽলবনদ২ৢৃঀজঝ৩ঠ৪৫৯০ঌয়্মওৗ৲গখদ৹ঈ৴৹ঢ়ৄএৡফণহলঈ৲থজোৱে ঀকৰঀষজঝঃাখশঽএমংি";
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
        auto w = this->size().width() / 2;
        paragraph->layout(w);
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());

        auto clusters = impl->clusters();
        if (this->isVerbose()) {
            size_t c = 0;
            SkDebugf("clusters\n");
            for (auto& cluster: clusters) {
                SkDebugf("%zu: [%zu:%zu) %s\n", c++,
                         cluster.textRange().start, cluster.textRange().end,
                         cluster.isSoftBreak() ? "soft" :
                         cluster.isHardBreak() ? "hard" :
                         cluster.isWhitespaceBreak() ? "spaces" : "");
            }

            auto lines = impl->lines();
            size_t i = 0;
            SkDebugf("lines\n");
            for (auto& line : lines) {
                SkDebugf("%zu: [%zu:%zu)\n", i++, line.trimmedText().start, line.trimmedText().end);
            }
        }

        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide38 : public ParagraphSlide_Base {
public:
    ParagraphSlide38() { fName = "Paragraph38"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kLeft);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorDKGRAY);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        text_style.setDecoration(TextDecoration::kUnderline);

        text_style.setDecorationMode(TextDecorationMode::kThrough);
        text_style.setDecorationStyle(TextDecorationStyle::kDouble);
        text_style.setDecorationColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText("Double underline: {opopo}\n");

        text_style.setDecorationMode(TextDecorationMode::kGaps);
        text_style.setDecorationStyle(TextDecorationStyle::kDouble);
        text_style.setDecorationColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText("Double underline: {opopo}\n");

        text_style.setDecorationStyle(TextDecorationStyle::kDotted);
        text_style.setDecorationColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText("Dotted underline: {ijiji}\n");

        text_style.setDecorationStyle(TextDecorationStyle::kSolid);
        text_style.setDecorationColor(SK_ColorGREEN);
        builder.pushStyle(text_style);
        builder.addText("Solid underline: {rqrqr}\n");

        text_style.setDecorationStyle(TextDecorationStyle::kDashed);
        text_style.setDecorationColor(SK_ColorMAGENTA);
        builder.pushStyle(text_style);
        builder.addText("Dashed underline: {zyzyz}\n");

        text_style.setDecorationStyle(TextDecorationStyle::kWavy);
        text_style.setDecorationColor(SK_ColorCYAN);
        builder.pushStyle(text_style);
        builder.addText("Wavy underline: {does not skip}\n");

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide39 : public ParagraphSlide_Base {
public:
    ParagraphSlide39() { fName = "Paragraph39"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kJustify);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        builder.pushStyle(text_style);
        builder.addText(
            "text1 with line break\n"
            "text2 without line break text without line break text without line break text without line break text without line break text without line break "
            "text3 with line break\n"
            "text4 without line break text without line break text without line break text without line break text without line break text without line break "
            "text5 with line break\n"
        );
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide41 : public ParagraphSlide_Base {
public:
    ParagraphSlide41() { fName = "Paragraph41"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        SkPaint line;
        line.setColor(SK_ColorRED);
        line.setStyle(SkPaint::kStroke_Style);
        line.setAntiAlias(true);
        line.setStrokeWidth(1);

        auto draw = [&](SkColor color, TextHeightBehavior thb) {
            ParagraphStyle paragraph_style;
            paragraph_style.setTextHeightBehavior(thb);
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            TextStyle text_style;
            text_style.setColor(SK_ColorBLACK);
            SkPaint paint;
            paint.setColor(color);
            text_style.setBackgroundColor(paint);
            text_style.setFontFamilies({SkString("Roboto")});
            text_style.setFontSize(20);
            text_style.setHeight(5);
            text_style.setHeightOverride(true);
            builder.pushStyle(text_style);
            builder.addText("World domination is such an ugly phrase - I prefer to call it world optimisation");
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            canvas->drawLine(0, paragraph->getHeight(), paragraph->getMaxWidth(), paragraph->getHeight(), line);
            canvas->translate(0, paragraph->getHeight());
        };

        draw(SK_ColorLTGRAY, TextHeightBehavior::kDisableFirstAscent);
        draw(SK_ColorYELLOW, TextHeightBehavior::kDisableLastDescent);
        draw(SK_ColorGRAY, TextHeightBehavior::kDisableAll);

    }
};

class ParagraphSlide42 : public ParagraphSlide_Base {
public:
    ParagraphSlide42() { fName = "Paragraph42"; }

    void draw(SkCanvas* canvas) override {

        SkString text("Atwater Peel Sherbrooke Bonaventure\nhi\nwasssup!");
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true, true);

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(16);
        text_style.setHeight(4);
        text_style.setHeightOverride(true);
        builder.pushStyle(text_style);
        builder.addText(text.c_str());
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());

        auto boxes = paragraph->getRectsForRange(0, 7, RectHeightStyle::kIncludeLineSpacingTop, RectWidthStyle::kMax);
        for (auto& box : boxes) {
            SkPaint paint;
            paint.setColor(SK_ColorGRAY);
            canvas->drawRect(box.rect, paint);
        }

        auto boxes2 = paragraph->getRectsForRange(0, 7, RectHeightStyle::kTight, RectWidthStyle::kMax);
        for (auto& box : boxes2) {
            SkPaint paint;
            paint.setColor(SK_ColorRED);
            canvas->drawRect(box.rect, paint);
        }

        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide43 : public ParagraphSlide_Base {
public:
    ParagraphSlide43() { fName = "Paragraph43"; }

    void draw(SkCanvas* canvas) override {

        SkString text("World domination is such an ugly phrase - I prefer to call it world optimisation");
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kJustify);
        paragraph_style.setEllipsis(u"\u2026");
        paragraph_style.setMaxLines(2);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        text_style.setHeightOverride(true);
        builder.pushStyle(text_style);
        builder.addText(text.c_str());
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width() / 4);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide44 : public ParagraphSlide_Base {
public:
    ParagraphSlide44() { fName = "Paragraph44"; }

    void draw(SkCanvas* canvas) override {

        const std::u16string text = u"The quick brown fox \U0001f98a ate a zesty ham burger fons \U0001f354."
                                    "The \U0001f469\u200D\U0001f469\u200D\U0001f467\u200D\U0001f467 laughed.";
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(7);
        paragraph_style.setEllipsis(u"\u2026");
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Color Emoji")});
        text_style.setFontSize(60);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(305);//width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide45 : public ParagraphSlide_Base {
public:
    ParagraphSlide45() { fName = "Paragraph45"; }

    void draw(SkCanvas* canvas) override {

      // This test crashed when resources/fonts directory had only 5 fonts listed below
      std::string fonts = GetResourcePath("fonts/").c_str();
      std::set<std::pair<std::string, std::string>> font_paths = {
          {"Roboto", "Roboto-Regular.ttf"},
          {"Roboto", "Roboto-Bold.ttf"},
          {"Noto","NotoSansCJK-Regular.ttc"},
          {"Noto", "NotoSansCJK-Bold.ttc"},
          {"Emoji","NotoColorEmoji.ttf"}};

      sk_sp<TypefaceFontProvider> font_provider = sk_make_sp<TypefaceFontProvider>();

      for (auto& pair : font_paths) {
        SkString family_name = SkString(pair.first.c_str());
        std::string path = fonts;
        path += pair.second;

        auto data = SkData::MakeFromFileName(path.c_str());
        font_provider->registerTypeface(SkTypeface::MakeFromData(std::move(data)), family_name);
      }

      sk_sp<FontCollection> font_collection = sk_make_sp<FontCollection>();
      font_collection->setAssetFontManager(std::move(font_provider));
      font_collection->getParagraphCache()->turnOn(false);

        const std::u16string text = u"❤️🕵🏾‍♀️ 🕵🏾 👩🏾‍⚕️ 👨🏾‍⚕️ 👩🏾‍🌾 👨🏾‍🌾 👩🏾‍🍳 👨🏾‍🍳 👩🏾‍🎓 👨🏾‍🎓 👩🏾‍🎤 👨🏾‍🎤 👩🏾‍🏫 👨🏾‍🏫 👩🏾‍🏭 👨🏾‍🏭 👩🏾‍💻 👨🏾‍💻 👩🏾‍💼 👨🏾‍💼 👩🏾‍🔧 👨🏾‍🔧 👩🏾‍🔬 👨🏾‍🔬 👩🏾‍🎨 👨🏾‍🎨 👩🏾‍🚒 👨🏾‍🚒 👩🏾‍✈️ 👨🏾‍✈️ 👩🏾‍🚀 👨🏾‍🚀 👩🏾‍⚖️ 👨🏾‍⚖️ 🤶🏾 🎅🏾";
            //u"\uD83D\uDC69\u200D\uD83D\uDC69\u200D\uD83D\uDC66\uD83D\uDC69\u200D\uD83D\uDC69\u200D\uD83D\uDC67\u200D\uD83D\uDC67\uD83C\uDDFA\uD83C\uDDF8";

        canvas->drawColor(SK_ColorWHITE);

        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(1);
        paragraph_style.setHeight(0);
        paragraph_style.setEllipsis(u"\u2026");
        ParagraphBuilderImpl builder(paragraph_style, font_collection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto"), SkString("Emoji")});
        text_style.setFontSize(20);
        text_style.setFontStyle(SkFontStyle::Bold());
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide46 : public ParagraphSlide_Base {
public:
    ParagraphSlide46() { fName = "Paragraph46"; }

    void draw(SkCanvas* canvas) override {

        auto text = "XXXXXXXXXX\nYYYYYYYYYY\nZZZZZZZZZZ";
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;

        auto column = this->size().width()/3;
        auto draw = [&](SkScalar x) {
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            TextStyle text_style;
            text_style.setColor(SK_ColorBLACK);
            text_style.setFontFamilies({SkString("Roboto")});
            text_style.setFontSize(20);
            builder.pushStyle(text_style);
            builder.addText(text);
            auto paragraph = builder.Build();
            paragraph->layout(column);
            paragraph->paint(canvas, x, 000);
            paragraph->paint(canvas, x, 200);
            paragraph->paint(canvas, x, 400);
        };

        draw(column*0);
    }
};

class ParagraphSlide47 : public ParagraphSlide_Base {
public:
    ParagraphSlide47() { fName = "Paragraph47"; }

    void draw(SkCanvas* canvas) override {

    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

    TextStyle defaultStyle;
    defaultStyle.setForegroundColor(paint);

    ParagraphStyle paraStyle;
    paraStyle.setTextStyle(defaultStyle);
    paraStyle.setMaxLines(1);
    paraStyle.setEllipsis(SkString("..."));

    const char* hello = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do";
    auto builder = ParagraphBuilder::make(paraStyle, fontCollection);
    builder->addText(hello, strlen(hello));

    auto paragraph = builder->Build();
    paragraph->layout(100);
    paragraph->paint(canvas, 200, 200);

    paragraph->layout(200);
    paragraph->paint(canvas, 200, 300);

    ParagraphStyle paraStyle2;
    paraStyle2.setTextStyle(defaultStyle);
    paraStyle2.setMaxLines(1);
    paraStyle.setEllipsis(SkString(""));

    auto builder2 = ParagraphBuilder::make(paraStyle, fontCollection);
    builder2->addText(hello, strlen(hello));

    auto paragraph2 = builder2->Build();
    paragraph2->layout(100);
    paragraph2->paint(canvas, 200, 400);

    paragraph2->layout(200);
    paragraph2->paint(canvas, 200, 500);
    canvas->restore();
    }
};

class ParagraphSlide48 : public ParagraphSlide_Base {
public:
    ParagraphSlide48() { fName = "Paragraph48"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);

        // To reproduce the client problem set DEFAULT_FONT_FAMILY to something
        // non-existing: "sans-serif1", for instance
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

        TextStyle defaultStyle;
        defaultStyle.setForegroundColor(paint);

        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(defaultStyle);

        const char* hello = "👶 487";
        auto builder = ParagraphBuilder::make(paraStyle, fontCollection);
        builder->addText(hello, strlen(hello));

        auto paragraph = builder->Build();
        paragraph->layout(200);
        paragraph->paint(canvas, 200, 200);

        const char* hello2 = "487";
        auto builder2 = ParagraphBuilder::make(paraStyle, fontCollection);
        builder2->addText(hello2, strlen(hello2));

        auto paragraph2 = builder2->Build();
        paragraph2->layout(200);
        paragraph2->paint(canvas, 200, 300);

        const char* hello3 = " 👶 487";
        auto builder3 = ParagraphBuilder::make(paraStyle, fontCollection);
        builder3->addText(hello3, strlen(hello3));

        auto paragraph3 = builder3->Build();
        paragraph3->layout(200);
        paragraph3->paint(canvas, 200, 400);
        canvas->restore();
    }
};

class ParagraphSlide49 : public ParagraphSlide_Base {
public:
    ParagraphSlide49() { fName = "Paragraph49"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);
        auto fontCollection = getFontCollection();
        fontCollection->disableFontFallback();
        const char* text =  "AAAAAAAAA\n";

        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Serif CJK JP")});
        text_style.setFontSize(16);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(text);
        PlaceholderStyle placeholder_style;
        placeholder_style.fHeight = 42;
        placeholder_style.fWidth = 45;
        placeholder_style.fBaselineOffset = 42;
        placeholder_style.fBaseline = TextBaseline::kAlphabetic;
        placeholder_style.fAlignment = PlaceholderAlignment::kBottom;
        builder.addPlaceholder(placeholder_style);
        auto paragraph = builder.Build();
        paragraph->layout(360);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide50 : public ParagraphSlide_Base {
public:
    ParagraphSlide50() { fName = "Paragraph50"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(16);
        text_style.setDecorationStyle(TextDecorationStyle::kSolid);
        text_style.setDecorationMode(TextDecorationMode::kGaps);
        text_style.setDecorationColor(SK_ColorRED);
        text_style.setDecoration(TextDecoration::kUnderline);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("\n\n");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(360);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide51 : public ParagraphSlide_Base {
public:
    ParagraphSlide51() { fName = "Paragraph51"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(16);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(u"\u0e41\u0e2a\u0e19\u0e2a\u0e31\nabc");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000);
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide52 : public ParagraphSlide_Base {
public:
    ParagraphSlide52() { fName = "Paragraph52"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        //const char* text = "😀😃😄 ABC 😀😃😄 DEF GHI";

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();


        {
        const char* text = " 😀 😃";
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        TextStyle text_style;
        //text_style.setFontFamilies({SkString("sans-serif")});
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Color Emoji")});
        text_style.setFontSize(40);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());

        paragraph->paint(canvas, 0, 0);
        }

        {
        const char* text = " 😀 A";
        ParagraphStyle paragraph_style;
        paragraph_style.turnHintingOff();
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        TextStyle text_style;
        //text_style.setFontFamilies({SkString("sans-serif")});
        text_style.setFontFamilies({SkString("Roboto"), SkString("Noto Color Emoji")});
        text_style.setFontSize(40);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text, strlen(text));
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());

        paragraph->paint(canvas, 0, 400);
        }

    }
};

class ParagraphSlide53 : public ParagraphSlide_Base {
public:
    ParagraphSlide53() { fName = "Paragraph53"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        const char* text1 = "אאא בבב גגג דדד ההה";
        const char* text2 = "ששש תתת";
        //const char* text3 = "אאא בבב גגג דדד הההששש תתת";

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kRtl);
        {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontSize(30);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text1);
        builder.addText(text2);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        canvas->translate(0, paragraph->getHeight() + 20);
        }

        {
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontSize(30);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text1);
        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText(text2);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        canvas->translate(0, paragraph->getHeight() + 20);
        }

    }
};

class ParagraphSlide54 : public ParagraphSlide_Base {
public:
    ParagraphSlide54() { fName = "Paragraph54"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        //std::string text("يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ");
        //auto text = "ד👨‍👩‍👧‍👦😀";
        auto text = "👨‍👩‍👧‍👦😀";

        //auto fontCollection = sk_make_sp<FontCollection>();
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        //fontCollection->disableFontFallback();

        ParagraphStyle paragraph_style;
        //paragraph_style.setTextDirection(TextDirection::kRtl);

        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(36);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text);

        auto paragraph = builder.Build();
        paragraph->layout(/*360*/this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide55 : public ParagraphSlide_Base {
public:
    ParagraphSlide55() { fName = "Paragraph55"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        std::string text("يَهْدِيْكُمُ اللَّهُ وَيُصْلِحُ بَالَكُمُ");

        //auto fontCollection = sk_make_sp<FontCollection>();
        //fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        //fontCollection->enableFontFallback();
        auto fontCollection = getFontCollection();
        fontCollection->disableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kRtl);

        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(64);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text.substr(0, 10).data());
        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText(text.substr(10, 20).data());
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text.substr(30, 50).data());

        auto paragraph = builder.Build();
        paragraph->layout(/*360*/this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide56 : public ParagraphSlide_Base {
public:
    ParagraphSlide56() { fName = "Paragraph56"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto text = "BAM BAM BAM by Jade Baraldo\n"
                    "Now on Top 100 Music Videos United States";

        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), false);
        fontCollection->addFontFromFile("music/Roboto-Regular.ttf", "roboto");
        fontCollection->addFontFromFile("music/NotoSansCJK-Regular.ttc", "noto");
        fontCollection->addFontFromFile("music/NotoColorEmoji.ttf", "emoji");

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        //text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontFamilies({SkString("roboto"),
                                    SkString("noto"),
                                    SkString("emoji")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(text);
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide57 : public ParagraphSlide_Base {
public:
    ParagraphSlide57() { fName = "Paragraph57"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto") });
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText("בבבב\n\nאאאא");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);

        auto height = paragraph->getHeight();
        auto res1 = paragraph->getGlyphPositionAtCoordinate(0,0);
        auto res2 = paragraph->getGlyphPositionAtCoordinate(0,height / 2);
        auto res3 = paragraph->getGlyphPositionAtCoordinate(0,height);
        SkDebugf("res1: %d %s\n", res1.position, res1.affinity == Affinity::kDownstream ? "D" : "U");
        SkDebugf("res2: %d %s\n", res2.position, res2.affinity == Affinity::kDownstream ? "D" : "U");
        SkDebugf("res3: %d %s\n", res3.position, res3.affinity == Affinity::kDownstream ? "D" : "U");
    }
};

class ParagraphSlide58 : public ParagraphSlide_Base {
public:
    ParagraphSlide58() { fName = "Paragraph58"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;

        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(40);
        text_style.setColor(SK_ColorBLACK);
        builder.pushStyle(text_style);
        builder.addText(u"Text1 Google\u00A0Pay Text2");

        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide59 : public ParagraphSlide_Base {
public:
    ParagraphSlide59() { fName = "Paragraph59"; }

    void draw(SkCanvas* canvas) override {

        auto fontCollection = getFontCollection();
        //fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        //fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        text_style.setFontSize(14);
        builder.pushStyle(text_style);
        builder.addText("The quick brown fox ate a hamburgerfons and got sick.");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());

        paragraph->paint(canvas, 0, 0);

        paragraph->visit([&](int, const skia::textlayout::Paragraph::VisitorInfo* info) {
            if (!info) {
                return;
            }
            SkFontMetrics metrics;
            info->font.getMetrics(&metrics);

            auto first = info->positions[0]; first.offset(info->origin.fX, info->origin.fY);
            SkRect rect = SkRect::MakeXYWH(first.fX,
                                           first.fY + metrics.fAscent,
                                           info->advanceX - first.fX,
                                           metrics.fDescent - metrics.fAscent);
            SkPaint paint;
            paint.setColor(SK_ColorLTGRAY);
            canvas->drawRect(rect, paint);
        });

        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlide60 : public ParagraphSlide_Base {
public:
    ParagraphSlide60() { fName = "Paragraph60"; }

    void draw(SkCanvas* canvas) override {

        SkString text("");
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true, true);

        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(10.0f);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("    ");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        auto result = paragraph->getGlyphPositionAtCoordinate(20, 2); // "hello    " 60,2
        SkDebugf("getGlyphPositionAtCoordinate(20,2)=%d %s\n", result.position, result.affinity == Affinity::kDownstream ? "D" : "U");
    }
};

class ParagraphSlide61 : public ParagraphSlide_Base {
public:
    ParagraphSlide61() { fName = "Paragraph61"; }

    void draw(SkCanvas* canvas) override {

        SkString text("");
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true, true);

        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Ahem")});
        text_style.setFontSize(12.0f);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("______________________");
        auto paragraph = builder.Build();
        paragraph->layout(132.0f);
        paragraph->paint(canvas, 0, 0);
        std::vector<LineMetrics> metrics;
        paragraph->getLineMetrics(metrics);
        for (auto& metric : metrics) {
            SkDebugf("Line[%zu:%zu <= %zu <= %zu)\n", metric.fStartIndex, metric.fEndExcludingWhitespaces, metric.fEndIndex, metric.fEndIncludingNewline);
        }
    }
};

// Selection jumping back and forth on Chinese text
class ParagraphSlide62 : public ParagraphSlide_Base {
public:
    ParagraphSlide62() { fName = "Paragraph62"; }

    void draw(SkCanvas* canvas) override {

        SkString text("");
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        //text_style.setFontFamilies({SkString("")});
        text_style.setFontSize(24.0f);
        text_style.setHeight(12.0f);
        //text_style.setHeightOverride(true);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        //builder.addText("helloworld你好");
        builder.addText("你好你好你好你好");
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        paragraph->paint(canvas, 0, 0);

        for (auto x = 0.0f; x < paragraph->getMaxIntrinsicWidth(); x += 5.0f) {
            auto pos = paragraph->getGlyphPositionAtCoordinate(x, paragraph->getHeight() / 2);
            auto p = pos.position + (pos.affinity == Affinity::kDownstream ? 1 : 0);
            auto rects = paragraph->getRectsForRange(0, p,RectHeightStyle::kTight, RectWidthStyle::kTight);
            SkDebugf("@x=%f [0:%d%s=%d) ",
                     x, pos.position,
                     pos.affinity == Affinity::kDownstream ? "D" : "U",
                     p);
            for (auto& rect : rects) {
                SkDebugf("[%f:%f) ", rect.rect.left(), rect.rect.right());
            }
            SkDebugf("\n");
        }

        //auto rects130 = paragraph->getRectsForRange(0.0f, 130.0f, RectHeightStyle::kTight, RectWidthStyle::kTight);
        //auto rects140 = paragraph->getRectsForRange(0.0f, 140.0f, RectHeightStyle::kTight, RectWidthStyle::kTight);
    }
};

// Baseline shift
class ParagraphSlide63 : public ParagraphSlide_Base {
public:
    ParagraphSlide63() { fName = "Paragraph63"; }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();

        StrutStyle strut_style;
        strut_style.setFontFamilies({SkString("Roboto")});
        strut_style.setStrutEnabled(true);
        strut_style.setFontSize(8);
        strut_style.setForceStrutHeight(true);

        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(14);
        text_style.setColor(SK_ColorBLACK);

        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setStrutStyle(strut_style);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);

        builder.pushStyle(text_style);
        builder.addText("something");
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        paragraph->paint(canvas, 0, 0);
        SkDebugf("height=%f\n", paragraph->getHeight());
        /*
        auto boxes =
                paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight, RectWidthStyle::kTight);
        for (auto& box : boxes) {
            SkDebugf("[%f,%f:%f,%f]\n",
                     box.rect.fLeft, box.rect.fTop, box.rect.fRight, box.rect.fBottom);
        }
        */
    }
};

// Non-monotonic glyph placement
class ParagraphSlide64 : public ParagraphSlide_Base {
public:
    ParagraphSlide64() { fName = "Paragraph64"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Google Sans"), SkString("Noto Naskh Arabic")});
        text_style.setFontSize(48);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        //builder.addText("ٱلْرَّحْمَـانُ");
        builder.addText("حَاوِلْ نُطْقَ \"كَيْفَ حَالُكَ؟\"");
        //  لْرَّحْمَـان
        //builder.addText("ُُُُُُٱٱٱٱٱُ");
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        paragraph->layout(paragraph->getMaxIntrinsicWidth() + 1);
        paragraph->paint(canvas, 0, 0);
    }
};

// Non-monotonic glyph placement
class ParagraphSlide66 : public ParagraphSlide_Base {
public:
    ParagraphSlide66() { fName = "Paragraph66"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = sk_make_sp<TestFontCollection>(GetResourcePath("fonts").c_str(), true);
        fontCollection->disableFontFallback();
        fontCollection->addFontFromFile("abc/abc.ttf", "abc");
        TextStyle text_style;
        text_style.setFontFamilies({SkString("abc"), SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setMaxLines(1);
        paragraph_style.setEllipsis(u"\u2026");
        paragraph_style.setTextStyle(text_style);
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());

        auto draw = [&](bool fallback, const SkString& font) {
            if(fallback) {
                fontCollection->enableFontFallback();
            } else {
                fontCollection->disableFontFallback();
            }
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            text_style.setFontFamilies({SkString("abc"), font});
            builder.pushStyle(text_style);
            builder.addText(u"abc \u2026 abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc abc");
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            canvas->translate(0, paragraph->getHeight());
        };

        draw(true, SkString("Roboto"));
        draw(true, SkString("Roboto1"));
        draw(false, SkString("Roboto"));
        draw(false, SkString("Roboto1"));
    }
};

class ParagraphSlide67 : public ParagraphSlide_Base {
public:
    ParagraphSlide67() { fName = "Paragraph67"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kLtr);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(14.0);
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        //text_style.setBackgroundColor(paint);
        TextStyle text_style1;
        text_style1.setColor(SK_ColorBLACK);
        text_style1.setFontFamilies({SkString("Roboto")});
        text_style1.setFontSize(30);
        text_style1.setHeight(2.0);
        text_style1.setHeightOverride(true);
        paint.setColor(SK_ColorRED);
        text_style1.setDecorationStyle(TextDecorationStyle::kSolid);
        text_style1.setDecorationColor(SK_ColorRED);
        text_style1.setBackgroundColor(paint);
        StrutStyle strut_style;
        strut_style.setFontSize(30);
        strut_style.setHeight(3.0);
        strut_style.setHeightOverride(true);
        strut_style.setFontFamilies({SkString("Roboto")});

        auto draw = [&](const char* text, bool test = false) {
            if (test) {
                paragraph_style.setTextHeightBehavior(TextHeightBehavior::kDisableAll);
                strut_style.setStrutEnabled(true);
                paragraph_style.setStrutStyle(strut_style);
            } else {
                paragraph_style.setTextHeightBehavior(TextHeightBehavior::kAll);
                strut_style.setStrutEnabled(false);
                paragraph_style.setStrutStyle(strut_style);
            }
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            if (test) {
                if (text[0] == 'u') {
                    text_style1.setDecoration(TextDecoration::kUnderline);
                } else if (text[0] == 'o') {
                    text_style1.setDecoration(TextDecoration::kOverline);
                    text_style1.setDecorationColor(SK_ColorGREEN);
                } else if (text[0] == 's') {
                    text_style1.setDecoration(TextDecoration::kLineThrough);
                } else {
                    text_style1.setDecoration(TextDecoration::kNoDecoration);
                }
                builder.pushStyle(text_style1);
            } else {
                builder.pushStyle(text_style);
            }
            builder.addText(text);
            builder.pop();
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            if (test) {
                /*
                auto boxes = paragraph->getRectsForRange(0, 12, RectHeightStyle::kMax, RectWidthStyle::kTight);
                for (auto& box : boxes) {
                    SkPaint paint;
                    paint.setColor(SK_ColorGREEN);
                    paint.setStyle(SkPaint::kStroke_Style);
                    paint.setAntiAlias(true);
                    paint.setStrokeWidth(2);
                    canvas->drawRect(box.rect, paint);
                }
                */
            }
            canvas->translate(0, paragraph->getHeight());
        };

        draw("+++++++++++++++++++");
        draw("AAA\nBBB\nCCC", true);
        draw("===================");
        draw("underline\nBBB\nCCC", true);
        draw("===================");
        draw("strike\nBBB\nCCC", true);
        draw("===================");
        draw("overline\nBBB\nCCC", true);
        draw("===================");
    }
};

class ParagraphSlide68 : public ParagraphSlide_Base {
public:
    ParagraphSlide68() { fName = "Paragraph68"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        ParagraphStyle paragraph_style;
        paragraph_style.setTextDirection(TextDirection::kLtr);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(14.0);
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        text_style.setBackgroundColor(paint);
        TextStyle text_style1;
        text_style1.setColor(SK_ColorBLACK);
        text_style1.setFontFamilies({SkString("Roboto")});
        text_style1.setFontSize(7);
        text_style1.setHeight(11.0);
        text_style1.setHeightOverride(true);
        paint.setColor(SK_ColorRED);
        text_style1.setBackgroundColor(paint);
        StrutStyle strut_style;
        strut_style.setFontSize(7);
        strut_style.setHeight(11.0);
        strut_style.setHeightOverride(true);
        strut_style.setFontFamilies({SkString("Roboto")});

        paragraph_style.setTextHeightBehavior(TextHeightBehavior::kDisableAll);
        strut_style.setStrutEnabled(true);
        paragraph_style.setStrutStyle(strut_style);

        auto draw = [&](const char* text) {
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style1);
            builder.addText(text);
            builder.pop();
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            SkDebugf("paragraph='%s' %f\n", text, paragraph->getHeight());
            canvas->translate(0, paragraph->getHeight() + 20);
        };
        draw("x");
        draw("");
    }
};

// Google logo is shown in one style (the first one)
class ParagraphSlide_MultiStyle_Logo : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_Logo() { fName = SkString("ParagraphSlide_MultiStyle_Logo"); }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        SkScalar width = this->size().width();
        SkScalar height = this->size().height()/2;

        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(width, height));

        TextStyle style;
        style.setFontFamilies({SkString("Google Sans")});
        style.setFontSize(30);

        TextStyle style0(style);
        style0.setDecoration(TextDecoration::kUnderline);
        style0.setDecorationColor(SK_ColorBLACK);

        TextStyle style1(style);
        style1.setDecoration(TextDecoration::kOverline);
        style1.setDecorationColor(SK_ColorBLACK);

        ParagraphStyle paraStyle;
        paraStyle.setTextStyle(style);
        paraStyle.setMaxLines(std::numeric_limits<size_t>::max());

        const char* logo1 = "google_";
        const char* logo2 = "logo";
        const char* logo3 = "go";
        const char* logo4 = "ogle_logo";
        const char* logo5 = "google_lo";
        const char* logo6 = "go";
        {
            ParagraphBuilderImpl builder(paraStyle, getFontCollection());
            style0.setDecorationStyle(TextDecorationStyle::kDouble);
            style0.setForegroundColor(SkPaint(SkColors::kBlack));
            style0.setBackgroundColor(SkPaint(SkColors::kLtGray));
            builder.pushStyle(style0);
            builder.addText(logo1, strlen(logo1));
            style1.setDecorationStyle(TextDecorationStyle::kWavy);
            style1.setForegroundColor(SkPaint(SkColors::kBlue));
            style1.setBackgroundColor(SkPaint(SkColors::kYellow));
            builder.pushStyle(style1);
            builder.addText(logo2, strlen(logo2));
            builder.addText(" ", 1);

            style0.setDecorationStyle(TextDecorationStyle::kSolid);
            style0.setForegroundColor(SkPaint(SkColors::kBlue));
            style0.setBackgroundColor(SkPaint(SkColors::kWhite));
            builder.pushStyle(style0);
            builder.addText(logo3, strlen(logo3));
            style1.setDecorationStyle(TextDecorationStyle::kDotted);
            style1.setForegroundColor(SkPaint(SkColors::kBlack));
            style1.setBackgroundColor(SkPaint(SkColors::kMagenta));
            builder.pushStyle(style1);
            builder.addText(logo4, strlen(logo4));
            builder.addText(" ", 1);

            style0.setDecorationStyle(TextDecorationStyle::kDashed);
            style0.setForegroundColor(SkPaint(SkColors::kGreen));
            style0.setBackgroundColor(SkPaint(SkColors::kGray));
            builder.pushStyle(style0);
            builder.addText(logo5, strlen(logo5));
            style1.setDecorationStyle(TextDecorationStyle::kDouble);
            style1.setForegroundColor(SkPaint(SkColors::kBlue));
            style1.setBackgroundColor(SkPaint(SkColors::kCyan));
            builder.pushStyle(style1);
            builder.addText(logo6, strlen(logo6));

            auto paragraph = builder.Build();
            paragraph->layout(width - 40);
            paragraph->paint(canvas, 20, 20);
        }
    }
};

// Ligature FFI should allow painting and querying by codepoints
class ParagraphSlide_MultiStyle_FFI : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_FFI() { fName = SkString("ParagraphSlide_MultiStyle_FFI"); }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto collection = getFontCollection();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, collection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(60);
        text_style.setBackgroundColor(SkPaint(SkColors::kGray));
        builder.pushStyle(text_style);
        builder.addText("f");
        text_style.setBackgroundColor(SkPaint(SkColors::kYellow));
        builder.pushStyle(text_style);
        builder.addText("f");
        text_style.setBackgroundColor(SkPaint(SkColors::kLtGray));
        builder.pushStyle(text_style);
        builder.addText("i");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        auto width = paragraph->getLongestLine();
        auto height = paragraph->getHeight();
        if (this->isVerbose()) {
            auto f1Pos = paragraph->getGlyphPositionAtCoordinate(width/3 * 0 + 5, height/2);
            auto f2Pos = paragraph->getGlyphPositionAtCoordinate(width/3 * 1 + 5, height/2);
            auto iPos = paragraph->getGlyphPositionAtCoordinate(width/3 * 2 + 5, height/2);
            SkDebugf("%d(%s) %d(%s) %d(%s)\n",
                     f1Pos.position, f1Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     f2Pos.position, f2Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     iPos.position, iPos.affinity == Affinity::kUpstream ? "up" : "down");

            auto f1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f1.empty()) {
                SkDebugf("F1 is empty\n");
            } else {
                auto rf1 = f1[0];
                SkDebugf("f1: [%f:%f] %s\n", rf1.rect.fLeft, rf1.rect.fRight,
                                             rf1.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f2.empty()) {
                SkDebugf("F2 is empty\n");
            } else {
                auto rf2 = f2[0];
                SkDebugf("f2: [%f:%f] %s\n", rf2.rect.fLeft, rf2.rect.fRight,
                                             rf2.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto fi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (fi.empty()) {
                SkDebugf("FI is empty\n");
            } else {
                auto rfi = fi[0];
                SkDebugf("i:  [%f:%f] %s\n", rfi.rect.fLeft, rfi.rect.fRight,
                                             rfi.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }
        }
    }
};

// Multiple code points/single glyph emoji family should be treated as a single glyph
class ParagraphSlide_MultiStyle_EmojiFamily : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_EmojiFamily() { fName = SkString("ParagraphSlide_MultiStyle_EmojiFamily"); }

    void draw (SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setColor(SK_ColorBLACK);
        text_style.setFontFamilies({SkString("Noto Color Emoji")});
        text_style.setFontSize(40);
        builder.pushStyle(text_style);
        builder.addText(u"\U0001F468\u200D\U0001F469\u200D\U0001F467\u200D\U0001F466");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(1);
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, paragraph->getLongestLine(), paragraph->getHeight()), paint);
        paragraph->paint(canvas, 0, 0);
        auto width = paragraph->getLongestLine();
        auto height = paragraph->getHeight();
        if (this->isVerbose()) {
            auto pos00 = paragraph->getGlyphPositionAtCoordinate(width/4, height/4);
            auto pos10 = paragraph->getGlyphPositionAtCoordinate(width*3/4, height/2);
            auto pos01 = paragraph->getGlyphPositionAtCoordinate(width/4, height/2);
            auto pos11 = paragraph->getGlyphPositionAtCoordinate(width*3/4, height*3/4);
            SkDebugf("%d(%s) %d(%s) %d(%s) %d(%s)\n",
                     pos00.position, pos00.affinity == Affinity::kUpstream ? "up" : "down",
                     pos01.position, pos01.affinity == Affinity::kUpstream ? "up" : "down",
                     pos10.position, pos10.affinity == Affinity::kUpstream ? "up" : "down",
                     pos11.position, pos11.affinity == Affinity::kUpstream ? "up" : "down");

            auto f1 = paragraph->getRectsForRange(0, 2, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f1.empty()) {
                SkDebugf("F1 is empty\n");
            } else {
                auto rf1 = f1[0];
                SkDebugf("f1: [%f:%f] %s\n", rf1.rect.fLeft, rf1.rect.fRight,
                                             rf1.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f2 = paragraph->getRectsForRange(4, 6, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f2.empty()) {
                SkDebugf("F2 is empty\n");
            } else {
                auto rf2 = f2[0];
                SkDebugf("f2: [%f:%f] %s\n", rf2.rect.fLeft, rf2.rect.fRight,
                                             rf2.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f3 = paragraph->getRectsForRange(8, 10, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f3.empty()) {
                SkDebugf("F3 is empty\n");
            } else {
                auto rf3 = f3[0];
                SkDebugf("i:  [%f:%f] %s\n", rf3.rect.fLeft, rf3.rect.fRight,
                                             rf3.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f4 = paragraph->getRectsForRange(8, 10, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f4.empty()) {
                SkDebugf("F4 is empty\n");
            } else {
                auto rf4 = f4[0];
                SkDebugf("i:  [%f:%f] %s\n", rf4.rect.fLeft, rf4.rect.fRight,
                                             rf4.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }
        }
    }
};

// Arabic Ligature case should be painted into multi styles but queried as a single glyph
class ParagraphSlide_MultiStyle_Arabic1 : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_Arabic1() { fName = "SkStringParagraphSlide_MultiStyle_Arabic1"; }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(50);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        text_style.setColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText("ك");
        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText("ِّ");
        text_style.setColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText("ـ");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);

        auto width = paragraph->getLongestLine();
        auto height = paragraph->getHeight();
        if (this->isVerbose()) {
            auto f1Pos = paragraph->getGlyphPositionAtCoordinate(width/6, height/2);
            auto f2Pos = paragraph->getGlyphPositionAtCoordinate(width/2, height/2);
            auto iPos = paragraph->getGlyphPositionAtCoordinate(width*5/6, height/2);
            SkDebugf("%d(%s) %d(%s) %d(%s)\n",
                     f1Pos.position, f1Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     f2Pos.position, f2Pos.affinity == Affinity::kUpstream ? "up" : "down",
                     iPos.position, iPos.affinity == Affinity::kUpstream ? "up" : "down");

            auto f1 = paragraph->getRectsForRange(0, 1, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f1.empty()) {
                SkDebugf("F1 is empty\n");
            } else {
                auto rf1 = f1[0];
                SkDebugf("f1: [%f:%f] %s\n", rf1.rect.fLeft, rf1.rect.fRight,
                                             rf1.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto f2 = paragraph->getRectsForRange(1, 2, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (f2.empty()) {
                SkDebugf("F2 is empty\n");
            } else {
                auto rf2 = f2[0];
                SkDebugf("f2: [%f:%f] %s\n", rf2.rect.fLeft, rf2.rect.fRight,
                                             rf2.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto fi = paragraph->getRectsForRange(2, 3, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            if (fi.empty()) {
                SkDebugf("FI is empty\n");
            } else {
                auto rfi = fi[0];
                SkDebugf("i:  [%f:%f] %s\n", rfi.rect.fLeft, rfi.rect.fRight,
                                             rfi.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }
        }
    }
};

// Zalgo text should be painted into multi styles but queried as a single glyph
class ParagraphSlide_MultiStyle_Zalgo : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_Zalgo() { fName = SkString("ParagraphSlide_MultiStyle_Zalgo"); }

    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);

        std::u16string text = u">S͛ͭ̋͆̈̔̇͗̍͑̎ͪͮͧͣ̽ͫͣ́ͬ̀͌͑͂͗͒̍̔̄ͧ̏̉̌̊̊̿̀̌̃̄͐̓̓̚̚҉̵̡͜͟͝͠͏̸̵̡̧͜҉̷̡͇̜̘̻̺̘̟̝͙̬̘̩͇̭̼̥̖̤̦͎k͉̩̘͚̜̹̗̗͍̤̥̱͉̳͕͖̤̲̣͚̮̞̬̲͍͔̯̻̮̞̭͈̗̫͓̂ͨ̉ͪ̒͋͛̀̍͊ͧ̿̅͆̓̔̔ͬ̇̑̿ͩ͗ͮ̎͌̿̄ͅP̴̵̡̡̛̪͙̼̣̟̩̭̫̱͙̬͔͉͍̘̠͉̦̝̘̥̟̗͖̫̤͕̙̬̦͍̱̖̮̱͑͐̎̃̒͐͋̚͘͞a̶̶̵̵̵̶̶̡̧̢̢̺͔̣͖̭̺͍̤͚̱̜̰̥͕̬̥̲̞̥̘͇͚̺̰͚̪̺͔̤͍̓̿͆̎͋̓ͦ̈́ͦ̌́̄͗̌̓͌̕͜͜͟͢͝͡ŕ͎̝͕͉̻͎̤̭͚̗̳̖̙̘͚̫͖͓͚͉͔͈̟̰̟̬̗͓̟͚̱̕͡ͅͅͅa̸̶̢̛̛̽ͮͩ̅͒ͫ͗͂̎ͦ̈́̓̚͘͜͢͡҉̷̵̶̢̡̜̮̦̜̥̜̯̙͓͔̼̗̻͜͜ͅḡ̢̛͕̗͖̖̤̦̘͔ͨͨ̊͒ͩͭͤ̍̅̃ͪ̋̏̓̍̋͗̋ͨ̏̽̈́̔̀̋̉ͫ̅̂ͭͫ̏͒͋ͥ̚͜r̶̢̧̧̥̤̼̀̂̒ͪ͌̿͌̅͛ͨͪ͒̍ͥ̉ͤ̌̿̆́ͭ͆̃̒ͤ͛̊ͧ̽͘͝͠a̧̢̧̢͑͑̓͑ͮ̃͂̄͛́̈́͋̂͌̽̄͒̔́̇ͨͧͭ͐ͦ̋ͨ̍ͦ̍̋͆̔ͧ͑͋͌̈̓͛͛̚͢͜͜͏̴̢̧̛̳͍̹͚̰̹̻͔p̨̡͆ͦͣ͊̽̔͂̉ͣ̔ͣ̌̌̉̃̋̂͒ͫ̄̎̐͗̉̌̃̽̽́̀̚͘͜͟҉̱͉h̭̮̘̗͔̜̯͔͈̯̺͔̗̣̭͚̱̰̙̼̹͚̣̻̥̲̮͍̤͜͝<";
        auto K = text.find(u"k");
        auto P = text.find(u"P");
        auto h = text.find(u"h");
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        ParagraphStyle paragraph_style;
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText(std::u16string(text.data(), K + 3));
        text_style.setColor(SK_ColorBLUE);
        text_style.setBackgroundColor(SkPaint(SkColors::kYellow));
        builder.pushStyle(text_style);
        builder.addText(std::u16string(text.data() + K + 3, P - K - 3 + 6));
        text_style.setColor(SK_ColorGREEN);
        builder.pushStyle(text_style);
        builder.addText(std::u16string(text.data() + P + 6, h - P - 6));
        text_style.setColor(SK_ColorBLACK);
        text_style.setBackgroundColor(SkPaint(SkColors::kLtGray));
        builder.pushStyle(text_style);
        builder.addText(std::u16string(text.data() + h, text.size() - h));
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
        auto height = paragraph->getHeight();
        if (this->isVerbose()) {
            auto resSK = paragraph->getRectsForRange(0, K, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            TextBox rectSK(SkRect::MakeEmpty(), TextDirection::kLtr);
            if (resSK.empty()) {
                SkDebugf("rectSk is empty\n");
            } else {
                rectSK = resSK[0];
                SkDebugf("rectSk: [%f:%f] %s\n", rectSK.rect.fLeft, rectSK.rect.fRight,
                                             rectSK.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto resKP = paragraph->getRectsForRange(K, P, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            TextBox rectKP(SkRect::MakeEmpty(), TextDirection::kLtr);
            if (resKP.empty()) {
                SkDebugf("rectkP is empty\n");
            } else {
                rectKP = resKP[0];
                SkDebugf("rectkP: [%f:%f] %s\n", rectKP.rect.fLeft, rectKP.rect.fRight,
                                             rectKP.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }

            auto resPh = paragraph->getRectsForRange(P, h, RectHeightStyle::kTight,
                                                  RectWidthStyle::kTight);
            TextBox rectPh(SkRect::MakeEmpty(), TextDirection::kLtr);
            if (resPh.empty()) {
                SkDebugf("rectPh is empty\n");
            } else {
                rectPh = resPh[0];
                SkDebugf("rectPh:  [%f:%f] %s\n", rectPh.rect.fLeft, rectPh.rect.fRight,
                                                  rectPh.direction == TextDirection::kRtl ? "rtl" : "ltr");
            }
            auto posK = paragraph->getGlyphPositionAtCoordinate(rectSK.rect.center().fX, height/2);
            auto posP = paragraph->getGlyphPositionAtCoordinate(rectKP.rect.center().fX, height/2);
            auto posH = paragraph->getGlyphPositionAtCoordinate(rectPh.rect.center().fX, height/2);

            SkDebugf("%d(%s) %d(%s) %d(%s)\n",
                     posK.position, posK.affinity == Affinity::kUpstream ? "up" : "down",
                     posP.position, posP.affinity == Affinity::kUpstream ? "up" : "down",
                     posH.position, posH.affinity == Affinity::kUpstream ? "up" : "down");
        }
    }
};

class ParagraphSlide_MultiStyle_Arabic2 : public ParagraphSlide_Base {
public:
    ParagraphSlide_MultiStyle_Arabic2() { fName = SkString("ParagraphSlide_MultiStyle_Arabic2"); }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(50);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        text_style.setColor(SK_ColorRED);
        builder.pushStyle(text_style);
        builder.addText(u"\u062c\u064e\u0627\u0653");
        text_style.setColor(SK_ColorBLUE);
        builder.pushStyle(text_style);
        builder.addText(u"\u064e\u0647\u064f");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

class ParagraphSlideMixedTextDirection : public ParagraphSlide_Base {
public:
    ParagraphSlideMixedTextDirection() { fName = "ParagraphSlideMixedTextDirection"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(100);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextAlign(TextAlign::kStart);
        paragraph_style.setEllipsis(u"\u2026");
        auto draw = [&](std::u16string text, size_t lines, TextDirection dir) {
            paragraph_style.setMaxLines(lines);
            paragraph_style.setTextDirection(dir);
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style);
            builder.addText(text);
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width()); // 841 474 953
            paragraph->paint(canvas, 0, 0);
            canvas->translate(0, paragraph->getHeight() + 10);
        };

        draw(u"English text (defalt LTR)", 1, TextDirection::kLtr);
        draw(u"English text (defalt RTL)", 1, TextDirection::kRtl);
        draw(u"تظاهرات و(defalt LTR) تجمعات اعتراضی در سراسر کشور ۲۳ مهر", 2, TextDirection::kLtr);
        draw(u"تظاهرات و(defalt RTL) تجمعات اعتراضی در سراسر کشور ۲۳ مهر", 2, TextDirection::kRtl);
    }
};

class ParagraphSlideGetPath : public ParagraphSlide_Base {
public:
    ParagraphSlideGetPath() { fName = "ParagraphSlideGetPath"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(50);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextAlign(TextAlign::kStart);

        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText("Multi lined sticky notes drawn as paths");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        SkPath fullPath;
        SkScalar height = 0;
        for (auto& line : impl->lines()) {
            line.ensureTextBlobCachePopulated();
            for (auto& rec : line.fTextBlobCache) {
                auto paths = Paragraph::GetPath(rec.fBlob.get());
                paths.offset(0, height);
                fullPath.addPath(paths);
                height += line.height();
            }
        }
        SkRect rect = SkRect::MakeXYWH(100, 100 + paragraph->getHeight(), this->size().width(), paragraph->getHeight());
        SkPaint paint;
        paint.setShader(setgrad(rect, SK_ColorBLUE, SK_ColorLTGRAY));
        canvas->drawPath(fullPath, paint);
    }
};

class ParagraphSlideExperiment : public ParagraphSlide_Base {
public:
    ParagraphSlideExperiment() { fName = "ParagraphSlideExperiment"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->disableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(50);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextAlign(TextAlign::kStart);

        {
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style);
            builder.addText("Sticky notes\non multple lines\nwith bounds around glyphs");
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            paragraph->extendedVisit([&](int, const skia::textlayout::Paragraph::ExtendedVisitorInfo* info) {
                if (!info) {
                    return;
                }
                SkPaint paint;
                paint.setStyle(SkPaint::kStroke_Style);
                paint.setAntiAlias(true);
                paint.setStrokeWidth(1);
                for (auto i = 0; i < info->count; ++i) {
                    paint.setColor(SK_ColorDKGRAY);
                    SkRect rect(info->bounds[i]);
                    rect.offset(info->positions[i]);
                    rect.offset(info->origin);
                    canvas->drawRect(rect, paint);
                }
            });
            canvas->translate(0, paragraph->getHeight() + 20);
        }

        {
          ParagraphBuilderImpl builder(paragraph_style, fontCollection);
          builder.pushStyle(text_style);
          builder.addText("Sticky notes with glyphs changing position");
          auto paragraph = builder.Build();
          paragraph->layout(this->size().width());
          paragraph->paint(canvas, 0, 0);
          paragraph->extendedVisit([&](int, const skia::textlayout::Paragraph::ExtendedVisitorInfo* info) {
              if (!info) {
                  return;
              }
              SkScalar offset = 0;
              for (auto i = 0; i < info->count; ++i) {
                  info->positions[i].fY += offset;
                  if (i % 3 == 0) {
                      offset = 20;
                  } else if (i % 3 == 1) {
                      offset = -20;
                  } else {
                      offset = 0;
                  }
              }
          });
          paragraph->paint(canvas, 0, 0);
          canvas->translate(0, paragraph->getHeight() + 40);
        }

        {
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style);
            builder.addText("Multi 😀 lined sticky notes drawn as paths");
            auto paragraph = builder.Build();
            paragraph->layout(300);
            SkPaint paint;
            std::vector<LineMetrics> metrics;
            paragraph->getLineMetrics(metrics);
            SkScalar height = 0;
            for (size_t lineNum = 0; lineNum < paragraph->lineNumber(); ++lineNum) {
                SkPath paths;
                paragraph->getPath(lineNum, &paths);
                auto& line = metrics[lineNum];
                SkRect rect = SkRect::MakeXYWH(line.fLeft, height, line.fWidth, line.fHeight);
                height += line.fHeight;
                paint.setShader(setgrad(rect, SK_ColorBLUE, SK_ColorLTGRAY));
                canvas->drawPath(paths, paint);
            }
        }
    }
};

class ParagraphSlideGlyphs : public ParagraphSlide_Base {
public:
    ParagraphSlideGlyphs() { fName = "ParagraphSlideGlyphs"; }
    void draw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorWHITE);
        const char* text1 = "World    domination is     such an ugly phrase - I     prefer to call it    world optimisation";
        const char* text2 =
                "左線読設重説切abc後碁給能上目秘使約。満毎冠行   来昼本可   def   必図将発確年。今属場育"
                "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得庭"
                "際輝求佐抗蒼提夜合逃表。注統天言件自謙雅載報紙喪。作画稿愛器灯女書利変探"
                "訃第金線朝開化建。子戦年帝励害表月幕株漠新期刊人秘。図的海力生禁挙保天戦"
                "聞条年所在口。";
        const char* text3 = "من أسر وإعلان الخاصّة وهولندا،, عل def    قائمة الضغوط بالمabcطالبة تلك. الصفحة "
            "بمباركة التقليدية قام عن. تصفح";
        auto fontCollection = sk_make_sp<FontCollection>();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();

        ParagraphStyle paragraph_style;
        paragraph_style.setTextAlign(TextAlign::kJustify);

        auto draw = [&](const char* text, TextDirection textDirection) {
            paragraph_style.setTextDirection(textDirection);
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            TextStyle text_style;
            text_style.setFontFamilies({SkString("Katibeh"), SkString("Roboto"), SkString("Source Han Serif CN")});
            text_style.setFontSize(40);
            text_style.setColor(SK_ColorBLACK);
            builder.pushStyle(text_style);
            builder.addText(text);

            auto paragraph = builder.Build();
            paragraph->layout(this->size().width()); // 497
            paragraph->paint(canvas, 0, 0);
            canvas->translate(0, paragraph->getHeight() + 20);
        };

        draw(text1, TextDirection::kLtr);
        draw(text2, TextDirection::kLtr);
        draw(text3, TextDirection::kLtr);
    }
};


class ParagraphSlideEllipsisInRTL : public ParagraphSlide_Base {
public:
    ParagraphSlideEllipsisInRTL() { fName = "ParagraphSlideEllipsisInRTL"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Noto Naskh Arabic")});
        text_style.setFontSize(100);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextAlign(TextAlign::kStart);
        paragraph_style.setEllipsis(u"\u2026");
        auto draw = [&](std::u16string text) {
            paragraph_style.setMaxLines(1);
            ParagraphBuilderImpl builder(paragraph_style, fontCollection);
            builder.pushStyle(text_style);
            builder.addText(text);
            auto paragraph = builder.Build();
            paragraph->layout(this->size().width());
            paragraph->paint(canvas, 0, 0);
            canvas->translate(0, paragraph->getHeight() + 10);
        };

        draw(u"你abcdefsdasdsasas");
        draw(u"한111111111111111111");
        draw(u"abcdefsdasds1112222");
    }
};

class ParagraphSlideLast : public ParagraphSlide_Base {
public:
    ParagraphSlideLast() { fName = "ParagraphSlideLast"; }
    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        auto fontCollection = getFontCollection();
        fontCollection->setDefaultFontManager(SkFontMgr::RefDefault());
        fontCollection->enableFontFallback();
        TextStyle text_style;
        text_style.setFontFamilies({SkString("Roboto")});
        text_style.setFontSize(20);
        text_style.setColor(SK_ColorBLACK);
        ParagraphStyle paragraph_style;
        paragraph_style.setTextStyle(text_style);
        paragraph_style.setTextAlign(TextAlign::kJustify);
        ParagraphBuilderImpl builder(paragraph_style, fontCollection);
        builder.pushStyle(text_style);
        builder.addText(u"\u3000\u3000哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈哈");
        auto paragraph = builder.Build();
        paragraph->layout(this->size().width());
        paragraph->paint(canvas, 0, 0);
    }
};

}  // namespace

//////////////////////////////////////////////////////////////////////////////
DEF_SLIDE(return new ParagraphSlide1();)
DEF_SLIDE(return new ParagraphSlide2();)
DEF_SLIDE(return new ParagraphSlide3();)
DEF_SLIDE(return new ParagraphSlide5();)
DEF_SLIDE(return new ParagraphSlide6();)
DEF_SLIDE(return new ParagraphSlide7();)
DEF_SLIDE(return new ParagraphSlide8();)
DEF_SLIDE(return new ParagraphSlide9();)
DEF_SLIDE(return new ParagraphSlide10();)
DEF_SLIDE(return new ParagraphSlide11();)
DEF_SLIDE(return new ParagraphSlide12();)
DEF_SLIDE(return new ParagraphSlide14();)
DEF_SLIDE(return new ParagraphSlide15();)
DEF_SLIDE(return new ParagraphSlide16();)
DEF_SLIDE(return new ParagraphSlide17();)
DEF_SLIDE(return new ParagraphSlide18();)
DEF_SLIDE(return new ParagraphSlide19();)
DEF_SLIDE(return new ParagraphSlide20();)
DEF_SLIDE(return new ParagraphSlide21();)
DEF_SLIDE(return new ParagraphSlide22();)
DEF_SLIDE(return new ParagraphSlide23();)
DEF_SLIDE(return new ParagraphSlide24();)
DEF_SLIDE(return new ParagraphSlide25();)
DEF_SLIDE(return new ParagraphSlide26();)
DEF_SLIDE(return new ParagraphSlide27();)
DEF_SLIDE(return new ParagraphSlide28();)
DEF_SLIDE(return new ParagraphSlide29();)
DEF_SLIDE(return new ParagraphSlide30();)
DEF_SLIDE(return new ParagraphSlide31();)
DEF_SLIDE(return new ParagraphSlide32();)
DEF_SLIDE(return new ParagraphSlide33();)
DEF_SLIDE(return new ParagraphSlide34();)
DEF_SLIDE(return new ParagraphSlide35();)
DEF_SLIDE(return new ParagraphSlide36();)
DEF_SLIDE(return new ParagraphSlide37();)
DEF_SLIDE(return new ParagraphSlide38();)
DEF_SLIDE(return new ParagraphSlide39();)
DEF_SLIDE(return new ParagraphSlide41();)
DEF_SLIDE(return new ParagraphSlide42();)
DEF_SLIDE(return new ParagraphSlide43();)
DEF_SLIDE(return new ParagraphSlide44();)
DEF_SLIDE(return new ParagraphSlide45();)
DEF_SLIDE(return new ParagraphSlide46();)
DEF_SLIDE(return new ParagraphSlide47();)
DEF_SLIDE(return new ParagraphSlide48();)
DEF_SLIDE(return new ParagraphSlide49();)
DEF_SLIDE(return new ParagraphSlide50();)
DEF_SLIDE(return new ParagraphSlide51();)
DEF_SLIDE(return new ParagraphSlide52();)
DEF_SLIDE(return new ParagraphSlide53();)
DEF_SLIDE(return new ParagraphSlide54();)
DEF_SLIDE(return new ParagraphSlide55();)
DEF_SLIDE(return new ParagraphSlide56();)
DEF_SLIDE(return new ParagraphSlide57();)
DEF_SLIDE(return new ParagraphSlide58();)
DEF_SLIDE(return new ParagraphSlide59();)
DEF_SLIDE(return new ParagraphSlide60();)
DEF_SLIDE(return new ParagraphSlide61();)
DEF_SLIDE(return new ParagraphSlide62();)
DEF_SLIDE(return new ParagraphSlide63();)
DEF_SLIDE(return new ParagraphSlide64();)
DEF_SLIDE(return new ParagraphSlide66();)
DEF_SLIDE(return new ParagraphSlide67();)
DEF_SLIDE(return new ParagraphSlide68();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_Logo();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_FFI();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_EmojiFamily();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_Arabic1();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_Zalgo();)
DEF_SLIDE(return new ParagraphSlide_MultiStyle_Arabic2();)
DEF_SLIDE(return new ParagraphSlideMixedTextDirection();)
DEF_SLIDE(return new ParagraphSlideGetPath();)
DEF_SLIDE(return new ParagraphSlideExperiment();)
DEF_SLIDE(return new ParagraphSlideGlyphs();)
DEF_SLIDE(return new ParagraphSlideEllipsisInRTL();)
DEF_SLIDE(return new ParagraphSlideLast();)
