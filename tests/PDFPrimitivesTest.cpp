/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_SUPPORT_PDF

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDocument.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/docs/SkPDFDocument.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkRandom.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/pdf/SkClusterator.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFont.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUnion.h"
#include "src/pdf/SkPDFUtils.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkFloatToDecimal.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>

class SkTypeface;

template <typename T>
static SkString emit_to_string(T& obj) {
    SkDynamicMemoryWStream buffer;
    obj.emitObject(&buffer);
    SkString tmp(buffer.bytesWritten());
    buffer.copyTo(tmp.data());
    return tmp;
}

static bool eq(const SkString& str, const char* strPtr, size_t len) {
    return len == str.size() && 0 == memcmp(str.c_str(), strPtr, len);
}

static void assert_eql(skiatest::Reporter* reporter,
                       const SkString& skString,
                       const char* str,
                       size_t len) {
    if (!eq(skString, str, len)) {
        REPORT_FAILURE(reporter, "", SkStringPrintf(
                "'%*s' != '%s'", (int)len, str, skString.c_str()));
    }
}

static void assert_eq(skiatest::Reporter* reporter,
                      const SkString& skString,
                      const char* str) {
    assert_eql(reporter, skString, str, strlen(str));
}


template <typename T>
static void assert_emit_eq(skiatest::Reporter* reporter,
                           T& object,
                           const char* string) {
    SkString result = emit_to_string(object);
    assert_eq(reporter, result, string);
}

// This test used to assert without the fix submitted for
// http://code.google.com/p/skia/issues/detail?id=1083.
// SKP files might have invalid glyph ids. This test ensures they are ignored,
// and there is no assert on input data in Debug mode.
static void test_issue1083() {
    SkDynamicMemoryWStream outStream;
    auto doc = SkPDF::MakeDocument(&outStream);
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);

    uint16_t glyphID = 65000;
    SkFont font = ToolUtils::DefaultFont();
    canvas->drawSimpleText(&glyphID, 2, SkTextEncoding::kGlyphID, 0, 0, font, SkPaint());

    doc->close();
}

static void assert_emit_eq_number(skiatest::Reporter* reporter, float number) {
    SkPDFUnion pdfUnion = SkPDFUnion::Scalar(number);
    SkString result = emit_to_string(pdfUnion);
    float value = static_cast<float>(std::atof(result.c_str()));
    if (value != number) {
        ERRORF(reporter, "%.9g != %s", number, result.c_str());
    }
}


static void TestPDFUnion(skiatest::Reporter* reporter) {
    SkPDFUnion boolTrue = SkPDFUnion::Bool(true);
    assert_emit_eq(reporter, boolTrue, "true");

    SkPDFUnion boolFalse = SkPDFUnion::Bool(false);
    assert_emit_eq(reporter, boolFalse, "false");

    SkPDFUnion int42 = SkPDFUnion::Int(42);
    assert_emit_eq(reporter, int42, "42");

    assert_emit_eq_number(reporter, SK_ScalarHalf);
    assert_emit_eq_number(reporter, 110999.75f);  // bigScalar
    assert_emit_eq_number(reporter, 50000000.1f);  // biggerScalar
    assert_emit_eq_number(reporter, 1.0f / 65536);  // smallScalar

    SkPDFUnion stringSimple = SkPDFUnion::TextString("test ) string ( foo");
    assert_emit_eq(reporter, stringSimple, "(test \\) string \\( foo)");

    SkString stringComplexInput("\ttest ) string ( foo");
    SkPDFUnion stringComplex = SkPDFUnion::TextString(stringComplexInput);
    assert_emit_eq(reporter, stringComplex, "(\\011test \\) string \\( foo)");

    SkString binaryStringInput("\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20");
    SkPDFUnion binaryString = SkPDFUnion::ByteString(binaryStringInput);
    assert_emit_eq(reporter, binaryString, "<0102030405060708090A0B0C0D0E0F10>");

    SkString nameInput("Test name\twith#tab");
    SkPDFUnion name = SkPDFUnion::Name(nameInput);
    assert_emit_eq(reporter, name, "/Test#20name#09with#23tab");

    SkString nameInput2("A#/%()<>[]{}B");
    SkPDFUnion name2 = SkPDFUnion::Name(nameInput2);
    assert_emit_eq(reporter, name2, "/A#23#2F#25#28#29#3C#3E#5B#5D#7B#7DB");

    SkPDFUnion name3 = SkPDFUnion::Name("SimpleNameWithOnlyPrintableASCII");
    assert_emit_eq(reporter, name3, "/SimpleNameWithOnlyPrintableASCII");

    // Test that we correctly handle characters with the high-bit set.
    SkString highBitString("\xDE\xAD" "be\xEF");
    SkPDFUnion highBitName = SkPDFUnion::Name(highBitString);
    assert_emit_eq(reporter, highBitName, "/#DE#ADbe#EF");

    // https://bugs.skia.org/9508
    // https://crbug.com/494913
    // Trailing '\0' characters must be removed.
    const char nameInput4[] = "Test name with nil\0";
    SkPDFUnion name4 = SkPDFUnion::Name(SkString(nameInput4, strlen(nameInput4) + 1));
    assert_emit_eq(reporter, name4, "/Test#20name#20with#20nil");
}

static void TestPDFArray(skiatest::Reporter* reporter) {
    std::unique_ptr<SkPDFArray> array(new SkPDFArray);
    assert_emit_eq(reporter, *array, "[]");

    array->appendInt(42);
    assert_emit_eq(reporter, *array, "[42]");

    array->appendScalar(SK_ScalarHalf);
    assert_emit_eq(reporter, *array, "[42 .5]");

    array->appendInt(0);
    assert_emit_eq(reporter, *array, "[42 .5 0]");

    array->appendBool(true);
    assert_emit_eq(reporter, *array, "[42 .5 0 true]");

    array->appendName("ThisName");
    assert_emit_eq(reporter, *array, "[42 .5 0 true /ThisName]");

    array->appendName(SkString("AnotherName"));
    assert_emit_eq(reporter, *array, "[42 .5 0 true /ThisName /AnotherName]");

    array->appendTextString("This String");
    assert_emit_eq(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String)]");

    array->appendByteString(SkString("Another String"));
    assert_emit_eq(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String) "
                   "(Another String)]");

    std::unique_ptr<SkPDFArray> innerArray(new SkPDFArray);
    innerArray->appendInt(-1);
    array->appendObject(std::move(innerArray));
    assert_emit_eq(reporter, *array,
                   "[42 .5 0 true /ThisName /AnotherName (This String) "
                   "(Another String) [-1]]");
}

static void TestPDFDict(skiatest::Reporter* reporter) {
    std::unique_ptr<SkPDFDict> dict(new SkPDFDict);
    assert_emit_eq(reporter, *dict, "<<>>");

    dict->insertInt("n1", SkToSizeT(42));
    assert_emit_eq(reporter, *dict, "<</n1 42>>");

    dict = std::make_unique<SkPDFDict>();
    assert_emit_eq(reporter, *dict, "<<>>");

    dict->insertInt("n1", 42);
    assert_emit_eq(reporter, *dict, "<</n1 42>>");

    dict->insertScalar("n2", SK_ScalarHalf);

    SkString n3("n3");
    std::unique_ptr<SkPDFArray> innerArray(new SkPDFArray);
    innerArray->appendInt(-100);
    dict->insertObject(n3, std::move(innerArray));
    assert_emit_eq(reporter, *dict, "<</n1 42\n/n2 .5\n/n3 [-100]>>");

    dict = std::make_unique<SkPDFDict>();
    assert_emit_eq(reporter, *dict, "<<>>");

    dict->insertInt("n1", 24);
    assert_emit_eq(reporter, *dict, "<</n1 24>>");

    dict->insertInt("n2", SkToSizeT(99));
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99>>");

    dict->insertScalar("n3", SK_ScalarHalf);
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5>>");

    dict->insertName("n4", "AName");
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName>>");

    dict->insertName("n5", SkString("AnotherName"));
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName>>");

    dict->insertTextString("n6", "A String");
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName\n/n6 (A String)>>");

    dict->insertByteString("n7", SkString("Another String"));
    assert_emit_eq(reporter, *dict, "<</n1 24\n/n2 99\n/n3 .5\n/n4 /AName\n"
                   "/n5 /AnotherName\n/n6 (A String)\n/n7 (Another String)>>");

    dict = std::make_unique<SkPDFDict>("DType");
    assert_emit_eq(reporter, *dict, "<</Type /DType>>");
}

DEF_TEST(SkPDF_Primitives, reporter) {
    TestPDFUnion(reporter);
    TestPDFArray(reporter);
    TestPDFDict(reporter);
    test_issue1083();
}

namespace {

class TestImageFilter : public SkImageFilter_Base {
public:
    TestImageFilter() : SkImageFilter_Base(nullptr, 0), fVisited(false) {}

    bool visited() const { return fVisited; }

private:
    Factory getFactory() const override {
        SK_ABORT("Does not participate in serialization");
        return nullptr;
    }
    const char* getTypeName() const override { return "TestImageFilter"; }

    skif::FilterResult onFilterImage(const skif::Context& ctx) const override {
        fVisited = true;
        return ctx.source();
    }

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override {
        return desiredOutput;
    }

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override {
        return contentBounds;
    }

    mutable bool fVisited;
};

}  // namespace

// Check that PDF rendering of image filters successfully falls back to
// CPU rasterization.
DEF_TEST(SkPDF_ImageFilter, reporter) {
    REQUIRE_PDF_DOCUMENT(SkPDF_ImageFilter, reporter);
    SkDynamicMemoryWStream stream;
    auto doc = SkPDF::MakeDocument(&stream);
    SkCanvas* canvas = doc->beginPage(100.0f, 100.0f);

    sk_sp<TestImageFilter> filter(new TestImageFilter());

    // Filter just created; should be unvisited.
    REPORTER_ASSERT(reporter, !filter->visited());
    SkPaint paint;
    paint.setImageFilter(filter);
    canvas->drawRect(SkRect::MakeWH(100, 100), paint);
    doc->close();

    // Filter was used in rendering; should be visited.
    REPORTER_ASSERT(reporter, filter->visited());
}

// Check that PDF rendering of image filters successfully falls back to
// CPU rasterization.
DEF_TEST(SkPDF_FontCanEmbedTypeface, reporter) {
    SkNullWStream nullWStream;
    SkPDFDocument doc(&nullWStream, SkPDF::Metadata());

    const char resource[] = "fonts/Roboto2-Regular_NoEmbed.ttf";
    sk_sp<SkTypeface> noEmbedTypeface(ToolUtils::CreateTypefaceFromResource(resource));
    if (noEmbedTypeface) {
        REPORTER_ASSERT(reporter,
                        !SkPDFFont::CanEmbedTypeface(noEmbedTypeface.get(), &doc));
    }
    sk_sp<SkTypeface> portableTypeface(ToolUtils::DefaultTypeface());
    REPORTER_ASSERT(reporter,
                    SkPDFFont::CanEmbedTypeface(portableTypeface.get(), &doc));
}


// test to see that all finite scalars round trip via scanf().
static void check_pdf_scalar_serialization(
        skiatest::Reporter* reporter, float inputFloat) {
    char floatString[kMaximumSkFloatToDecimalLength];
    size_t len = SkFloatToDecimal(inputFloat, floatString);
    if (len >= sizeof(floatString)) {
        ERRORF(reporter, "string too long: %u", (unsigned)len);
        return;
    }
    if (floatString[len] != '\0' || strlen(floatString) != len) {
        ERRORF(reporter, "terminator misplaced.");
        return;  // The terminator is needed for sscanf().
    }
    if (reporter->verbose()) {
        SkDebugf("%15.9g = \"%s\"\n", inputFloat, floatString);
    }
    float roundTripFloat;
    if (1 != sscanf(floatString, "%f", &roundTripFloat)) {
        ERRORF(reporter, "unscannable result: %s", floatString);
        return;
    }
    if (std::isfinite(inputFloat) && roundTripFloat != inputFloat) {
        ERRORF(reporter, "roundTripFloat (%.9g) != inputFloat (%.9g)",
               roundTripFloat, inputFloat);
    }
}

// Test SkPDFUtils::AppendScalar for accuracy.
DEF_TEST(SkPDF_Primitives_Scalar, reporter) {
    SkRandom random(0x5EED);
    int iterationCount = 512;
    while (iterationCount-- > 0) {
        union { uint32_t u; float f; };
        u = random.nextU();
        static_assert(sizeof(float) == sizeof(uint32_t), "");
        check_pdf_scalar_serialization(reporter, f);
    }
    float alwaysCheck[] = {
        0.0f, -0.0f, 1.0f, -1.0f, SK_ScalarPI, 0.1f, FLT_MIN, FLT_MAX,
        -FLT_MIN, -FLT_MAX, FLT_MIN / 16.0f, -FLT_MIN / 16.0f,
        SK_FloatNaN, SK_FloatInfinity, SK_FloatNegativeInfinity,
        -FLT_MIN / 8388608.0
    };
    for (float inputFloat: alwaysCheck) {
        check_pdf_scalar_serialization(reporter, inputFloat);
    }
}

// Test SkPDFUtils:: for accuracy.
DEF_TEST(SkPDF_Primitives_Color, reporter) {
    char buffer[5];
    for (int i = 0; i < 256; ++i) {
        size_t len = SkPDFUtils::ColorToDecimal(i, buffer);
        REPORTER_ASSERT(reporter, len == strlen(buffer));
        float f;
        REPORTER_ASSERT(reporter, 1 == sscanf(buffer, "%f", &f));
        int roundTrip = (int)(0.5 + f * 255);
        REPORTER_ASSERT(reporter, roundTrip == i);
    }
}

static sktext::GlyphRun make_run(size_t len, const SkGlyphID* glyphs, SkPoint* pos,
                           const SkFont& font, const uint32_t* clusters,
                           size_t utf8TextByteLength, const char* utf8Text) {
    return sktext::GlyphRun(font,
                            SkSpan<const SkPoint>{pos, len},
                            SkSpan<const SkGlyphID>{glyphs, len},
                            SkSpan<const char>{utf8Text, utf8TextByteLength},
                            SkSpan<const uint32_t>{clusters, len},
                            SkSpan<const SkVector>{});
}

DEF_TEST(SkPDF_Clusterator, reporter) {
    SkFont font;
    {
        constexpr unsigned len = 11;
        const uint32_t clusters[len] = { 3, 2, 2, 1, 0, 4, 4, 7, 6, 6, 5 };
        const SkGlyphID glyphs[len] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
        SkPoint pos[len] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
                                  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
        const char text[] = "abcdefgh";
        sktext::GlyphRun run = make_run(len, glyphs, pos, font, clusters, strlen(text), text);
        SkClusterator clusterator(run);
        SkClusterator::Cluster expectations[] = {
            {&text[3], 1, 0, 1},
            {&text[2], 1, 1, 2},
            {&text[1], 1, 3, 1},
            {&text[0], 1, 4, 1},
            {&text[4], 1, 5, 2},
            {&text[7], 1, 7, 1},
            {&text[6], 1, 8, 2},
            {&text[5], 1, 10, 1},
            {nullptr, 0, 0, 0},
        };
        for (const auto& expectation : expectations) {
            REPORTER_ASSERT(reporter, clusterator.next() == expectation);
        }
    }
    {
        constexpr unsigned len = 5;
        const uint32_t clusters[len] = { 0, 1, 4, 5, 6 };
        const SkGlyphID glyphs[len] = { 43, 167, 79, 79, 82, };
        SkPoint pos[len] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
        const char text[] = "Ha\xCC\x8A" "llo";
        sktext::GlyphRun run = make_run(len, glyphs, pos, font, clusters, strlen(text), text);
        SkClusterator clusterator(run);
        SkClusterator::Cluster expectations[] = {
            {&text[0], 1, 0, 1},
            {&text[1], 3, 1, 1},
            {&text[4], 1, 2, 1},
            {&text[5], 1, 3, 1},
            {&text[6], 1, 4, 1},
            {nullptr, 0, 0, 0},
        };
        for (const auto& expectation : expectations) {
            REPORTER_ASSERT(reporter, clusterator.next() == expectation);
        }
    }
}

DEF_TEST(fuzz875632f0, reporter) {
    SkNullWStream stream;
    auto doc = SkPDF::MakeDocument(&stream);
    REPORTER_ASSERT(reporter, doc);
    SkCanvas* canvas = doc->beginPage(128, 160);

    SkAutoCanvasRestore autoCanvasRestore(canvas, false);

    SkPaint layerPaint({0, 0, 0, 0});
    layerPaint.setImageFilter(SkImageFilters::Dilate(536870912, 0, nullptr, nullptr));
    layerPaint.setBlendMode(SkBlendMode::kClear);

    canvas->saveLayer(nullptr, &layerPaint);
    canvas->saveLayer(nullptr, nullptr);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDarken);
    paint.setShader(SkShaders::MakeFractalNoise(0, 0, 2, 0, nullptr));
    paint.setColor4f(SkColor4f{0, 0, 0 ,0});

    canvas->drawPath(SkPath(), paint);
}
#endif
