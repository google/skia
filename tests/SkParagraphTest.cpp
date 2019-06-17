// Copyright 2019 Google LLC.
#include <sstream>
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkShaperJSONWriter.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

using namespace skia::textlayout;
namespace {

bool equal(SkSpan<const char> a, const char* b) {
    return std::strncmp(b, a.data(), a.size()) == 0;
}
class TestFontCollection : public FontCollection {
public:
    TestFontCollection()
            : fFontsFound(false)
            , fResolvedFonts(0)
            , fResourceDir(GetResourcePath("fonts").c_str())
            , fFontProvider(sk_make_sp<TypefaceFontProvider>()) {
        std::vector<SkString> fonts;
        SkOSFile::Iter iter(fResourceDir.c_str());
        SkString path;
        while (iter.next(&path)) {
            if (path.endsWith("Roboto-Italic.ttf")) {
                fFontsFound = true;
            }
            fonts.emplace_back(path);
        }

        if (!fFontsFound) {
            return;
        }
        // Only register fonts if we have to
        for (auto& font : fonts) {
            SkString file_path;
            file_path.printf("%s/%s", fResourceDir.c_str(), font.c_str());
            fFontProvider->registerTypeface(SkTypeface::MakeFromFile(file_path.c_str()));
        }

        this->setTestFontManager(std::move(fFontProvider));
        this->disableFontFallback();

        if (!fFontsFound) SkDebugf("Fonts not found, skipping all the tests\n");
    }

    ~TestFontCollection() = default;

    size_t resolvedFonts() const { return fResolvedFonts; }

    // TODO: temp solution until we check in fonts
    bool fontsFound() const { return fFontsFound; }

private:
    bool fFontsFound;
    size_t fResolvedFonts;
    std::string fResourceDir;
    sk_sp<TypefaceFontProvider> fFontProvider;
};
}  // namespace

DEF_TEST(SkParagraph_SimpleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "Hello World Text Dialog";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));

    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorBLACK);
                            ++index;
                            return true;
                        });
    }
}

DEF_TEST(SkParagraph_SimpleRedParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "I am RED";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);  // paragraph style does not count
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));

    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                            REPORTER_ASSERT(reporter, index == 0);
                            REPORTER_ASSERT(reporter, style.getColor() == SK_ColorRED);
                            ++index;
                            return true;
                        });
    }
}

DEF_TEST(SkParagraph_RainbowParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text1 = "Red Roboto";
    const char* text2 = "big Greeen Default";
    const char* text3 = "Defcolor Homemade Apple";
    const char* text4 = "Small Blue Roboto";
    const char* text5 =
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not";
    const char* text45 =
            "Small Blue Roboto"
            "Continue Last Style With lots of words to check if it overlaps "
            "properly or not";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(1);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style1;
    text_style1.setFontFamilies({SkString("Roboto")});

    text_style1.setColor(SK_ColorRED);
    builder.pushStyle(text_style1);
    builder.addText(text1);

    TextStyle text_style2;
    text_style2.setFontFamilies({SkString("Roboto")});
    text_style2.setFontSize(50);
    text_style2.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                         SkFontStyle::kUpright_Slant));
    text_style2.setLetterSpacing(10);
    text_style2.setDecorationColor(SK_ColorBLACK);
    text_style2.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style2.setWordSpacing(30);
    text_style2.setColor(SK_ColorGREEN);
    builder.pushStyle(text_style2);
    builder.addText(text2);

    TextStyle text_style3;
    text_style3.setFontFamilies({SkString("Homemade Apple")});
    builder.pushStyle(text_style3);
    builder.addText(text3);

    TextStyle text_style4;
    text_style4.setFontFamilies({SkString("Roboto")});
    text_style4.setFontSize(14);
    text_style4.setDecorationColor(SK_ColorBLACK);
    text_style4.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style4.setColor(SK_ColorBLUE);
    builder.pushStyle(text_style4);
    builder.addText(text4);

    builder.addText(text5);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(VeryLongCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 4);
    REPORTER_ASSERT(reporter, impl->styles().size() == 4);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    // Some of the formatting lazily done on paint
    impl->formatLines(VeryLongCanvasWidth);

    size_t index = 0;
    impl->lines()[0].scanStyles(StyleType::kAllAttributes,
                                [&](SkSpan<const char> text, TextStyle style, SkScalar) {
                                    switch (index) {
                                        case 0:
                                            REPORTER_ASSERT(reporter, style.equals(text_style1));
                                            REPORTER_ASSERT(reporter, equal(text, text1));
                                            break;
                                        case 1:
                                            REPORTER_ASSERT(reporter, style.equals(text_style2));
                                            REPORTER_ASSERT(reporter, equal(text, text2));
                                            break;
                                        case 2:
                                            REPORTER_ASSERT(reporter, style.equals(text_style3));
                                            REPORTER_ASSERT(reporter, equal(text, text3));
                                            break;
                                        case 3:
                                            REPORTER_ASSERT(reporter, style.equals(text_style4));
                                            REPORTER_ASSERT(reporter, equal(text, text45));
                                            break;
                                        default:
                                            REPORTER_ASSERT(reporter, false);
                                            break;
                                    }
                                    ++index;
                                    return true;
                                });
    REPORTER_ASSERT(reporter, index == 4);
}

DEF_TEST(SkParagraph_DefaultStyleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "No TextStyle! Uh Oh!";

    ParagraphStyle paragraph_style;
    TextStyle defaultStyle;
    defaultStyle.setFontFamilies({SkString("Roboto")});
    paragraph_style.setTextStyle(defaultStyle);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(VeryLongCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    impl->formatLines(VeryLongCanvasWidth);

    size_t index = 0;
    impl->lines()[0].scanStyles(
            StyleType::kAllAttributes, [&](SkSpan<const char> text1, TextStyle style, SkScalar) {
                REPORTER_ASSERT(reporter, style.equals(paragraph_style.getTextStyle()));
                REPORTER_ASSERT(reporter, equal(text1, text));
                ++index;
                return true;
            });
    REPORTER_ASSERT(reporter, index == 1);
}

DEF_TEST(SkParagraph_BoldParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "This is Red max bold text!";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(60);
    text_style.setLetterSpacing(0);
    text_style.setFontStyle(SkFontStyle(SkFontStyle::kBlack_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kUpright_Slant));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(VeryLongCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    impl->formatLines(VeryLongCanvasWidth);

    size_t index = 0;
    impl->lines()[0].scanStyles(StyleType::kAllAttributes,
                                [&](SkSpan<const char> text1, TextStyle style, SkScalar) {
                                    REPORTER_ASSERT(reporter, style.equals(text_style));
                                    REPORTER_ASSERT(reporter, equal(text1, text));
                                    ++index;
                                    return true;
                                });
    REPORTER_ASSERT(reporter, index == 1);
}

DEF_TEST(SkParagraph_LeftAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    // Apparently, Minikin records start from the base line (24)
    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());

    // Tests for GetGlyphPositionAtCoordinate()
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(0, 0).position == 0);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 1).position == 0);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 35).position == 68);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(1, 70).position == 134);
    REPORTER_ASSERT(reporter, impl->getGlyphPositionAtCoordinate(2000, 35).position == 134);
}

DEF_TEST(SkParagraph_RightAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kRight);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
    // Minikin has two records for each due to 'ghost' trailing whitespace run, SkParagraph - 1
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    // Apparently, Minikin records start from the base line (24)
    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - line.offset().fX - line.width();
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[13]), 0, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

DEF_TEST(SkParagraph_CenterAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kCenter);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(1);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
    // Minikin has two records for each due to 'ghost' trailing whitespace run, SkParagraph - 1
    REPORTER_ASSERT(reporter, impl->lines().size() == paragraph_style.getMaxLines());

    // Apparently, Minikin records start from the base line (24)
    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 10;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[13].offset().fY, expected_y, epsilon));

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - (line.offset().fX * 2 + line.width());
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[13]), 0, epsilon));

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

DEF_TEST(SkParagraph_JustifyAlignParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
            "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum. "
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
            "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
            "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
            "velit esse cillum dolore eu fugiat.";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));

    double expected_y = 0;
    double epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].baseline(), 24.121f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[0].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[1].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[2].offset().fY, expected_y, epsilon));
    expected_y += 30;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[3].offset().fY, expected_y, epsilon));
    expected_y += 30 * 9;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[12].offset().fY, expected_y, epsilon));

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - (line.offset().fX + line.width());
    };

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[0]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[1]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[2]), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(impl->lines()[3]), 0, epsilon));
    REPORTER_ASSERT(reporter, calculate(impl->lines()[12]) > 0);

    REPORTER_ASSERT(reporter,
                    paragraph_style.getTextAlign() == impl->paragraphStyle().getTextAlign());
}

DEF_TEST(SkParagraph_JustifyRTL, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "אאא בּבּבּבּ אאאא בּבּ אאא בּבּבּ אאאאא בּבּבּבּ אאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּ אאאבּבּבּבּבּבּאאאאא בּבּבּבּבּבּאאאאאבּבּבּבּבּבּ אאאאא בּבּבּבּבּ "
            "אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ אאאאא בּבּבּבּבּבּ";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(26);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    auto calculate = [](const TextLine& line) -> SkScalar {
        return TestCanvasWidth - 100 - (line.offset().fX + line.width());
    };

    SkScalar epsilon = 0.1f;
    for (auto& line : impl->lines()) {
        if (&line == impl->lines().end() - 1) {
            REPORTER_ASSERT(reporter, calculate(line) > epsilon);
        } else {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(calculate(line), 0, epsilon));
        }
    }

    // Just make sure the the text is actually RTL
    for (auto& run : impl->runs()) {
        REPORTER_ASSERT(reporter, !run.leftToRight());
    }
}

DEF_TEST(SkParagraph_DecorationsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(26);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(5);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(2);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecoration((TextDecoration)(
            TextDecoration::kUnderline | TextDecoration::kOverline | TextDecoration::kLineThrough));
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(2.0);
    builder.pushStyle(text_style);
    builder.addText("This text should be");

    text_style.setDecorationStyle(TextDecorationStyle::kDouble);
    text_style.setDecorationColor(SK_ColorBLUE);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(" decorated even when");

    text_style.setDecorationStyle(TextDecorationStyle::kDotted);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(" wrapped around to");

    text_style.setDecorationStyle(TextDecorationStyle::kDashed);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationThicknessMultiplier(3.0);
    builder.pushStyle(text_style);
    builder.addText(" the next line.");

    text_style.setDecorationStyle(TextDecorationStyle::kWavy);
    text_style.setDecorationColor(SK_ColorRED);
    text_style.setDecorationThicknessMultiplier(1.0);
    builder.pushStyle(text_style);
    builder.addText(" Otherwise, bad things happen.");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(
            StyleType::kDecorations,
            [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                auto decoration = (TextDecoration)(TextDecoration::kUnderline |
                                                   TextDecoration::kOverline |
                                                   TextDecoration::kLineThrough);
                REPORTER_ASSERT(reporter, style.getDecoration() == decoration);
                switch (index) {
                    case 0:
                        REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                          TextDecorationStyle::kSolid);
                        REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                        REPORTER_ASSERT(reporter,
                                        style.getDecorationThicknessMultiplier() == 2.0);
                        break;
                    case 1:  // The style appears on 2 lines so it has 2 pieces
                        REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                          TextDecorationStyle::kDouble);
                        REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLUE);
                        REPORTER_ASSERT(reporter,
                                        style.getDecorationThicknessMultiplier() == 1.0);
                        break;
                    case 2:
                        REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                          TextDecorationStyle::kDotted);
                        REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                        REPORTER_ASSERT(reporter,
                                        style.getDecorationThicknessMultiplier() == 1.0);
                        break;
                    case 3:
                    case 4:
                        REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                          TextDecorationStyle::kDashed);
                        REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorBLACK);
                        REPORTER_ASSERT(reporter,
                                        style.getDecorationThicknessMultiplier() == 3.0);
                        break;
                    case 5:
                        REPORTER_ASSERT(reporter, style.getDecorationStyle() ==
                                                          TextDecorationStyle::kWavy);
                        REPORTER_ASSERT(reporter, style.getDecorationColor() == SK_ColorRED);
                        REPORTER_ASSERT(reporter,
                                        style.getDecorationThicknessMultiplier() == 1.0);
                        break;
                    default:
                        REPORTER_ASSERT(reporter, false);
                        break;
                }
                ++index;
                return true;
            });
    }
}

DEF_TEST(SkParagraph_ItalicsParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(10);
    text_style.setColor(SK_ColorRED);
    builder.pushStyle(text_style);
    builder.addText("No italic ");

    text_style.setFontStyle(SkFontStyle(SkFontStyle::kNormal_Weight, SkFontStyle::kNormal_Width,
                                        SkFontStyle::kItalic_Slant));
    builder.pushStyle(text_style);
    builder.addText("Yes Italic ");
    builder.pop();
    builder.addText("No Italic again.");

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->runs().size() == 3);
    REPORTER_ASSERT(reporter, impl->styles().size() == 3);
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    auto& line = impl->lines()[0];
    size_t index = 0;
    line.scanStyles(
        StyleType::kForeground,
        [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
            switch (index) {
                case 0:
                    REPORTER_ASSERT(
                            reporter,
                            style.getFontStyle().slant() == SkFontStyle::kUpright_Slant);
                    break;
                case 1:
                    REPORTER_ASSERT(reporter,
                                    style.getFontStyle().slant() == SkFontStyle::kItalic_Slant);
                    break;
                case 2:
                    REPORTER_ASSERT(
                            reporter,
                            style.getFontStyle().slant() == SkFontStyle::kUpright_Slant);
                    break;
                default:
                    REPORTER_ASSERT(reporter, false);
                    break;
            }
            ++index;
            return true;
        });
}

DEF_TEST(SkParagraph_ChineseParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "左線読設重説切後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得庭"
            "際輝求佐抗蒼提夜合逃表。注統天言件自謙雅載報紙喪。作画稿愛器灯女書利変探"
            "訃第金線朝開化建。子戦年帝励害表月幕株漠新期刊人秘。図的海力生禁挙保天戦"
            "聞条年所在口。";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    auto decoration = (TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline |
                                       TextDecoration::kLineThrough);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setFontSize(35);
    text_style.setColor(SK_ColorBLACK);
    text_style.setLetterSpacing(2);
    text_style.setHeight(1);
    text_style.setDecoration(decoration);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
}

DEF_TEST(SkParagraph_ArabicParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "من أسر وإعلان الخاصّة وهولندا،, عل قائمة الضغوط بالمطالبة تلك. الصفحة "
            "بمباركة التقليدية قام عن. تصفح";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    auto decoration = (TextDecoration)(TextDecoration::kUnderline | TextDecoration::kOverline |
                                       TextDecoration::kLineThrough);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Katibeh")});
    text_style.setFontSize(35);
    text_style.setColor(SK_ColorBLACK);
    // TODO: turn off some font features for letter spacing (waiting on SkShaper)
    // text_style.setLetterSpacing(2);
    text_style.setDecoration(decoration);
    text_style.setDecorationColor(SK_ColorBLACK);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 2);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
}

DEF_TEST(SkParagraph_GetGlyphPositionAtCoordinateParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    // Tests for getGlyphPositionAtCoordinate()
    // NOTE: resulting values can be a few off from their respective positions in
    // the original text because the final trailing whitespaces are sometimes not
    // drawn (namely, when using "justify" alignment) and therefore are not active
    // glyphs.
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(-10000, -10000).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(-1, -1).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(0, 0).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(3, 3).position == 0);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(35, 1).position == 1);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(300, 2).position == 10);  // !!! 11
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.2f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(302, 2.6f).position == 11);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(301, 2.1f).position == 11);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(100000, 20).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(450, 20).position == 16);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(100000, 90).position == 36);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(-100000, 90).position == 18);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(20, -80).position == 0);  // !!! 1
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 90).position == 18);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 170).position == 36);
    REPORTER_ASSERT(reporter,
                    paragraph->getGlyphPositionAtCoordinate(10000, 180).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(70, 180).position == 56);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(1, 270).position == 72);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(35, 90).position == 19);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(10000, 10000).position == 77);
    REPORTER_ASSERT(reporter, paragraph->getGlyphPositionAtCoordinate(85, 10000).position == 75);
}

DEF_TEST(SkParagraph_GetRectsForRangeParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "12345,  \"67890\" 12345 67890 12345 67890 12345 67890 12345 67890 12345 "
            "67890 12345";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 28.417f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 56.835f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 177.97f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 177.97f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 507.031f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(30, 100, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 4);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 211.375f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
        // This number does not match: 463.617  & 451.171
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 451.171f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.top(), 236.406f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.right(), 142.089f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[3].rect.bottom(), 295, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 450.1875f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        // This number does not match 519.472 & 507.031
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 507.031f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeTight, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({ SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 16.898f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 66.899f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 74, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 528.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 172.199f, epsilon));
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingMiddle, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setHeight(1.3f);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingMiddle;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;
    // 16 glyphs per line 160/16 = 10 lines
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 8.60f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 16.90f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 93.60f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 66.90f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 8.60f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 264.10f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 93.60f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 264.10f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 8.60f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 528.20f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 93.60f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 9);
        SkScalar offsetY = 104.60f;
        for (auto& box : result) {
            if (&box != &result.back()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.right(), 528.20f, epsilon));
            }
            if (&box != &result.front()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.left(), 0, epsilon));
            }
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.top(), offsetY, epsilon));
            offsetY = box.rect.bottom() + 11;
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 97.20f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 104.60f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 197.20f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 189.60f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingTop, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setWordSpacing(0);
    textStyle.setLetterSpacing(0);
    textStyle.setHeight(1.3f);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingTop;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 16.898f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 91.199f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 66.899f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 91.199f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 528.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 91.199f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 9);
        SkScalar level = 96;
        for (auto& box : result) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.top(), level, epsilon));
            level += 96;
            if (&box != &result.back()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.right(), 528.199f, epsilon));
            }
            if (&box != &result.front()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.left(), 0, epsilon));
            }
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 97.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 96, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 197.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 187.199f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeIncludeLineSpacingBottom, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)("
            "　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)(　´･‿･｀)";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Noto Sans CJK JP")});
    textStyle.setFontSize(50);
    textStyle.setWordSpacing(0);
    textStyle.setLetterSpacing(0);
    textStyle.setHeight(1.3f);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    RectHeightStyle heightStyle = RectHeightStyle::kIncludeLineSpacingBottom;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 17.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 16.898f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 8, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 66.899f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 17.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 96, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(8, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 2);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 264.099f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 528.199f, epsilon));
        // It seems that Minikin does not take in account like breaks, but we do.
        // SkParagraph returns 528.199 instead
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[1].rect.right(), 172.199f, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(30, 150, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 9);
        SkScalar level = 17.199f + 96;
        for (auto& box : result) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.top(), level, epsilon));
            level += 96;
            if (&box != &result.back()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.right(), 528.199f, epsilon));
            }
            if (&box != &result.front()) {
                REPORTER_ASSERT(reporter, SkScalarNearlyEqual(box.rect.left(), 0, epsilon));
            }
        }
    }
    {
        auto result = paragraph->getRectsForRange(19, 22, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 97.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 113.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 197.199f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 192, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeIncludeCombiningCharacter, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "ดีสวัสดีชาวโลกที่น่ารัก";
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(1);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->formatLines(TestCanvasWidth - 100);

    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    RectHeightStyle heightStyle = RectHeightStyle::kTight;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto first = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(1, 2, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(0, 2, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 1 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter,
                        last[0].rect.fLeft = SkTMin(first[0].rect.fLeft, second[0].rect.fLeft));
        REPORTER_ASSERT(reporter,
                        last[0].rect.fRight = SkTMax(first[0].rect.fRight, second[0].rect.fRight));
    }
    {
        auto first = paragraph->getRectsForRange(3, 4, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(3, 5, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 1 && second.size() == 1 && last.size() == 1);
        REPORTER_ASSERT(reporter,
                        last[0].rect.fLeft = SkTMin(first[0].rect.fLeft, second[0].rect.fLeft));
        REPORTER_ASSERT(reporter,
                        last[0].rect.fRight = SkTMax(first[0].rect.fRight, second[0].rect.fRight));
    }
    {
        auto first = paragraph->getRectsForRange(14, 15, heightStyle, widthStyle);
        auto second = paragraph->getRectsForRange(15, 16, heightStyle, widthStyle);
        auto third = paragraph->getRectsForRange(16, 17, heightStyle, widthStyle);
        auto last = paragraph->getRectsForRange(14, 17, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, first.size() == 1 && second.size() == 1 && third.size() == 1 &&
                                          last.size() == 1);
        REPORTER_ASSERT(reporter,
                        last[0].rect.fLeft = SkTMin(first[0].rect.fLeft, third[0].rect.fLeft));
        REPORTER_ASSERT(reporter,
                        last[0].rect.fRight = SkTMax(first[0].rect.fRight, third[0].rect.fRight));
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeCenterParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    // Minikin uses a hard coded list of unicode characters that he treats as invisible - as spaces.
    // It's absolutely wrong - invisibility is a glyph attribute, not character/grapheme.
    // Any attempt to substitute one for another leads to errors
    // (for instance, some fonts can use these hard coded characters for something that is visible)
    const char* text =
            "01234    ";  //"01234  　 ";   // includes ideographic space and english space.

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(0);
    textStyle.setWordSpacing(0);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(550);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 260.791f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        auto result = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeCenterParagraphNewlineCentered, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "01234\n";

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(0);
    textStyle.setWordSpacing(0);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(550);

    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;

    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }

    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }

    {
        // Minikin has [6:7] not empty but I cannot imagine how and why
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.406f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
}

DEF_TEST(SkParagraph_GetRectsForRangeCenterMultiLineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "01234    \n0123          ";
    // "01234  　 \n0123　        ";  // includes ideographic space and english space.

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kCenter);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(50);
    textStyle.setLetterSpacing(0);
    textStyle.setWordSpacing(0);
    textStyle.setHeight(1);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    builder.pushStyle(textStyle);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(550);

    REPORTER_ASSERT(reporter, impl->lines().size() == 2);

    RectHeightStyle heightStyle = RectHeightStyle::kMax;
    RectWidthStyle widthStyle = RectWidthStyle::kTight;
    SkScalar epsilon = 0.01f;
    {
        auto result = paragraph->getRectsForRange(0, 0, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
    {
        auto result = paragraph->getRectsForRange(0, 1, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 203.955f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 232.373f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(2, 4, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 260.791f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(4, 5, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(4, 6, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 317.626f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(5, 6, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 0.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 346.044f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 59, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(10, 12, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 218.164f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 275, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
    }
    {
        // Minikin counts all spaces but they really don't appear on the screen
        // and the text even centered without them
        auto result = paragraph->getRectsForRange(14, 18, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.left(), 331.835f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.top(), 59.40625f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.right(), 331.835f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[0].rect.bottom(), 118, epsilon));
    }
    {
        auto result = paragraph->getRectsForRange(21, 21, heightStyle, widthStyle);
        REPORTER_ASSERT(reporter, result.empty());
    }
}

DEF_TEST(SkParagraph_GetWordBoundaries, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextAlign(TextAlign::kLeft);
    paragraphStyle.setMaxLines(10);
    paragraphStyle.turnHintingOff();
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(52);
    textStyle.setLetterSpacing(1.19039f);
    textStyle.setWordSpacing(5);
    textStyle.setHeight(1.5);
    textStyle.setColor(SK_ColorBLACK);
    textStyle.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
                                       SkFontStyle::kUpright_Slant));

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    builder.pushStyle(textStyle);
    builder.addText(
            "12345  67890 12345 67890 12345 67890 12345 67890 12345 67890 12345 67890 12345");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(0) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(1) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(2) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(3) == SkRange<size_t>(0, 5));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(4) == SkRange<size_t>(0, 5));

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(5) == SkRange<size_t>(5, 7));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(6) == SkRange<size_t>(5, 7));

    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(7) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(8) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(9) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(10) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(11) == SkRange<size_t>(7, 12));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(12) == SkRange<size_t>(12, 13));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(13) == SkRange<size_t>(13, 18));
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(30) == SkRange<size_t>(30, 31));

    auto len = static_cast<ParagraphImpl*>(paragraph.get())->text().size();
    REPORTER_ASSERT(reporter, paragraph->getWordBoundary(len - 1) == SkRange<size_t>(len - 5, len));
}

DEF_TEST(SkParagraph_SpacingParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(20);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(10);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(20);
    builder.pushStyle(text_style);
    builder.addText("H");
    builder.pop();

    text_style.setLetterSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("|");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(20);
    builder.pushStyle(text_style);
    builder.addText("H ");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    size_t index = 0;
    impl->lines().begin()->scanStyles(StyleType::kLetterSpacing,
                                      [&index](SkSpan<const char> text, TextStyle style, SkScalar) {
                                          ++index;
                                          return true;
                                      });
    REPORTER_ASSERT(reporter, index == 4);
    index = 0;
    impl->lines().begin()->scanStyles(StyleType::kWordSpacing,
                                      [&index](SkSpan<const char> text, TextStyle style, SkScalar) {
                                          ++index;
                                          return true;
                                      });
    REPORTER_ASSERT(reporter, index == 4);
}

DEF_TEST(SkParagraph_LongWordParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "A "
            "veryverylongwordtoseewherethiswillwraporifitwillatallandifitdoesthenthat"
            "wouldbeagoodthingbecausethebreakingisworking.";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorRED);
    text_style.setFontSize(31);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / 2);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->text().size() == std::string{text}.length());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles()[0].style().equals(text_style));
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    REPORTER_ASSERT(reporter, impl->lines()[0].width() > TestCanvasWidth / 2 - 20);
    REPORTER_ASSERT(reporter, impl->lines()[1].width() > TestCanvasWidth / 2 - 20);
    REPORTER_ASSERT(reporter, impl->lines()[2].width() > TestCanvasWidth / 2 - 20);
}

DEF_TEST(SkParagraph_KernScaleParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    float scale = 3.0f;
    ParagraphStyle paragraph_style;
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);
    TextStyle text_style;
    text_style.setFontFamilies({SkString("Droid Serif")});
    text_style.setFontSize(100 / scale);
    text_style.setWordSpacing(0);
    text_style.setLetterSpacing(0);
    text_style.setHeight(1);
    text_style.setColor(SK_ColorBLACK);

    builder.pushStyle(text_style);
    builder.addText("AV00\nVA00\nA0V0");
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth / scale);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth);

    // First and second lines must have the same width, the third one must be bigger
    SkScalar epsilon = 0.01f;
    REPORTER_ASSERT(reporter, impl->lines().size() == 3);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].width(), 80.58f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[1].width(), 80.58f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[2].width(), 83.25f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].height(), 39.00f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[1].height(), 39.00f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[2].height(), 39.00f, epsilon));
}

DEF_TEST(SkParagraph_NewlineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "line1\nline2 test1 test2 test3 test4 test5 test6 test7\nline3\n\nline4 "
            "test1 test2 test3 test4";
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
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
    paragraph->layout(TestCanvasWidth - 300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Minikin does not count empty lines but SkParagraph does
    REPORTER_ASSERT(reporter, impl->lines().size() == 7);

    REPORTER_ASSERT(reporter, impl->lines()[0].offset().fY == 0);
    REPORTER_ASSERT(reporter, impl->lines()[1].offset().fY == 70);
    REPORTER_ASSERT(reporter, impl->lines()[2].offset().fY == 140);
    REPORTER_ASSERT(reporter, impl->lines()[3].offset().fY == 210);
    REPORTER_ASSERT(reporter, impl->lines()[4].offset().fY == 280);  // Empty line
    REPORTER_ASSERT(reporter, impl->lines()[5].offset().fY == 350);
    REPORTER_ASSERT(reporter, impl->lines()[6].offset().fY == 420);

    SkScalar epsilon = 0.1f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[0].width(), 130.31f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[1].width(), 586.64f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[2].width(), 593.49f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[3].width(), 130.31f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(impl->lines()[4].width(), 0, epsilon));  // Empty line
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[5].width(), 586.64f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->lines()[6].width(), 137.16f, epsilon));

    REPORTER_ASSERT(reporter, impl->lines()[0].shift() == 0);
}

DEF_TEST(SkParagraph_EmojiParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "😀😃😄😁😆😅😂🤣☺😇🙂😍😡😟😢😻👽💩👍👎🙏👌👋👄👁👦👼👨‍🚀👨‍🚒🙋‍♂️👳👨‍👨‍👧"
            "‍"
            "👧"
            "💼👡👠☂🐶🐰🐻🐼🐷🐒🐵🐔🐧🐦🐋🐟🐡🕸🐌🐴🐊🐄🐪🐘🌸🌏🔥🌟🌚🌝"
            "💦"
            "💧"
            "❄🍕🍔🍟🥝🍱🕶🎩🏈⚽🚴‍♀️🎻🎼🎹🚨🚎🚐⚓🛳🚀🚁🏪🏢🖱⏰📱💾💉📉🛏"
            "🔑"
            "🔓"
            "📁🗓📊❤💯🚫🔻♠♣🕓❗🏳🏁🏳️‍🌈🇮🇹🇱🇷🇺🇸🇬🇧🇨🇳"
            "🇧"
            "🇴";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    text_style.setFontSize(50);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth);

    REPORTER_ASSERT(reporter, impl->lines().size() == 8);
    for (auto& line : impl->lines()) {
        if (&line != impl->lines().end() - 1) {
            REPORTER_ASSERT(reporter, line.width() == 998.25f);
        } else {
            REPORTER_ASSERT(reporter, line.width() < 998.25f);
        }
        REPORTER_ASSERT(reporter, line.height() == 59);
    }
}

DEF_TEST(SkParagraph_RepeatLayoutParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "Sentence to layout at diff widths to get diff line counts. short words "
            "short words short words short words short words short words short words "
            "short words short words short words short words short words short words "
            "end";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setFontSize(31);
    text_style.setLetterSpacing(0);
    text_style.setWordSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(300);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 12);

    paragraph->layout(600);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    REPORTER_ASSERT(reporter, impl->lines().size() == 6);
}

DEF_TEST(SkParagraph_Ellipsize, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "This is a very long sentence to test if the text will properly wrap "
            "around and go to the next line. Sometimes, short sentence. Longer "
            "sentences are okay too because they are nessecary. Very short. ";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(1);
    paragraph_style.setEllipsis(u"\u2026");
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Some of the formatting lazily done on paint
    impl->formatLines(TestCanvasWidth);

    // Check that the ellipsizer limited the text to one line and did not wrap to a second line.
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);

    auto& line = impl->lines()[0];
    REPORTER_ASSERT(reporter, line.ellipsis() != nullptr);
    size_t index = 0;
    line.scanRuns([&index, &line, reporter](Run* run, int32_t, size_t, SkRect, SkScalar, bool) {
        ++index;
        if (index == 2) {
            REPORTER_ASSERT(reporter, run->text() == line.ellipsis()->text());
        }
        return true;
    });
    REPORTER_ASSERT(reporter, index == 2);
}

DEF_TEST(SkParagraph_UnderlineShiftParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text1 = "fluttser ";
    const char* text2 = "mdje";
    const char* text3 = "fluttser mdje";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.setMaxLines(2);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1);
    text_style.setDecoration(TextDecoration::kUnderline);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text2);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    ParagraphBuilderImpl builder1(paragraph_style, fontCollection);
    text_style.setDecoration(TextDecoration::kNoDecoration);
    builder1.pushStyle(text_style);
    builder1.addText(text3);
    builder1.pop();

    auto paragraph1 = builder1.Build();
    paragraph1->layout(TestCanvasWidth);

    auto impl1 = static_cast<ParagraphImpl*>(paragraph1.get());

    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    REPORTER_ASSERT(reporter, impl1->lines().size() == 1);
    {
        auto& line = impl->lines()[0];
        size_t index = 0;
        line.scanStyles(
                StyleType::kDecorations,
                [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                    switch (index) {
                        case 0:
                            REPORTER_ASSERT(reporter,
                                            style.getDecoration() == TextDecoration::kNoDecoration);
                            break;
                        case 1:
                            REPORTER_ASSERT(reporter,
                                            style.getDecoration() == TextDecoration::kUnderline);
                            break;
                        default:
                            REPORTER_ASSERT(reporter, false);
                            break;
                    }
                    ++index;
                    return true;
                });
        REPORTER_ASSERT(reporter, index == 2);
    }
    {
        auto& line = impl1->lines()[0];
        size_t index = 0;
        line.scanStyles(StyleType::kDecorations,
                        [&index, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                            if (index == 0) {
                                REPORTER_ASSERT(reporter, style.getDecoration() ==
                                                                  TextDecoration::kNoDecoration);
                            } else {
                                REPORTER_ASSERT(reporter, false);
                            }
                            ++index;
                            return true;
                        });
        REPORTER_ASSERT(reporter, index == 1);
    }

    auto rect = paragraph->getRectsForRange(0, 12, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;
    auto rect1 = paragraph1->getRectsForRange(0, 12, RectHeightStyle::kMax, RectWidthStyle::kTight)
                         .front()
                         .rect;
    REPORTER_ASSERT(reporter, rect.fLeft == rect1.fLeft);
    REPORTER_ASSERT(reporter, rect.fRight == rect1.fRight);

    for (size_t i = 0; i < 12; ++i) {
        auto r =
                paragraph->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;
        auto r1 =
                paragraph1
                        ->getRectsForRange(i, i + 1, RectHeightStyle::kMax, RectWidthStyle::kTight)
                        .front()
                        .rect;

        REPORTER_ASSERT(reporter, r.fLeft == r1.fLeft);
        REPORTER_ASSERT(reporter, r.fRight == r1.fRight);
    }
}

DEF_TEST(SkParagraph_SimpleShadow, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "Hello World Text Dialog";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0));
    builder.pushStyle(text_style);
    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->styles().size() == 1);
    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(
                StyleType::kShadow,
                [&index, text_style, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                    REPORTER_ASSERT(reporter, index == 0 && style.equals(text_style));
                    ++index;
                    return true;
                });
    }
}

DEF_TEST(SkParagraph_ComplexShadow, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "Text Chunk ";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Roboto")});
    text_style.setColor(SK_ColorBLACK);
    text_style.addShadow(TextShadow(SK_ColorBLACK, SkPoint::Make(2.0f, 2.0f), 1.0f));
    builder.pushStyle(text_style);
    builder.addText(text);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(2.0f, 2.0f), 5.0f));
    text_style.addShadow(TextShadow(SK_ColorGREEN, SkPoint::Make(10.0f, -5.0f), 3.0f));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    builder.addText(text);

    text_style.addShadow(TextShadow(SK_ColorRED, SkPoint::Make(0.0f, 1.0f), 0.0f));
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    builder.addText(text);

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    size_t index = 0;
    for (auto& line : impl->lines()) {
        line.scanStyles(
                StyleType::kShadow,
                [&index, text_style, reporter](SkSpan<const char> text, TextStyle style, SkScalar) {
                    ++index;
                    switch (index) {
                        case 1:
                            REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                            break;
                        case 2:
                            REPORTER_ASSERT(reporter, style.getShadowNumber() == 3);
                            break;
                        case 3:
                            REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                            break;
                        case 4:
                            REPORTER_ASSERT(reporter, style.getShadowNumber() == 4);
                            REPORTER_ASSERT(reporter, style.equals(text_style));
                            break;
                        case 5:
                            REPORTER_ASSERT(reporter, style.getShadowNumber() == 1);
                            break;
                        default:
                            REPORTER_ASSERT(reporter, false);
                    }
                    return true;
                });
    }
}

DEF_TEST(SkParagraph_BaselineParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text =
            "左線読設Byg後碁給能上目秘使約。満毎冠行来昼本可必図将発確年。今属場育"
            "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維駆年策。立得";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    paragraph_style.setMaxLines(14);
    paragraph_style.setTextAlign(TextAlign::kJustify);
    paragraph_style.setHeight(1.5);
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Source Han Serif CN")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(55);
    text_style.setLetterSpacing(2);
    text_style.setDecorationStyle(TextDecorationStyle::kSolid);
    text_style.setDecorationColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth - 100);

    SkScalar epsilon = 0.01f;
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getIdeographicBaseline(), 79.035f, epsilon));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(paragraph->getAlphabeticBaseline(), 63.305f, epsilon));
}

DEF_TEST(SkParagraph_FontFallbackParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;

    const char* text1 = "Roboto 字典 ";
    const char* text2 = "Homemade Apple 字典";
    const char* text3 = "Chinese 字典";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({
        SkString("Not a real font"),
        SkString("Also a fake font"),
        SkString("So fake it is obvious"),
        SkString("Next one should be a real font..."),
        SkString("Roboto"),
        SkString("another fake one in between"),
        SkString("Homemade Apple"),
    });
    text_style.setColor(SK_ColorBLACK);
    builder.pushStyle(text_style);
    builder.addText(text1);

    text_style.setFontFamilies({
        SkString("Not a real font"),
        SkString("Also a fake font"),
        SkString("So fake it is obvious"),
        SkString("Homemade Apple"),
        SkString("Next one should be a real font..."),
        SkString("Roboto"),
        SkString("another fake one in between"),
        SkString("Noto Sans CJK JP"),
        SkString("Source Han Serif CN"),
    });
    builder.pushStyle(text_style);
    builder.addText(text2);

    text_style.setFontFamilies({
       SkString("Not a real font"),
       SkString("Also a fake font"),
       SkString("So fake it is obvious"),
       SkString("Homemade Apple"),
       SkString("Next one should be a real font..."),
       SkString("Roboto"),
       SkString("another fake one in between"),
       SkString("Source Han Serif CN"),
       SkString("Noto Sans CJK JP"),
    });
    builder.pushStyle(text_style);
    builder.addText(text3);

    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());

    // Font resolution in Skia produces 6 runs because 2 parts of "Roboto 字典 " have different
    // script (Minikin merges the first 2 into one because of unresolved) [Apple + Unresolved ]
    // [Apple + Noto] [Apple + Han]
    REPORTER_ASSERT(reporter, impl->runs().size() == 6);

    SkScalar epsilon = 0.01f;
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[0].advance().fX, 48.46f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[1].advance().fX, 15.90f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[2].advance().fX, 139.12f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[3].advance().fX, 27.99f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[4].advance().fX, 62.24f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(impl->runs()[5].advance().fX, 27.99f, epsilon));

    // When a different font is resolved, then the metrics are different.
    REPORTER_ASSERT(reporter, impl->runs()[1].ascent() != impl->runs()[3].ascent());
    REPORTER_ASSERT(reporter, impl->runs()[1].descent() != impl->runs()[3].descent());
    REPORTER_ASSERT(reporter, impl->runs()[3].ascent() != impl->runs()[5].ascent());
    REPORTER_ASSERT(reporter, impl->runs()[3].descent() != impl->runs()[5].descent());
    REPORTER_ASSERT(reporter, impl->runs()[1].ascent() != impl->runs()[5].ascent());
    REPORTER_ASSERT(reporter, impl->runs()[1].descent() != impl->runs()[5].descent());
}

DEF_TEST(SkParagraph_StrutParagraph1, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    // The chinese extra height should be absorbed by the strut.
    // const char* text = "01234満毎冠p来É本可\nabcd\n満毎É行p昼本可";
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({ SkString("BlahFake"), SkString("Ahem") });
    strut_style.setFontSize(50);
    strut_style.setHeight(1.8f);
    strut_style.setLeading(0.1f);
    strut_style.setForceStrutHeight(true);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    // text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
    // SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(0.5f);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    SkScalar epsilon = 0.001f;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 34.5f, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 84.5f, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 95, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 190, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 285, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 285, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 380, epsilon));
    }
}

DEF_TEST(SkParagraph_StrutParagraph2, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    // The chinese extra height should be absorbed by the strut.
    const char* text = "01234ABCDEFGH\nabcd\nABCDEFGH";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;

    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({ SkString("Ahem") });
    strut_style.setFontSize(50);
    strut_style.setHeight(1.6f);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    // text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
    // SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    SkScalar epsilon = 0.001f;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 24, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 74, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 80, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 160, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 240, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 320, epsilon));
    }
}

DEF_TEST(SkParagraph_StrutParagraph3, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    // The chinese extra height should be absorbed by the strut.
    // const char* text = "01234満毎p行来昼本可\nabcd\n満毎冠行来昼本可";
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({ SkString("Ahem") });
    strut_style.setFontSize(50);
    strut_style.setHeight(1.2f);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    // text_style.setFontStyle(SkFontStyle(SkFontStyle::kMedium_Weight, SkFontStyle::kNormal_Width,
    // SkFontStyle::kUpright_Slant));
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    SkScalar epsilon = 0.001f;
    {
        auto boxes = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.empty());
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 8, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 58, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 500, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 60, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 0, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 120, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 100, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 180, epsilon));
    }
    {
        auto boxes = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
        REPORTER_ASSERT(reporter, boxes.size() == 1);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.left(), 50, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.top(), 180, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.right(), 300, epsilon));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes[0].rect.bottom(), 240, epsilon));
    }
}

DEF_TEST(SkParagraph_StrutForceParagraph, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "01234満毎冠行来昼本可\nabcd\n満毎冠行来昼本可";

    ParagraphStyle paragraph_style;
    paragraph_style.setMaxLines(10);
    paragraph_style.setTextAlign(TextAlign::kLeft);
    paragraph_style.turnHintingOff();

    StrutStyle strut_style;
    strut_style.setStrutEnabled(true);
    strut_style.setFontFamilies({SkString("Ahem") });
    strut_style.setFontSize(50);
    strut_style.setHeight(1.5f);
    strut_style.setLeading(0.1f);
    strut_style.setForceStrutHeight(true);
    paragraph_style.setStrutStyle(strut_style);

    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Ahem")});
    text_style.setFontSize(50);
    text_style.setLetterSpacing(0);
    text_style.setColor(SK_ColorBLACK);
    text_style.setHeight(1);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(550);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    // Font is not resolved and the first line does not fit
    REPORTER_ASSERT(reporter, impl->lines().size() == 4);

    RectHeightStyle rect_height_style = RectHeightStyle::kTight;
    RectHeightStyle rect_height_max_style = RectHeightStyle::kMax;
    RectWidthStyle rect_width_style = RectWidthStyle::kTight;
    SkScalar epsilon = 0.001f;

    auto boxes1 = paragraph->getRectsForRange(0, 0, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes1.empty());

    auto boxes2 = paragraph->getRectsForRange(0, 1, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes2.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.left(), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.top(), 22.5f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.right(), 50, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes2[0].rect.bottom(), 72.5f, epsilon));

    auto boxes3 = paragraph->getRectsForRange(0, 1, rect_height_max_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes3.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.left(), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.top(), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.right(), 50, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes3[0].rect.bottom(), 80, epsilon));

    auto boxes4 = paragraph->getRectsForRange(6, 10, rect_height_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes4.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.left(), 300, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.top(), 22.5f, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.right(), 500, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes4[0].rect.bottom(), 72.5f, epsilon));

    auto boxes5 = paragraph->getRectsForRange(6, 10, rect_height_max_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes5.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.left(), 300, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.top(), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.right(), 500, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes5[0].rect.bottom(), 80, epsilon));

    auto boxes6 = paragraph->getRectsForRange(14, 16, rect_height_max_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes6.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.left(), 0, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.top(), 160, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.right(), 100, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes6[0].rect.bottom(), 240, epsilon));

    auto boxes7 = paragraph->getRectsForRange(20, 25, rect_height_max_style, rect_width_style);
    REPORTER_ASSERT(reporter, boxes7.size() == 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.left(), 50, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.top(), 240, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.right(), 300, epsilon));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(boxes7[0].rect.bottom(), 320, epsilon));
}

// Not in Minikin
DEF_TEST(SkParagraph_WhitespacesInMultipleFonts, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "English English 字典 字典 😀😃😄 😀😃😄";
    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies(
            {SkString("Roboto"), SkString("Noto Color Emoji"), SkString("Source Han Serif CN")});
    text_style.setFontSize(60);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    SkDEBUGCODE(auto impl = static_cast<ParagraphImpl*>(paragraph.get());)
    SkASSERT(impl->runs().size() == 3);
    SkASSERT(impl->runs()[0].text().end() == impl->runs()[1].text().begin());
    SkASSERT(impl->runs()[1].text().end() == impl->runs()[2].text().begin());
}

// 4 to 1
DEF_TEST(SkParagraph_JSON1, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "👨‍👩‍👧‍👦";

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Color Emoji")});
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto run = impl->runs().front();

    auto cluster = 0;
    SkShaperJSONWriter::VisualizeClusters(
            text, 0, std::strlen(text), run.glyphs(), run.clusterIndexes(),
            [&](int codePointCount, SkSpan<const char> utf1to1, SkSpan<const SkGlyphID> glyph1to1) {
                if (cluster == 0) {
                    std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                    SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                    SkASSERT(glyph1to1.size() == 1);
                    SkASSERT(*glyph1to1.begin() == 1611);
                }
                ++cluster;
            });
    SkASSERT(cluster <= 2);
}

// 5 to 3
DEF_TEST(SkParagraph_JSON2, reporter) {
    sk_sp<TestFontCollection> fontCollection = sk_make_sp<TestFontCollection>();
    if (!fontCollection->fontsFound()) return;
    const char* text = "p〠q";

    //icu::UnicodeString unicode(text, std::strlen(text));
    //std::string str;
    //unicode.toUTF8String(str);

    ParagraphStyle paragraph_style;
    paragraph_style.turnHintingOff();
    ParagraphBuilderImpl builder(paragraph_style, fontCollection);

    TextStyle text_style;
    text_style.setFontFamilies({SkString("Noto Sans CJK JP")});
    text_style.setColor(SK_ColorBLACK);
    text_style.setFontSize(50);
    builder.pushStyle(text_style);
    builder.addText(text);
    builder.pop();

    auto paragraph = builder.Build();
    paragraph->layout(TestCanvasWidth);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    auto run = impl->runs().front();

    auto cluster = 0;
    for (auto& run : impl->runs()) {
        SkShaperJSONWriter::VisualizeClusters(
                run.text().data(), 0, run.text().size(), run.glyphs(), run.clusterIndexes(),
                [&](int codePointCount, SkSpan<const char> utf1to1,
                    SkSpan<const SkGlyphID> glyph1to1) {
                    if (cluster == 0) {
                        std::string toCheckUtf8{utf1to1.data(), utf1to1.size()};
                        SkASSERT(std::strcmp(text, utf1to1.data()) == 0);
                        SkASSERT(glyph1to1.size() == 3);
                    }
                    ++cluster;
                });
    }

    SkASSERT(cluster <= 2);
}
