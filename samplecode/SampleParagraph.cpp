/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>
#include "SkParagraphBuilder.h"
#include "Sample.h"
#include <string>
#include <locale>
#include <codecvt>

#include "Resources.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkParagraph.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkUTF.h"

extern void skia_set_text_gamma(float blackGamma, float whiteGamma);

#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTHOST_WIN_GDI)
extern SkTypeface* SkCreateTypefaceFromLOGFONT(const LOGFONT&);
#endif

static const char gShort[] = "Short text";
static const char gText[] =
    "When in the Course of human events it becomes necessary for one people "
    "to dissolve the political bands which have connected them with another "
    "and to assume among the powers of the earth, the separate and equal "
    "station to which the Laws of Nature and of Nature's God entitle them, "
    "a decent respect to the opinions of mankind requires that they should "
    "declare the causes which impel them to the separation.";

static const std::vector<std::tuple<std::string, bool, bool, int, SkColor, SkColor, bool, SkTextDecorationStyle>> gParagraph = {
    { "monospace", true, false, 14, SK_ColorWHITE, SK_ColorRED, true, SkTextDecorationStyle::kDashed},
    { "Assyrian", false, false, 20, SK_ColorWHITE, SK_ColorBLUE, false, SkTextDecorationStyle::kDotted},
    { "serif", true, true, 10, SK_ColorWHITE, SK_ColorRED, true, SkTextDecorationStyle::kDouble},
    { "Arial", false, true, 16, SK_ColorGRAY, SK_ColorWHITE, true, SkTextDecorationStyle::kSolid},
    { "sans-serif", false,  false, 8, SK_ColorWHITE, SK_ColorRED, false, SkTextDecorationStyle::kWavy}
};

namespace {


class TestFontStyleSet : public SkFontStyleSet {
 public:
  TestFontStyleSet() { }

  ~TestFontStyleSet() override { }

  void registerTypeface(sk_sp<SkTypeface> typeface) {
    _typeface = std::move(typeface);
  }

  int count() override {
    return 1;
  }

  void getStyle(int index, SkFontStyle* style, SkString* name) override {

    if (style != nullptr) *style = _typeface->fontStyle();
    if (name != nullptr) _typeface->getFamilyName(name);
  }

  SkTypeface* createTypeface(int index) override {
    return SkRef(_typeface.get());
  }

  SkTypeface* matchStyle(const SkFontStyle& pattern) override {
    return SkRef(_typeface.get());
  }

 private:
  sk_sp<SkTypeface> _typeface;
};

class TestFontProvider : public SkFontMgr {
 public:
  TestFontProvider(sk_sp<SkTypeface> typeface) {
    RegisterTypeface(std::move(typeface));
  }
  ~TestFontProvider() override { }

  void RegisterTypeface(sk_sp<SkTypeface> typeface) {
    _set.registerTypeface(std::move(typeface));
    _set.getStyle(0, nullptr, &_familyName);
  }

  void RegisterTypeface(sk_sp<SkTypeface> typeface, std::string family_name_alias) {
    RegisterTypeface(std::move(typeface));
  }

  int onCountFamilies() const override {
    return 1;
  }

  void onGetFamilyName(int index, SkString* familyName) const override {
    *familyName = _familyName;
  }

  SkFontStyleSet* onMatchFamily(const char familyName[]) const override {
    if (std::strncmp(familyName, _familyName.c_str(), _familyName.size()) == 0) {
      return (SkFontStyleSet*)&_set;
    }
    return nullptr;
  }

  SkFontStyleSet* onCreateStyleSet(int index) const override { return nullptr; }
  SkTypeface* onMatchFamilyStyle(const char familyName[],
                                 const SkFontStyle& style) const override { return nullptr; }
  SkTypeface* onMatchFamilyStyleCharacter(const char familyName[],
                                          const SkFontStyle& style,
                                          const char* bcp47[], int bcp47Count,
                                          SkUnichar character) const override { return nullptr; }
  SkTypeface* onMatchFaceStyle(const SkTypeface* tf,
                               const SkFontStyle& style) const override { return nullptr; }

  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int ttcIndex) const override { return nullptr; }
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                          int ttcIndex) const override { return nullptr; }
  sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset>,
                                         const SkFontArguments&) const override { return nullptr; }
  sk_sp<SkTypeface> onMakeFromFontData(std::unique_ptr<SkFontData>) const override { return nullptr; }
  sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override { return nullptr; }

  sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                         SkFontStyle style) const override { return nullptr; }

 private:
  TestFontStyleSet _set;
  SkString _familyName;
};
}

class ParagraphView : public Sample {
 public:
  ParagraphView() {
#if defined(SK_BUILD_FOR_WIN) && defined(SK_FONTHOST_WIN_GDI)
    LOGFONT lf;
        sk_bzero(&lf, sizeof(lf));
        lf.lfHeight = 9;
        SkTypeface* tf0 = SkCreateTypefaceFromLOGFONT(lf);
        lf.lfHeight = 12;
        SkTypeface* tf1 = SkCreateTypefaceFromLOGFONT(lf);
        // we assert that different sizes should not affect which face we get
        SkASSERT(tf0 == tf1);
        tf0->unref();
        tf1->unref();
#endif

    testFontProvider = sk_make_sp<TestFontProvider>(MakeResourceAsTypeface("fonts/GoogleSans-Regular.ttf", 0));

    fontCollection = sk_make_sp<SkFontCollection>();
  }

  ~ParagraphView() {
  }
 protected:
  bool onQuery(Sample::Event* evt) override {
    if (Sample::TitleQ(*evt)) {
      Sample::TitleR(evt, "Paragraph");
      return true;
    }
    return this->INHERITED::onQuery(evt);
  }

  void drawCode(SkCanvas* canvas, SkScalar w, SkScalar h) {

    SkPaint comment; comment.setColor(SK_ColorGRAY);
    SkPaint constant; constant.setColor(SK_ColorMAGENTA);
    SkPaint null; null.setColor(SK_ColorMAGENTA);
    SkPaint literal; literal.setColor(SK_ColorGREEN);
    SkPaint code; code.setColor(SK_ColorDKGRAY);
    SkPaint number; number.setColor(SK_ColorBLUE);
    SkPaint name; name.setColor(SK_ColorRED);

    SkTextStyle defaultStyle;
    defaultStyle.setBackgroundColor(SK_ColorWHITE);
    defaultStyle.setForegroundColor(code);
    defaultStyle.setFontFamily("monospace");
    defaultStyle.setFontSize(30);
    SkParagraphStyle paraStyle;
    paraStyle.setTextStyle(defaultStyle);

    std::string line01 = "// Create a raised button.\n";
    std::string line02 = "RaisedButton(\n";
    std::string line03 = "  child: const Text('BUTTON TITLE'),\n";
    std::string line04 = "  onPressed: () {\n";
    std::string line05 = "    // Perform some action\n";
    std::string line06 = "  }\n";
    std::string line07 = ");\n";
    std::string line08 = "\n";
    std::string line09 = "// Create a disabled button.\n";
    std::string line10 = "// Buttons are disabled when onPressed isn't\n";
    std::string line11 = "// specified or is null.\n";
    std::string line12 = "const RaisedButton(\n";
    std::string line13 = "  child: Text('BUTTON TITLE'),\n";
    std::string line14 = "  onPressed: null\n";
    std::string line15 = ");\n";
    std::string line16 = "\n";
    std::string line17 = "// Create a button with an icon and a\n";
    std::string line18 = "// title.\n";
    std::string line19 = "RaisedButton.icon(\n";
    std::string line20 = "  icon: const Icon(Icons.add, size: 18.0),\n";
    std::string line21 = "  label: const Text('BUTTON TITLE'),\n";
    std::string line22 = "  onPressed: () {\n";
    std::string line23 = "    // Perform some action\n";
    std::string line24 = "  },\n";
    std::string line25 = ");";

    std::vector<std::string> lines = {
        line01, line02, line03, line04, line05, line06, line07, line08, line09,
        line10, line11, line12, line13, line14, line15, line16, line17, line18, line19,
        line20, line21, line22, line23, line24, line25,
    };

    SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());

    //builder.PushStyle(style(comment));
    //builder.AddText("// Create a raised button.\n");
    //builder.Pop();

    builder.PushStyle(style(name));
    builder.AddText("RaisedButton");
    builder.Pop();
    builder.AddText("(\n");
    builder.AddText("  child: ");
    builder.PushStyle(style(constant));
    builder.AddText("const");
    builder.Pop();
    builder.AddText(" ");
    builder.PushStyle(style(name));
    builder.AddText("Text");
    builder.Pop();
    builder.AddText("(");
    builder.PushStyle(style(literal));
    builder.AddText("'BUTTON TITLE'");
    builder.Pop();
    builder.AddText("),\n");

    auto paragraph = builder.Build();
    paragraph->Layout(w - 20);

    paragraph->Paint(canvas, 20, 20);
  }

  SkTextStyle style(SkPaint paint) {
    SkTextStyle style;
    paint.setAntiAlias(true);
    style.setForegroundColor(paint);
    style.setFontFamily("monospace");
    style.setFontSize(30);

    return style;
  }

  void drawTest(SkCanvas* canvas, SkScalar w, SkScalar h, SkColor fg, SkColor bg) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->clipRect(SkRect::MakeWH(w, h));
    canvas->drawColor(SK_ColorWHITE);

    SkScalar margin = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(fg);

    SkTextStyle style;
    style.setBackgroundColor(SK_ColorBLUE);
    style.setForegroundColor(paint);
    SkParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);

    for (auto i = 3; i < 4; ++i) {
      paraStyle.getTextStyle().setFontSize(24 * i);
      SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());
      builder.AddText("Paragraph:");
      for (auto para : gParagraph) {
        SkTextStyle style;
        style.setBackgroundColor(bg);
        style.setForegroundColor(paint);
        style.setFontFamily(std::get<0>(para));
        SkFontStyle fontStyle(
            std::get<1>(para) ? SkFontStyle::Weight::kBold_Weight : SkFontStyle::Weight::kNormal_Weight,
            SkFontStyle::Width::kNormal_Width,
            std::get<2>(para) ? SkFontStyle::Slant::kItalic_Slant : SkFontStyle::Slant::kUpright_Slant);
        style.setFontStyle(fontStyle);
        style.setFontSize(std::get<3>(para) * i);
        style.setBackgroundColor(std::get<4>(para));
        SkPaint foreground;
        foreground.setColor(std::get<5>(para));
        style.setForegroundColor(foreground);
        if (std::get<6>(para)) {
          style.addShadow(SkTextShadow(SK_ColorBLACK, SkPoint::Make(5, 5), 2));
        }

        auto decoration = (i % 4);
        if (decoration == 3) { decoration = 4; }

        bool test = (SkTextDecoration)decoration != SkTextDecoration::kNone;
        std::string deco = std::to_string((int)decoration);
        if (test) {
          style.setDecoration((SkTextDecoration)decoration);
          style.setDecorationStyle(std::get<7>(para));
          style.setDecorationColor(std::get<5>(para));
        }
        builder.PushStyle(style);
        std::string name = " " +
            std::get<0>(para) +
            (std::get<1>(para) ? ", bold" : "") +
            (std::get<2>(para) ? ", italic" : "") + " " +
            std::to_string(std::get<3>(para) * i) +
            (std::get<4>(para) != bg ? ", background" : "") +
            (std::get<5>(para) != fg ? ", foreground" : "") +
            (std::get<6>(para) ? ", shadow" : "") +
            (test ? ", decorations " + deco : "") +
            ";";
        builder.AddText(name);
        builder.Pop();
      }

      auto paragraph = builder.Build();
      paragraph->Layout(w - margin);

      paragraph->Paint(canvas, margin, margin);

      canvas->translate(0, paragraph->GetHeight());
    }
  }

  void drawSimpleTest(SkCanvas* canvas, SkScalar w, SkScalar h,
                      SkTextDecoration decoration,
                      SkTextDecorationStyle decorationStyle
  ) {

    SkColor fg = SK_ColorDKGRAY;
    SkColor bg = SK_ColorWHITE;
    std::string ff = "sans-serif";
    SkScalar fs = 20;
    bool shadow = false;
    bool has_decoration = true;

    SkAutoCanvasRestore acr(canvas, true);

    canvas->clipRect(SkRect::MakeWH(w, h));
    canvas->drawColor(bg);

    SkScalar margin = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(fg);

    SkTextStyle style;
    style.setBackgroundColor(SK_ColorBLUE);
    style.setForegroundColor(paint);
    SkParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);

    paraStyle.getTextStyle().setFontSize(10);
    SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());

    style.setBackgroundColor(bg);
    style.setForegroundColor(paint);
    style.setFontFamily(ff);
    style.setFontStyle(SkFontStyle());
    style.setFontSize(fs);
    style.setBackgroundColor(bg);
    SkPaint foreground;
    foreground.setColor(fg);
    style.setForegroundColor(foreground);

    if (shadow) {
      style.addShadow(SkTextShadow(SK_ColorBLACK, SkPoint::Make(5, 5), 2));
    }

    if (has_decoration) {
      style.setDecoration(decoration);
      style.setDecorationStyle(decorationStyle);
      style.setDecorationColor(SK_ColorBLACK);
    }
    builder.PushStyle(style);
    builder.AddText(gText);
    builder.Pop();

    auto paragraph = builder.Build();
    paragraph->Layout(w - margin);

    paragraph->Paint(canvas, margin, margin);

    canvas->translate(0, paragraph->GetHeight() + margin);
  }

  void drawText(SkCanvas* canvas, SkScalar w, SkScalar h,
                std::vector<std::string>& text,
                SkColor fg = SK_ColorDKGRAY,
                SkColor bg = SK_ColorWHITE,
                std::string ff = "sans-serif",
                SkScalar fs = 24,
                size_t lineLimit = std::numeric_limits<size_t>::max(),
                const std::u16string& ellipsis = u"\u2026") {

    SkAutoCanvasRestore acr(canvas, true);

    canvas->clipRect(SkRect::MakeWH(w, h));
    canvas->drawColor(bg);

    SkScalar margin = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(fg);

    SkTextStyle style;
    style.setBackgroundColor(SK_ColorBLUE);
    style.setForegroundColor(paint);
    style.setFontFamily(ff);
    style.setFontSize(fs);
    SkParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setMaxLines(lineLimit);

    paraStyle.setEllipsis(ellipsis);
    paraStyle.getTextStyle().setFontSize(10);
    fontCollection->SetTestFontManager(testFontProvider);
    SkParagraphBuilder builder(paraStyle, fontCollection);

    SkPaint foreground;
    foreground.setColor(fg);
    style.setForegroundColor(foreground);
    style.setBackgroundColor(bg);

    for (auto& part : text) {
      builder.PushStyle(style);
      builder.AddText(part);
      builder.Pop();
    }

    auto paragraph = builder.Build();
    paragraph->Layout(180);

    paragraph->Paint(canvas, margin, margin);

    canvas->translate(0, paragraph->GetHeight() + margin);
  }

  void drawLine(SkCanvas* canvas, SkScalar w, SkScalar h,
                const std::string& text,
                SkTextAlign align) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->clipRect(SkRect::MakeWH(w, h));
    canvas->drawColor(SK_ColorWHITE);

    SkScalar margin = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);

    SkTextStyle style;
    style.setBackgroundColor(SK_ColorLTGRAY);
    style.setForegroundColor(paint);
    style.setFontFamily("Arial");
    style.setFontSize(30);
    SkParagraphStyle paraStyle;
    paraStyle.setTextStyle(style);
    paraStyle.setTextAlign(align);

    SkParagraphBuilder builder(paraStyle, sk_make_sp<SkFontCollection>());
    builder.AddText(text);

    auto paragraph = builder.Build();
    paragraph->Layout(w - margin * 2);

    paragraph->Paint(canvas, margin, margin);

    canvas->translate(0, paragraph->GetHeight() + margin);

  }

  void onDrawContent(SkCanvas* canvas) override {
    drawTest(canvas, this->width(), this->height(), SK_ColorRED, SK_ColorWHITE);
    /*
    SkScalar height = this->height() / 5;
    drawSimpleTest(canvas, width(), height, SkTextDecoration::kOverline, SkTextDecorationStyle::kSolid);
    canvas->translate(0, height);
    drawSimpleTest(canvas, width(), height, SkTextDecoration::kUnderline, SkTextDecorationStyle::kWavy);
    canvas->translate(0, height);
    drawSimpleTest(canvas, width(), height, SkTextDecoration::kLineThrough, SkTextDecorationStyle::kWavy);
    canvas->translate(0, height);
    drawSimpleTest(canvas, width(), height, SkTextDecoration::kOverline, SkTextDecorationStyle::kDouble);
    canvas->translate(0, height);
    drawSimpleTest(canvas, width(), height, SkTextDecoration::kOverline, SkTextDecorationStyle::kWavy);
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
    std::vector<std::string> cupertino = { "google_logogoogle_gsuper_g_logo" };
    std::vector<std::string>  text = {
        "My neighbor came over to say,\n"
        "Although not in a neighborly way,\n\n"
        "That he'd knock me around,\n\n\n"
        "If I didn't stop the sound,\n\n\n\n"
        "Of the classical music I play."
    };
    std::vector<std::string> code = {
        "// Create a flat button.\n",
        "FlatButton(\n",
        "  child: const Text('BUTTON TITLE'),\n",
        "  onPressed: () {\n",
        "    // Perform some action\n",
        "  }\n",
        ");"
        "\n\n",
        "// Create a disabled button.\n",
        "// Buttons are disabled when onPressed isn't\n",
        "// specified or is null.\n",
        "const FlatButton(\n  child: ",
        "Text('BUTTON TITLE'),\n",
        "  ",
        "onPressed: null\n",
        ");"
    };
    std::vector<std::string> very_long = { "A very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very very long text" };
    //drawText(canvas, this->width(), this->height(), cupertino, SK_ColorBLACK, SK_ColorWHITE, "Google Sans", 16);
    //drawText(canvas, this->width(), this->height(), very_long, SK_ColorBLACK, SK_ColorWHITE, "monospace", 20, 4, u"\u2026");
    /*
    SkScalar height = this->height() / 4;
    std::string line = "Hesitation is always easy rarely useful.";
    std::string str(gText);

    drawLine(canvas, this->width(), height, str, SkTextAlign::left);
    canvas->translate(0, height);
    drawLine(canvas, this->width(), height, str, SkTextAlign::right);
    canvas->translate(0, height);
    drawLine(canvas, this->width(), height, str, SkTextAlign::center);
    canvas->translate(0, height);
    drawLine(canvas, this->width(), height, str, SkTextAlign::justify);
    */
    //drawCode(canvas, width(), height());
  }

 private:
  typedef Sample INHERITED;

  sk_sp<TestFontProvider> testFontProvider;
  sk_sp<SkFontCollection> fontCollection;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ParagraphView(); )