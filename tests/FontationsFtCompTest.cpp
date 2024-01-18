/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "tests/Test.h"
#include "tools/TestFontDataProvider.h"

#include <memory>

namespace {

bool textBlobsAllPathsEqual(sk_sp<const SkTextBlob> blobA,
                            sk_sp<const SkTextBlob> blobB,
                            SkString fontName) {
    SkTextBlob::Iter iterA(*blobA);
    SkTextBlob::Iter iterB(*blobB);
    SkTextBlob::Iter::ExperimentalRun runAInfo;
    SkTextBlob::Iter::ExperimentalRun runBInfo;
    SkPath pathA, pathB;
    while (iterA.experimentalNext(&runAInfo)) {
        pathA.reset();
        pathB.reset();
        SkASSERT(iterB.experimentalNext(&runBInfo));
        for (int i = 0; i < runAInfo.count; ++i) {
            runAInfo.font.getPath(runAInfo.glyphs[i], &pathA);
            runBInfo.font.getPath(runBInfo.glyphs[i], &pathB);
            SkDynamicMemoryWStream streamA, streamB;
            // Re-use the logic in Path::dump() to canonicalize the output and account for
            // differences between FreeType (inserting a lineTo() to original moveTo() coordinate)
            // or Fontations (which saves the extra lineTo() before close()).
            pathA.dump(&streamA, false);
            pathB.dump(&streamB, false);
            sk_sp<SkData> dataA = streamA.detachAsData();
            sk_sp<SkData> dataB = streamB.detachAsData();
            if (dataA->size() != dataB->size() ||
                memcmp(dataA->data(), dataB->data(), dataA->size() - 1)) {
                SkDebugf(
                        "Different path in font %s for glyph index: %d glyph id: %d, data sizes "
                        "%ld "
                        "vs %ld\n",
                        fontName.c_str(),
                        i,
                        runAInfo.glyphs[i],
                        dataA->size(),
                        dataB->size());
                std::string fontationsPath(reinterpret_cast<const char*>(dataA->bytes()),
                                           dataA->size());
                std::string freetypePath(reinterpret_cast<const char*>(dataB->bytes()),
                                         dataB->size());
                SkDebugf("Path A (Fontations): \n%s\n", fontationsPath.c_str());
                SkDebugf("Path B (FreeType): \n%s\n", freetypePath.c_str());
                return false;
            }
        }
    }
    return true;
}

class FontationsFtComparison {
public:
    FontationsFtComparison(std::string fontMatch, std::string langMatch)
            : fTestDataIterator(fontMatch, langMatch) {}

    void assertAllPathsEqual(skiatest::Reporter* reporter) {
        TestFontDataProvider::TestSet testSet;
        size_t numTestsExecuted = 0;
        while (fTestDataIterator.next(&testSet)) {
            sk_sp<SkTypeface> fontationsTypeface = SkTypeface_Make_Fontations(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());
            sk_sp<SkTypeface> freetypeTypeface = SkTypeface_FreeType::MakeFromStream(
                    SkStream::MakeFromFile(testSet.fontFilename.c_str()), SkFontArguments());

            SkASSERT_RELEASE(fontationsTypeface);
            SkASSERT_RELEASE(freetypeTypeface);

            int upem = fontationsTypeface->getUnitsPerEm();
            SkFont fontationsFont(fontationsTypeface);
            SkFont freetypeFont(freetypeTypeface);

            auto configureFont = [upem](SkFont& font) {
                font.setSize(upem);
                font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                font.setSubpixel(true);
                font.setHinting(SkFontHinting::kNone);
            };

            configureFont(fontationsFont);
            configureFont(freetypeFont);

            for (const auto& sampleLang : testSet.langSamples) {
                sk_sp<const SkTextBlob> fontationsTextBlob =
                        makeTextBlobWithFontAndText(fontationsFont, sampleLang.sampleLong);
                sk_sp<const SkTextBlob> freetypeTextBlob =
                        makeTextBlobWithFontAndText(freetypeFont, sampleLang.sampleLong);

                REPORTER_ASSERT(reporter,
                                textBlobsAllPathsEqual(
                                        fontationsTextBlob, freetypeTextBlob, testSet.fontName),
                                "paths not equal for %s",
                                testSet.fontName.c_str());
            }
            numTestsExecuted++;
        }
        REPORTER_ASSERT(reporter,
                        numTestsExecuted > 0,
                        "Error: FontationsFtComparison did not run any tests, missing third-party "
                        "googlefonts_testdata resource? See bin/fetch-fonts-testdata.");
    }

private:
    sk_sp<const SkTextBlob> makeTextBlobWithFontAndText(const SkFont& font,
                                                        const SkString& testPhrase) {
        std::unique_ptr<SkShaper> shaper = SkShaper::Make();
        SkTextBlobBuilderRunHandler textBlobBuilder(testPhrase.c_str(), {0, 0});
        SkString fam;
        font.getTypeface()->getFamilyName(&fam);
        SkASSERT_RELEASE(testPhrase.size());
        // Reserve enough space for the test phrase to fit into one line.
        SkScalar shapeWidth = font.getTypeface()->getUnitsPerEm() * testPhrase.size() * 1.10f;
        shaper->shape(
                testPhrase.c_str(), testPhrase.size(), font, true, shapeWidth, &textBlobBuilder);
        return textBlobBuilder.makeBlob();
    }

    TestFontDataProvider fTestDataIterator;
};

}  // namespace

DEF_TEST(Fontations_NotoSans, reporter) {
    FontationsFtComparison("Noto Sans",
                           "en_Latn|es_Latn|pt_Latn|id_Latn|ru_Cyrl|fr_Latn|tr_Latn|vi_Latn|de_"
                           "Latn|it_Latn|pl_Latn|nl_Latn|uk_Cyrl|gl_Latn|ro_Latn|cs_Latn|hu_Latn|"
                           "el_Grek|se_Latn|da_Latn|bg_Latn|sk_Latn|fi_Latn|bs_Latn|ca_Latn|no_"
                           "Latn|sr_Latn|sr_Cyrl|lt_Latn|hr_Latn|sl_Latn|uz_Latn|uz_Cyrl|lv_Latn|"
                           "et_Latn|az_Latn|az_Cyrl|la_Latn|tg_Latn|tg_Cyrl|sw_Latn|mn_Cyrl|kk_"
                           "Latn|kk_Cyrl|sq_Latn|af_Latn|ha_Latn|ky_Cyrl")
            .assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Deva, reporter) {
    FontationsFtComparison("Noto Sans Devanagari", "hi_Deva|mr_Deva").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_ar_Arab, reporter) {
    FontationsFtComparison("Noto Sans Arabic", "ar_Arab|uz_Arab|kk_Arab|ky_Arab")
            .assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Beng, reporter) {
    FontationsFtComparison("Noto Sans Bengali", "bn_Beng").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Jpan, reporter) {
    FontationsFtComparison("Noto Sans JP", "ja_Jpan").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Thai, reporter) {
    FontationsFtComparison("Noto Sans Thai", "th_Thai").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Hans, reporter) {
    FontationsFtComparison("Noto Sans SC", "zh_Hans").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Hant, reporter) {
    FontationsFtComparison("Noto Sans TC", "zh_Hant").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Kore, reporter) {
    FontationsFtComparison("Noto Sans KR", "ko_Kore").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Taml, reporter) {
    FontationsFtComparison("Noto Sans Tamil", "ta_Taml").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Newa, reporter) {
    FontationsFtComparison("Noto Sans Newa", "new_Newa").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Knda, reporter) {
    FontationsFtComparison("Noto Sans Kannada", "kn_Knda").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Tglg, reporter) {
    FontationsFtComparison("Noto Sans Tagalog", "fil_Tglg").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Telu, reporter) {
    FontationsFtComparison("Noto Sans Telugu", "te_Telu").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Gujr, reporter) {
    FontationsFtComparison("Noto Sans Gujarati", "gu_Gujr").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Geor, reporter) {
    FontationsFtComparison("Noto Sans Georgian", "ka_Geor").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Mlym, reporter) {
    FontationsFtComparison("Noto Sans Malayalam", "ml_Mlym").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Khmr, reporter) {
    FontationsFtComparison("Noto Sans Khmer", "km_Khmr").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Sinh, reporter) {
    FontationsFtComparison("Noto Sans Sinhala", "si_Sinh").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Mymr, reporter) {
    FontationsFtComparison("Noto Sans Myanmar", "my_Mymr").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Java, reporter) {
    FontationsFtComparison("Noto Sans Javanese", "jv_Java").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Mong, reporter) {
    FontationsFtComparison("Noto Sans Mongolian", "mn_Mong").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Armn, reporter) {
    FontationsFtComparison("Noto Sans Armenian", "hy_Armn").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Elba, reporter) {
    FontationsFtComparison("Noto Sans Elbasan", "sq_Elba").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Vith, reporter) {
    FontationsFtComparison("Noto Sans Vithkuqi", "sq_Vith").assertAllPathsEqual(reporter);
}

DEF_TEST(Fontations_NotoSans_Guru, reporter) {
    FontationsFtComparison("Noto Sans Gurmukhi", "pa_Guru").assertAllPathsEqual(reporter);
}
