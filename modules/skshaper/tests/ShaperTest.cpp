// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tests/Test.h"

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkZip.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <cinttypes>
#include <cstdint>
#include <memory>

#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

namespace {

sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU::Make()) {
        return unicode;
    }
#endif  // defined(SK_UNICODE_ICU_IMPLEMENTATION)
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::Libgrapheme::Make()) {
        return unicode;
    }
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
    if (auto unicode = SkUnicodes::ICU4X::Make()) {
        return unicode;
    }
#endif
    return nullptr;
}

struct RunHandler final : public SkShaper::RunHandler {
    const char* fResource;
    skiatest::Reporter* fReporter;
    const char* fUtf8;
    size_t fUtf8Size;
    std::unique_ptr<SkGlyphID[]> fGlyphs;
    std::unique_ptr<SkPoint[]> fPositions;
    std::unique_ptr<uint32_t[]> fClusters;
    SkShaper::RunHandler::Range fRange;
    unsigned fGlyphCount = 0;

    bool fBeginLine = false;
    bool fCommitRunInfo = false;
    bool fCommitLine = false;

    RunHandler(const char* resource, skiatest::Reporter* reporter, const char* utf8,size_t utf8Size)
        : fResource(resource), fReporter(reporter), fUtf8(utf8), fUtf8Size(utf8Size) {}

    void beginLine() override { fBeginLine = true;}
    void runInfo(const SkShaper::RunHandler::RunInfo& info) override {}
    void commitRunInfo() override { fCommitRunInfo = true; }
    SkShaper::RunHandler::Buffer runBuffer(const SkShaper::RunHandler::RunInfo& info) override {
        fGlyphCount = SkToUInt(info.glyphCount);
        fRange = info.utf8Range;
        fGlyphs = std::make_unique<SkGlyphID[]>(info.glyphCount);
        fPositions = std::make_unique<SkPoint[]>(info.glyphCount);
        fClusters = std::make_unique<uint32_t[]>(info.glyphCount);
        return SkShaper::RunHandler::Buffer{fGlyphs.get(),
                                            fPositions.get(),
                                            nullptr,
                                            fClusters.get(),
                                            {0, 0}};
    }
    void commitRunBuffer(const RunInfo& info) override {
        REPORTER_ASSERT(fReporter, fGlyphCount == info.glyphCount, "%s", fResource);
        REPORTER_ASSERT(fReporter, fRange.begin() == info.utf8Range.begin(), "%s", fResource);
        REPORTER_ASSERT(fReporter, fRange.size() == info.utf8Range.size(), "%s", fResource);
        if (!(fRange.begin() + fRange.size() <= fUtf8Size)) {
            REPORTER_ASSERT(fReporter, fRange.begin() + fRange.size() <= fUtf8Size, "%s",fResource);
            return;
        }

        if ((false)) {
            SkString familyName;
            SkString postscriptName;
            SkTypeface* typeface = info.fFont.getTypeface();
            int ttcIndex = 0;
            size_t fontSize = 0;
            if (typeface) {
                typeface->getFamilyName(&familyName);
                typeface->getPostScriptName(&postscriptName);
                std::unique_ptr<SkStreamAsset> stream = typeface->openStream(&ttcIndex);
                if (stream) {
                    fontSize = stream->getLength();
                }
            }
            SkString glyphs;
            for (auto&& [glyph, cluster] : SkZip(info.glyphCount, fGlyphs.get(), fClusters.get())) {
                glyphs.appendU32(glyph);
                glyphs.append(":");
                glyphs.appendU32(cluster);
                glyphs.append(" ");
            }
            SkString chars;
            for (const char c : SkSpan(fUtf8 + fRange.begin(), fRange.size())) {
                chars.appendHex((unsigned char)c, 2);
                chars.append(" ");
            }
            SkDebugf(
                "%s range: %zu-%zu(%zu) glyphCount:%u font: \"%s\" \"%s\" #%d %zuB\n"
                "rangeText: \"%.*s\"\n"
                "rangeBytes: %s\n"
                "glyphs:%s\n\n",
                fResource, fRange.begin(), fRange.end(), fRange.size(), fGlyphCount,
                familyName.c_str(), postscriptName.c_str(), ttcIndex, fontSize,
                (int)fRange.size(), fUtf8 + fRange.begin(),
                chars.c_str(),
                glyphs.c_str());
        }

        for (unsigned i = 0; i < fGlyphCount; ++i) {
            REPORTER_ASSERT(fReporter, fClusters[i] >= fRange.begin(),
                            "%" PRIu32 " >= %zu %s i:%u glyphCount:%u",
                            fClusters[i], fRange.begin(), fResource, i, fGlyphCount);
            REPORTER_ASSERT(fReporter, fClusters[i] < fRange.end(),
                            "%" PRIu32 " < %zu %s i:%u glyphCount:%u",
                            fClusters[i], fRange.end(), fResource, i, fGlyphCount);
        }
    }
    void commitLine() override { fCommitLine = true; }
};

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
void shaper_test(skiatest::Reporter* reporter, const char* name, SkData* data) {
    skiatest::ReporterContext context(reporter, name);
    auto unicode = get_unicode();
    if (!unicode) {
        ERRORF(reporter, "Could not create unicode.");
        return;
    }

    auto shaper = SkShapers::HB::ShaperDrivenWrapper(unicode,
                                                     SkFontMgr::RefEmpty());  // no fallback
    if (!shaper) {
        ERRORF(reporter, "Could not create shaper.");
        return;
    }
    if (!unicode) {
        ERRORF(reporter, "Could not create unicode.");
        return;
    }
    constexpr float kWidth = 400;
    SkFont font = ToolUtils::DefaultFont();
    const char* utf8 = (const char*)data->data();
    size_t utf8Bytes = data->size();

    RunHandler rh(name, reporter, utf8, utf8Bytes);

    const SkBidiIterator::Level defaultLevel = SkBidiIterator::kLTR;
    std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
            SkShapers::unicode::BidiRunIterator(unicode, utf8, utf8Bytes, defaultLevel);
    SkASSERT(bidi);

    std::unique_ptr<SkShaper::LanguageRunIterator> language =
            SkShaper::MakeStdLanguageRunIterator(utf8, utf8Bytes);
    SkASSERT(language);

    std::unique_ptr<SkShaper::ScriptRunIterator> script =
            SkShapers::HB::ScriptRunIterator(utf8, utf8Bytes);
    SkASSERT(script);

    std::unique_ptr<SkShaper::FontRunIterator> fontRuns =
            SkShaper::MakeFontMgrRunIterator(utf8, utf8Bytes, font, SkFontMgr::RefEmpty());
    SkASSERT(fontRuns);
    shaper->shape(utf8, utf8Bytes, *fontRuns, *bidi, *script, *language, nullptr, 0, kWidth, &rh);

    // Even on empty input, expect that the line is started, that the zero run infos are committed,
    // and the empty line is committed. This allows the user to properly handle empty runs.
    REPORTER_ASSERT(reporter, rh.fBeginLine);
    REPORTER_ASSERT(reporter, rh.fCommitRunInfo);
    REPORTER_ASSERT(reporter, rh.fCommitLine);

    constexpr SkFourByteTag latn = SkSetFourByteTag('l','a','t','n');
    auto fontIterator = SkShaper::TrivialFontRunIterator(font, data->size());
    auto bidiIterator = SkShaper::TrivialBiDiRunIterator(0, data->size());
    auto scriptIterator = SkShaper::TrivialScriptRunIterator(latn, data->size());
    auto languageIterator = SkShaper::TrivialLanguageRunIterator("en-US", data->size());
    shaper->shape((const char*)data->data(),
                  data->size(),
                  fontIterator,
                  bidiIterator,
                  scriptIterator,
                  languageIterator,
                  nullptr,
                  0,
                  kWidth,
                  &rh);
}

void cluster_test(skiatest::Reporter* reporter, const char* resource) {
    auto data = GetResourceAsData(resource);
    if (!data) {
        ERRORF(reporter, "Could not get resource %s.", resource);
        return;
    }

    shaper_test(reporter, resource, data.get());
}

#endif  // defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)

}  // namespace

#if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)

DEF_TEST(Shaper_cluster_empty, r) { shaper_test(r, "empty", SkData::MakeEmpty().get()); }

#define SHAPER_TEST(X) DEF_TEST(Shaper_cluster_ ## X, r) { cluster_test(r, "text/" #X ".txt"); }
SHAPER_TEST(arabic)
SHAPER_TEST(armenian)
SHAPER_TEST(balinese)
SHAPER_TEST(buginese)
SHAPER_TEST(cherokee)
SHAPER_TEST(cyrillic)
SHAPER_TEST(emoji)
SHAPER_TEST(english)
SHAPER_TEST(ethiopic)
SHAPER_TEST(greek)
SHAPER_TEST(hangul)
SHAPER_TEST(han_simplified)
SHAPER_TEST(han_traditional)
SHAPER_TEST(hebrew)
SHAPER_TEST(javanese)
SHAPER_TEST(kana)
SHAPER_TEST(lao)
SHAPER_TEST(mandaic)
SHAPER_TEST(newtailue)
SHAPER_TEST(nko)
SHAPER_TEST(sinhala)
SHAPER_TEST(sundanese)
SHAPER_TEST(syriac)
SHAPER_TEST(thaana)
SHAPER_TEST(thai)
SHAPER_TEST(tibetan)
SHAPER_TEST(tifnagh)
SHAPER_TEST(vai)
SHAPER_TEST(bengali)
SHAPER_TEST(devanagari)
SHAPER_TEST(khmer)
SHAPER_TEST(myanmar)
SHAPER_TEST(taitham)
SHAPER_TEST(tamil)
#undef SHAPER_TEST

#endif  // #if defined(SK_SHAPER_HARFBUZZ_AVAILABLE) && defined(SK_SHAPER_UNICODE_AVAILABLE)
